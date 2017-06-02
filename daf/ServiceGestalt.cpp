/***************************************************************
    Copyright 2016, 2017 Defence Science and Technology Group,
    Department of Defence,
    Australian Government

	This file is part of LASAGNE.

    LASAGNE is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as
    published by the Free Software Foundation, either version 3
    of the License, or (at your option) any later version.

    LASAGNE is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with LASAGNE.  If not, see <http://www.gnu.org/licenses/>.
***************************************************************/
#define DAF_SERVICEGESTALT_CPP

#include "ServiceGestalt.h"

#include "PropertyManager.h"
#include "Runnable.h"
#include "Monitor.h"

#include <ace/Service_Types.h>

namespace {

    class ServiceAction : public DAF::Runnable
    {
        virtual int run(void);

    public:

        DAF_DEFINE_REFCOUNTABLE(ServiceAction);

        ServiceAction(ACE_Service_Gestalt *gestalt, const std::string &command);

        int wait_result(time_t timeout) const;

    private:

        mutable ACE_Semaphore   resultLock_;

        ACE_Service_Gestalt *   gestalt_;
        const std::string       command_;
        int                     result_;
        int                     error_;
    };

    DAF_DECLARE_REFCOUNTABLE(ServiceAction);

    ServiceAction::ServiceAction(ACE_Service_Gestalt *gestalt, const std::string &command) : DAF::Runnable()
        , resultLock_   (0)
        , gestalt_      (gestalt)
        , command_      (command)
        , result_       (-1)
        , error_        (0)
    {
        DAF_OS::last_error(0); // Reset Error Code for invoker
    }

    int
    ServiceAction::run(void)
    {
        if (this->gestalt_) try {

            ACE_Service_Config_Guard svc_guard(this->gestalt_); ACE_UNUSED_ARG(svc_guard);
            if ((this->result_ = this->gestalt_->process_directive(this->command_.c_str())) != 0) {
#if defined(ACE_WIN32)
                for (int error = ::GetLastError(); error;) {
                    DAF_OS::last_error(error); break;
                }
#endif
                this->error_ = DAF_OS::last_error();
            }

        } DAF_CATCH_ALL{ /* Possible failure in user code */ }

        this->resultLock_.release(); return 0;
    }

    int
    ServiceAction::wait_result(time_t timeout) const
    {
        for (ACE_Time_Value tm(DAF_OS::gettimeofday(timeout)); this->resultLock_.acquire(tm);) {
            DAF_OS::last_error(DAF_OS::last_error()); return -1;  // Possibly a timeout
        }

        this->resultLock_.release(); DAF_OS::last_error(this->error_); return this->result_;
    }
}

namespace DAF
{
#if !defined(ACE_HAS_SERVICE_REPOSITORY_LOCK_ACCESS)
    ACE_Recursive_Thread_Mutex & getRepositoryLock(const ACE_Service_Repository *repo) // Hack to get hold of Repository Lock
    {
        // Hack to gain access to Repository Lock - relies on ACE_Service_Repository being non-polymorphic
        struct DAF_Service_Repository : ACE_Service_Repository {
            ACE_Recursive_Thread_Mutex & _repositoryLock(void) const {
                return const_cast<DAF_Service_Repository*>(this)->lock_;
            }
        };

        for (const DAF_Service_Repository *repo_hack = reinterpret_cast<const DAF_Service_Repository *>(repo); repo_hack;) {
            return repo_hack->_repositoryLock(); // Hack
        }

        DAF_THROW_EXCEPTION(DAF::ClassCastException);
    }
#endif

    std::string
    locateServiceIdent(const ACE_Service_Object *svc_obj, const ACE_Service_Gestalt *sg)
    {
        ACE_Service_Repository *repo = (sg ? const_cast<ACE_Service_Gestalt*>(sg)->current_service_repository() : 0);

        if (repo && svc_obj) do {

#if defined(ACE_HAS_SERVICE_REPOSITORY_LOCK_ACCESS)
            ACE_GUARD_REACTION(ACE_Recursive_Thread_Mutex, mon, repo->lock(), break);
#else
            ACE_GUARD_REACTION(ACE_Recursive_Thread_Mutex, mon, DAF::getRepositoryLock(repo), break);
#endif
            ACE_Service_Repository_Iterator it(*repo, false);
            for (const ACE_Service_Type *svc_type = 0; it.next(svc_type); it.advance()) {
                if (svc_type) {
                    const ACE_Service_Object_Type * so = dynamic_cast<const ACE_Service_Object_Type*>(svc_type->type());
                    if (so && so->object() == svc_obj) {
                        return svc_type->name();
                    }
                }
            }

        } while (false);

        DAF_THROW_EXCEPTION(DAF::NotFoundException);
    }

    void
    print_gestalt(const ACE_Service_Gestalt *sg)
    {
        ACE_Service_Repository *repo = (sg ? const_cast<ACE_Service_Gestalt*>(sg)->current_service_repository() : 0);

        if (repo) do {

#if defined(ACE_HAS_SERVICE_REPOSITORY_LOCK_ACCESS)
            ACE_GUARD_REACTION(ACE_Recursive_Thread_Mutex, mon, repo->lock(), break);
#else
            ACE_GUARD_REACTION(ACE_Recursive_Thread_Mutex, mon, DAF::getRepositoryLock(repo), break);
#endif
            ACE_Service_Repository_Iterator it(*repo, false);

            const ACE_Service_Type *svc_type = 0;

            for (int i = 0; it.next(svc_type); it.advance()) {
                if (svc_type) {
                    ACE_DEBUG((LM_DEBUG, ACE_TEXT("Service[%03d] - %s:%s %s\n")
                        , i++
                        , svc_type->name()
                        , svc_type->dll().dll_name_
                        , ACE_TEXT(svc_type->fini_called() ? "(finished)" : (svc_type->active() ? "(active)" : ""))));
                }
            }

        } while (false);
    }

    /************************************************************************************/

    ServiceGestalt::ServiceGestalt(const ACE_TCHAR *program_name) : ACE_Service_Gestalt(DEFAULT_SIZE, true, false)
    {
        ACE_Service_Gestalt::intrusive_add_ref(this);
        ACE_Service_Gestalt::open(program_name, 0, true, true, false); // Open the Gestalt
    }

    ServiceGestalt::~ServiceGestalt(void)
    {
        ACE_Service_Config_Guard svc_guard(this); ACE_UNUSED_ARG(svc_guard);
        ACE_Service_Gestalt::close();
    }

    int
    ServiceGestalt::process_command(const std::string &command, time_t timeout)
    {
        static const time_t service_timeout(DAF::get_numeric_property<time_t>(  DAF_SVCACTIONTIMEOUT,
                                                                                DAF_DEFAULT_SERVICE_ACTION_TIMEOUT,
                                                                                true) * DAF_MSECS_ONE_SECOND);
        /*
           NOTE: Thread this command through to a synchronised Active Object [Pattern] (AOP).
           This command maybe being invoked through from ORB Reactor thread (ORB call) so service logic
           like POA servant activations (ie through ACE_Service_Object::init()) would not have been valid
           otherwise without threading it through and synchronising with this AOP!!
        */

        if (command.length()) {

            const ServiceAction_ref svc_action(new ServiceAction(this, command));
            if (this->svcExecutor_.execute(*svc_action) == 0) {
                return svc_action->wait_result(ace_range(timeout, time_t(DEFAULT_MAXIMUM_TIMEOUT), service_timeout));
            }
        }

        DAF_OS::last_error(ENOEXEC); return -1;  // Not Executable
    }

    std::string
    ServiceGestalt::static_directive(const ACE_TCHAR *ident, const ACE_TCHAR *parameters)
    {
        std::stringstream s;

#if (ACE_USES_CLASSIC_SVC_CONF == 1)
        s << ACE_TEXT("static ");
        s << ident;
        s << ACE_TEXT(" \"");
        if (parameters && DAF_OS::strlen(parameters) > 0) {
            s << parameters;
        }
        s << '"';
#else
        s << ACE_TEXT("<ACE_Svc_Conf>");
        s << ACE_TEXT("<static id=\"");
        s << ident;
        s << '"';
        if (parameters && DAF_OS::strlen(parameters) > 0) {
            s << ACE_TEXT(" params=\"");
            s << parameters;
            s << '"';
        }
        s << ACE_TEXT("/>");
        s << ACE_TEXT("</static>");
        s << ACE_TEXT("</ACE_Svc_Conf>");
#endif
        std::ends(s); return s.str();
    }

    std::string
    ServiceGestalt::dynamic_directive(const ACE_TCHAR *ident, const ACE_TCHAR *libpathname, const ACE_TCHAR *objectclass, const ACE_TCHAR *parameters)
    {
        std::stringstream s;

#if (ACE_USES_CLASSIC_SVC_CONF == 1)
        s << ACE_TEXT("dynamic ");
        s << ident;
        s << ACE_TEXT(" Service_Object * ");
        s << libpathname;
        s << ':';
        s << objectclass;
        s << ACE_TEXT("() \"");
        if (parameters && DAF_OS::strlen(parameters) > 0) {
            s << parameters;
        }
        s << '"';
#else
        s << ACE_TEXT("<ACE_Svc_Conf>");
        s << ACE_TEXT("<dynamic id=\"");
        s << ident;
        s << ACE_TEXT("\" type=\"Service_Object\">");
        s << ACE_TEXT("<initializer path=\"");
        s << libpathname;
        s << ACE_TEXT("\" init=\"");
        s << objectclass;
        s << '"';
        if (parameters && DAF_OS::strlen(parameters) > 0) {
            s << ACE_TEXT(" params=\"");
            s << parameters;
            s << '"';
        }
        s << ACE_TEXT("/>");
        s << ACE_TEXT("</dynamic>");
        s << ACE_TEXT("</ACE_Svc_Conf>");
#endif
        std::ends(s); return s.str();
    }

    std::string
    ServiceGestalt::suspend_directive(const ACE_TCHAR *ident)
    {
        std::stringstream s;

#if (ACE_USES_CLASSIC_SVC_CONF == 1)
        s << ACE_TEXT("suspend ") << ident;
#else
        s << ACE_TEXT("<ACE_Svc_Conf>");
        s << ACE_TEXT("<suspend id=\"");
        s << ident;
        s << ACE_TEXT("\">");
        s << ACE_TEXT("</suspend>");
        s << ACE_TEXT("</ACE_Svc_Conf>");
#endif
        std::ends(s); return s.str();
    }

    std::string
    ServiceGestalt::resume_directive(const ACE_TCHAR *ident)
    {
        std::stringstream s;

#if (ACE_USES_CLASSIC_SVC_CONF == 1)
        s << ACE_TEXT("resume ") << ident;
#else
        s << ACE_TEXT("<ACE_Svc_Conf>");
        s << ACE_TEXT("<resume id=\"");
        s << ident;
        s << ACE_TEXT("\">");
        s << ACE_TEXT("</resume>");
        s << ACE_TEXT("</ACE_Svc_Conf>");
#endif
        std::ends(s); return s.str();
    }

    std::string
    ServiceGestalt::remove_directive(const ACE_TCHAR *ident)
    {
        std::stringstream s;

#if (ACE_USES_CLASSIC_SVC_CONF == 1)
        s << ACE_TEXT("remove ") << ident;
#else
        s << ACE_TEXT("<ACE_Svc_Conf>");
        s << ACE_TEXT("<remove id=\"");
        s << ident;
        s << ACE_TEXT("\">");
        s << ACE_TEXT("</remove>");
        s << ACE_TEXT("</ACE_Svc_Conf>");
#endif
        std::ends(s); return s.str();
    }

    /************************************************************************************/

} // namespace DAF
