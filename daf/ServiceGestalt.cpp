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
#include "TaskExecutor.h"
#include "DateTime.h"

#include <ace/Service_Types.h>
#include <ace/Min_Max.h>

namespace {

    const int DEFAULT_MAXIMUM_TIMEOUT       = 60;    // seconds
    const int DEFAULT_LOADYNAMIC_TIMEOUT    = 15;    // seconds
    const int DEFAULT_LOADSTATIC_TIMEOUT    = 10;    // seconds
    const int DEFAULT_REMOVE_TIMEOUT        = 6;     // seconds
    const int DEFAULT_RESUME_TIMEOUT        = 5;     // seconds
    const int DEFAULT_SUSPEND_TIMEOUT       = 4;     // seconds
    const int DEFAULT_MINIMUM_TIMEOUT       = 2;     // seconds

    const char INVALID_IDENT_CHARS []       = ACE_TEXT(" :.#%*~-(){}\\\'\"");

    time_t  get_activity_timeout(int min_timeout, int max_timeout = DEFAULT_MAXIMUM_TIMEOUT) // Returns milliseconds
    {
        return time_t(DAF_MSECS_ONE_SECOND * ace_range(min_timeout, max_timeout, DAF::get_numeric_property<int>(DAF_SVCACTIONTIMEOUT, DAF_DEFAULT_SERVICE_ACTION_TIMEOUT, false)));
    }
}

namespace DAF
{
    /****************************************************************************************/

    std::string
    locateServiceIdent(const ACE_Service_Object * svc_obj, const ACE_Service_Gestalt * sg)
    {
        ACE_Service_Repository *repo = (sg ? const_cast<ACE_Service_Gestalt *>(sg)->current_service_repository() : 0);

        if (repo && svc_obj) {

            do {
                ACE_GUARD_REACTION(ACE_Recursive_Thread_Mutex, mon, ServiceGestalt::repository_lock(repo), break);

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
        }

        DAF_THROW_EXCEPTION(NotFoundException);
    }

    /****************************************************************************************/

    void
    print_gestalt(const ACE_Service_Gestalt * sg)
    {
        ACE_Service_Repository * repo = (sg ? const_cast<ACE_Service_Gestalt *>(sg)->current_service_repository() : 0);

        if (repo) {

            do { // Scope lock
                ACE_GUARD_REACTION(ACE_Recursive_Thread_Mutex, mon, ServiceGestalt::repository_lock(repo), break);

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
    }

    /****************************************************************************************/

    class LoadActivity : public ServiceGestalt::ServiceActivity
    {
    protected:
        LoadActivity(ServiceGestalt *gestalt, const ServiceGestalt::service_descriptor_type &sd)
            : ServiceActivity(gestalt), sd_(sd) {}
        virtual int run(const std::string &directive);
    protected:
        ServiceGestalt::service_descriptor_type sd_;
    };

    class SuspendActivity : public ServiceGestalt::ServiceActivity
    {
        const ServiceGestalt::ident_type ident_;
    public:
        SuspendActivity(ServiceGestalt *gestalt, const ServiceGestalt::ident_type &ident)
            : ServiceActivity(gestalt), ident_(ident) {}
        virtual int run(void);
    };

    class ResumeActivity : public ServiceGestalt::ServiceActivity
    {
        const ServiceGestalt::ident_type ident_;
    public:
        ResumeActivity(ServiceGestalt *gestalt, const ServiceGestalt::ident_type &ident)
            : ServiceActivity(gestalt), ident_(ident) {}
        virtual int run(void);
    };

    class RemoveActivity : public ServiceGestalt::ServiceActivity
    {
        const ServiceGestalt::ident_type ident_;
    public:
        RemoveActivity(ServiceGestalt *gestalt, const ServiceGestalt::ident_type &ident)
            : ServiceActivity(gestalt), ident_(ident) {}
        virtual int run(void);
    };

    /****************************************************************************************/

    ServiceGestalt::ServiceGestalt(const ACE_TCHAR *program_name) : ACE_Service_Gestalt(DEFAULT_SIZE, true, false)
        , loadTime_(DAF_Date_Time::GMTime())
    {
        ACE_Service_Gestalt::intrusive_add_ref(this);
        ACE_Service_Gestalt::open(program_name, 0, true, true, false); // Open the Gestalt
    }

    ServiceGestalt::~ServiceGestalt(void)
    {
        ACE_Service_Config_Guard svc_guard(this); ACE_UNUSED_ARG(svc_guard);
        ACE_Service_Gestalt::close();
    }

    const char *
    ServiceGestalt::config_switch(void) const
    {
        ACE_ERROR((LM_ERROR, ACE_TEXT("DAF (%P | %t) ERROR ServiceGestalt:config_switch()\n")
            ACE_TEXT("\t- Configuration possibly not valid for operation.\n")));
        DAF_OS::last_error(ENOEXEC);
        return 0;
    }

    int
    ServiceGestalt::loadStatic(const std::string & ident, const std::string & parameters)
    {
        struct LoadStaticActivity : LoadActivity
        {
            LoadStaticActivity(ServiceGestalt *gestalt, const service_descriptor_type &sd)
                : LoadActivity(gestalt, sd) {}
            virtual int run(void)
            {
                return LoadActivity::run(ServiceGestalt::static_directive(this->sd_.ident_,this->sd_.parameters_));
            }
        };

        const ident_type svc_ident(DAF::trim_string(ident));

        do {

            // Checking for invalid ident
            if (svc_ident.length() == 0) {
                ACE_ERROR((LM_ERROR, ACE_TEXT("DAF (%P|%t) ServiceGestalt - Zero length ident provided\n")));
                DAF_OS::last_error(EINVAL); break;
            }

            int pos = int(svc_ident.find_first_of(INVALID_IDENT_CHARS));

            if (pos >= 0) {
                ACE_ERROR((LM_ERROR, ACE_TEXT("DAF (%P|%t) ServiceGestalt - Invalid characters (%C) found in ident '%C' (pos=%d)\n")
                    , INVALID_IDENT_CHARS, svc_ident.c_str(), pos));
                DAF_OS::last_error(EINVAL); break;
            }

            const std::string svc_params(DAF::format_args(parameters, true, true));

            const service_descriptor_type svc_descriptor = {
                svc_ident, "", "", svc_params, DAF_OS::gettimeofday(), (LOAD_STATIC | LOAD_OBJECT)
            };

            if (this->process_svc_action(new LoadStaticActivity(this, svc_descriptor), get_activity_timeout(DEFAULT_LOADSTATIC_TIMEOUT))) {
                break;
            }

            return 0; // All Done

        } while (false);

        if (DAF::debug()) {
            ACE_DEBUG((LM_WARNING, ACE_TEXT("DAF::GestaltService - Failed to load static service '%s' %s\n")
                , svc_ident.c_str(), DAF::last_error_text().c_str()));
        }

        return -1;
    }

    int
    ServiceGestalt::loadDynamic(const std::string & ident, const std::string & libpathname, const std::string & objectclass, const std::string & parameters)
    {
        struct LoadDynamicActivity : LoadActivity
        {
            LoadDynamicActivity(ServiceGestalt * gestalt, const service_descriptor_type & sd)
                : LoadActivity(gestalt, sd) {}
            virtual int run(void)
            {
                return LoadActivity::run(ServiceGestalt::dynamic_directive(this->sd_.ident_,this->sd_.libpathname_,this->sd_.objectclass_,this->sd_.parameters_));
            }
        };

        const ident_type svc_ident(DAF::trim_string(ident));

        do {

            // Checking for invalid ident
            if (svc_ident.length() == 0) {
                ACE_ERROR((LM_ERROR, ACE_TEXT("DAF (%P|%t) ServiceGestalt - Zero length ident provided\n")));
                DAF_OS::last_error(EINVAL); break;
            }

            int pos = int(svc_ident.find_first_of(INVALID_IDENT_CHARS));

            if (pos >= 0) {
                ACE_ERROR((LM_ERROR, ACE_TEXT("DAF (%P|%t) ServiceGestalt - Invalid characters (%C) found in ident '%C' (pos=%d)\n")
                    , INVALID_IDENT_CHARS, svc_ident.c_str(), pos));
                DAF_OS::last_error(EINVAL); break;
            }

            const std::string svc_objclass(DAF::trim_string(objectclass));

            if (svc_objclass.length() ? int(svc_objclass.find_first_of(' ')) > 0 : true) {
                DAF_OS::last_error(EINVAL); break;
            }

            std::string svc_libpath(DAF::parse_args(libpathname, true));

            if (svc_libpath.length() == 0) {
                DAF_OS::last_error(EINVAL); break;
            }

            // Cleanup libpath
            for (int pos = int(svc_libpath.length()); pos-- > 0;) {

                switch (svc_libpath[0]) {
                case '\"': case '\'': case ' ':
                    svc_libpath.erase(0, 1); continue; // Balance erase from both ends
                default:
                    if (::iscntrl(svc_libpath[0])) {
                        svc_libpath.erase(0, 1); continue; // Balance erase from both ends
                    }
                    break;
                }

                if (pos) {
                    switch (svc_libpath[pos]) {
                    case '\"': case '\'': case ' ':
                        svc_libpath.erase(pos, 1); continue; // Balance erase from both ends
                    default:
                        if (::iscntrl(svc_libpath[pos])) {
                            svc_libpath.erase(pos, 1); continue; // Balance erase from both ends
                        }
                        break;
                    }

                    while (pos--) {
                        if (::iscntrl(svc_libpath[pos])) { svc_libpath[pos] = ' '; }
                    }
                }

                break;
            }

            // Quote libpath if required
            if (svc_libpath.length() == 0) {
                DAF_OS::last_error(EINVAL); break;
            }
            else if (int(svc_libpath.find_first_of(' ')) > 0) {
                svc_libpath.insert(0, "'").append("'"); // Insert quotes around embedded spaced libpath
            }

            const std::string svc_params(DAF::format_args(parameters, true, true));

            const service_descriptor_type svc_descriptor = {
                svc_ident, svc_libpath, svc_objclass, svc_params, DAF_OS::gettimeofday(), (LOAD_DYNAMIC | LOAD_OBJECT)
            };

            if (this->process_svc_action(new LoadDynamicActivity(this, svc_descriptor), get_activity_timeout(DEFAULT_LOADYNAMIC_TIMEOUT))) {
                break;
            }

            return 0; // All Done

        } while (false);

        if (DAF::debug()) {
            ACE_DEBUG((LM_WARNING, ACE_TEXT("DAF::ServiceGestalt - Failed to load dynamic service '%s' %s\n")
                , svc_ident.c_str(), DAF::last_error_text().c_str()));
        }

        return -1;
    }

    int
    ServiceGestalt::suspendService(const std::string & ident)
    {
        const ident_type svc_ident(DAF::trim_string(ident));

        do {

            // Checking for invalid ident
            if (svc_ident.length() == 0) {
                ACE_ERROR((LM_ERROR, ACE_TEXT("DAF (%P|%t) ServiceGestalt - Zero length ident provided\n")));
                DAF_OS::last_error(EINVAL); break;
            }

            int pos = int(svc_ident.find_first_of(INVALID_IDENT_CHARS));

            if (pos >= 0) {
                ACE_ERROR((LM_ERROR, ACE_TEXT("DAF (%P|%t) ServiceGestalt - Invalid characters (%C) found in ident '%C' (pos=%d)\n")
                    , INVALID_IDENT_CHARS, svc_ident.c_str(), pos));
                DAF_OS::last_error(EINVAL); break;
            }

            if (this->process_svc_action(new SuspendActivity(this, svc_ident), get_activity_timeout(DEFAULT_SUSPEND_TIMEOUT))) {
                break;
            }

            return 0; // All Done

        } while (false);

        if (DAF::debug()) {
            ACE_DEBUG((LM_WARNING, ACE_TEXT("DAF::ServiceGestalt - Failed to suspend service '%s' %s\n")
                , svc_ident.c_str(), DAF::last_error_text().c_str()));
        }

        return -1;
    }

    int
    ServiceGestalt::resumeService(const std::string & ident)
    {
        const ident_type svc_ident(DAF::trim_string(ident));

        do {

            // Checking for invalid ident
            if (svc_ident.length() == 0) {
                ACE_ERROR((LM_ERROR, ACE_TEXT("DAF (%P|%t) ServiceGestalt - Zero length ident provided\n")));
                DAF_OS::last_error(EINVAL); break;
            }

            int pos = int(svc_ident.find_first_of(INVALID_IDENT_CHARS));

            if (pos >= 0) {
                ACE_ERROR((LM_ERROR, ACE_TEXT("DAF (%P|%t) ServiceGestalt - Invalid characters (%C) found in ident '%C' (pos=%d)\n")
                    , INVALID_IDENT_CHARS, svc_ident.c_str(), pos));
                DAF_OS::last_error(EINVAL); break;
            }

            if (this->process_svc_action(new ResumeActivity(this, svc_ident), get_activity_timeout(DEFAULT_RESUME_TIMEOUT))) {
                break;
            }

            return 0; // All Done

        } while (false);

        if (DAF::debug()) {
            ACE_DEBUG((LM_WARNING, ACE_TEXT("DAF::ServiceGestalt - Failed to resume service '%s' %s\n")
                , svc_ident.c_str(), DAF::last_error_text().c_str()));
        }

        return -1;
    }

    int
    ServiceGestalt::removeService(const std::string & ident)
    {
        const ident_type svc_ident(DAF::trim_string(ident));

        do {

            // Checking for invalid ident
            if (svc_ident.length() == 0) {
                ACE_ERROR((LM_ERROR, ACE_TEXT("DAF (%P|%t) ServiceGestalt - Zero length ident provided\n")));
                DAF_OS::last_error(EINVAL); break;
            }

            int pos = int(svc_ident.find_first_of(INVALID_IDENT_CHARS));

            if (pos >= 0) {
                ACE_ERROR((LM_ERROR, ACE_TEXT("DAF (%P|%t) ServiceGestalt - Invalid characters (%C) found in ident '%C' (pos=%d)\n")
                    , INVALID_IDENT_CHARS, svc_ident.c_str(), pos));
                DAF_OS::last_error(EINVAL); break;
            }

            if (this->process_svc_action(new RemoveActivity(this, svc_ident), get_activity_timeout(DEFAULT_REMOVE_TIMEOUT))) {
                break;
            }

            return 0; // All Done

        } while (false);

        if (DAF::debug()) {
            ACE_DEBUG((LM_WARNING, ACE_TEXT("DAF::ServiceGestalt - Failed to remove service '%s' %s\n")
                , svc_ident.c_str(), DAF::last_error_text().c_str()));
        }

        return -1;
    }

    int
    ServiceGestalt::suspendServiceAll(void)
    {
        service_list_type svc_list;

        int fails = 0;
        if (this->list_service_repository(svc_list, true) > 0) {
            for (service_list_type::const_iterator it(svc_list.begin()); it != svc_list.end(); it++) {
                if (0 > this->suspendService(it->ident_)) {
                    fails++;
                }
            }
        }

        return fails;
    }

    int
    ServiceGestalt::resumeServiceAll(void)
    {
        service_list_type svc_list;

        int fails = 0;
        if (this->list_service_repository(svc_list, true) > 0) {
            for (service_list_type::const_iterator it(svc_list.begin()); it != svc_list.end(); it++) {
                if (0 > this->resumeService(it->ident_)) {
                    fails++;
                }
            }
        }

        return fails;
    }

    int
    ServiceGestalt::removeServiceAll(void)
    {
        service_list_type svc_list;

        int fails = 0;
        if (this->list_service_repository(svc_list, true) > 0) {
            for (service_list_type::const_iterator it(svc_list.begin()); it != svc_list.end(); it++) {
                if (0 > this->removeService(it->ident_)) {
                    fails++;
                }
            }
        }

        return fails;
    }

    int
    ServiceGestalt::process_svc_action(const ServiceActivity_ref & svc_action, time_t timeout)
    {
        try { // Touch User Code (init)
            if (this->execute_svc_action(svc_action) == 0) {
                return svc_action->wait_result(timeout);
            }
        } DAF_CATCH_ALL {
            // Failure in user code??
        }

        DAF_OS::last_error(ENOEXEC); return -1; // Not Executable
    }

    int
    ServiceGestalt::list_service_repository(service_list_type &svc_list, bool reverse) const
    {
        svc_list.clear();

        ACE_READ_GUARD_RETURN(ACE_SYNCH_RW_MUTEX, mon, this->svc_descriptors(), 0);

        ACE_Service_Repository *repo = const_cast<ServiceGestalt *>(this)->current_service_repository();

        if (repo) do {

            ACE_GUARD_REACTION(ACE_Recursive_Thread_Mutex, mon, ServiceGestalt::repository_lock(repo), { DAF_OS::last_error(ENOLCK); break; });

            ACE_Service_Repository_Iterator repo_it(*repo, false);

            for (const ACE_Service_Type *svc_type = 0; repo_it.next(svc_type); repo_it.advance()) {
                service_map_type::const_iterator it = this->svc_descriptors().find(svc_type->name());
                if (it != this->svc_descriptors().end()) {
                    reverse ? svc_list.push_front(it->second) : svc_list.push_back(it->second);
                }
            }

        } while (false);

        return int(svc_list.size());
    }

    std::string
    ServiceGestalt::static_directive(const std::string &ident, const std::string &parameters)
    {
        std::stringstream s;

#if (ACE_USES_CLASSIC_SVC_CONF == 1)
        s << ACE_TEXT("static ");
        s << ident;
        s << ACE_TEXT(" \"");
        if (parameters.length()) {
            s << parameters;
        }
        s << '"';
#else
        s << ACE_TEXT("<ACE_Svc_Conf>");
        s << ACE_TEXT("<static id=\"");
        s << ident;
        s << '"';
        if (parameters.length()) {
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
    ServiceGestalt::dynamic_directive(const std::string &ident, const std::string &libpathname, const std::string &objectclass, const std::string &parameters)
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
        if (parameters.length()) {
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
        if (parameters.length()) {
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
    ServiceGestalt::suspend_directive(const std::string &ident)
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
    ServiceGestalt::resume_directive(const std::string &ident)
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
    ServiceGestalt::remove_directive(const std::string &ident)
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

    ACE_Recursive_Thread_Mutex &
    ServiceGestalt::repository_lock(const ACE_Service_Repository *repo) // Hack to get hold of Repository Lock
    {
#if defined(ACE_HAS_SERVICE_REPOSITORY_LOCK_ACCESS)
        if (repo) {
            return repo->lock();
        }
#else
        // Hack to gain access to Repository Lock - relies on ACE_Service_Repository being non-polymorphic
        struct Service_Repository : ACE_Service_Repository {
            ACE_Recursive_Thread_Mutex & _repositoryLock(void) const
            {
                return const_cast<Service_Repository *>(this)->lock_;
            }
        };

        if (repo) {
            for (const Service_Repository *repo_hack = reinterpret_cast<const Service_Repository *>(repo); repo_hack;) {
                return repo_hack->_repositoryLock(); // Hack
            }
        }
#endif
        DAF_THROW_EXCEPTION(LockFailureException);
    }

    int
    ServiceGestalt::execute_svc_action(const DAF::Runnable_ref & command)
    {
        return SingletonExecute(command);
    }

    /****************************************************************************************************************/

    ServiceGestalt::ServiceActivity::ServiceActivity(ServiceGestalt *gestalt) : DAF::Runnable()
        , resultLock_   (0)   // Initially Locked
        , gestalt_      (gestalt)
        , result_       (-1)
        , error_        (0)
    {
    }

    int
    ServiceGestalt::ServiceActivity::process_directive(const std::string &directive)
    {
        ACE_Service_Config_Guard svc_guard(this->gestalt_); ACE_UNUSED_ARG(svc_guard);
        return this->result_ = this->gestalt_->process_directive(directive.c_str());
    }

    int
    ServiceGestalt::ServiceActivity::wait_result(time_t msecs) const
    {
        int result = -1;

        const ACE_Time_Value timeout(DAF_OS::gettimeofday(ace_max(msecs, time_t(0))));

        if (this->resultLock_.acquire(timeout) == 0) {
            if ((result = this->result_) != 0) {
                DAF_OS::last_error(this->error_);
            }
            ACE_Errno_Guard g(errno); ACE_UNUSED_ARG(g);
            this->resultLock_.release();
        }

        return result;
    }

    /*************************************************************************************/

    int
    LoadActivity::run(const std::string & directive)
    {
        const std::string svc_ident(this->sd_.ident_);

        do {  // Scope Lock

            ACE_WRITE_GUARD_REACTION(ACE_SYNCH_RW_MUTEX, gestalt_mon, this->svc_descriptors(), { DAF_OS::last_error(ENOLCK); break; } );

            const ACE_Service_Type *svc_type = 0;

            if (this->gestalt_->find(svc_ident.c_str(), &svc_type, false) == 0) {
                DAF_OS::last_error(EEXIST);
            }
            else if (this->process_directive(directive)) {
                break;
            }
            else for (ServiceGestalt::service_map_Pairib ib(this->svc_descriptors().insert(ServiceGestalt::service_map_type::value_type(svc_ident, this->sd_)));ib.second;) {
                ACE_SET_BITS(ib.first->second.loadflags_, ServiceGestalt::LOAD_ACTIVE); break;
            }

        } while (false);

        this->error_ = DAF_OS::last_error();
        this->resultLock_.release();
        return 0;
    }

    int
    SuspendActivity::run(void)
    {
        do {  // Scope Lock

            ACE_READ_GUARD_REACTION(ACE_SYNCH_RW_MUTEX, mon, this->svc_descriptors(), { DAF_OS::last_error(ENOLCK); break; });

            const ACE_Service_Type *svc_type = 0;

            if (this->gestalt_->find(this->ident_.c_str(), &svc_type, false)) {
                DAF_OS::last_error(EEXIST);
            }
            else if (svc_type ? svc_type->fini_called() : true) {
                DAF_OS::last_error(EACCES);
            }
            else if (!svc_type->active()) {
                DAF_OS::last_error(this->result_ = 0); // No-op if already suspended
            }
            else if (this->process_directive(ServiceGestalt::suspend_directive(this->ident_))) {

                const_cast<ACE_Service_Type*>(svc_type)->active(true); // Set back to active

                switch (DAF_OS::last_error()) {
                case EINVAL: DAF_OS::last_error(ENOTSUP); break;
                }
            }
            else for (ServiceGestalt::service_map_type::iterator it = svc_descriptors().find(this->ident_); it != svc_descriptors().end();) {
                ACE_CLR_BITS(it->second.loadflags_, ServiceGestalt::LOAD_ACTIVE); break;
            }

        } while (false);

        this->error_ = DAF_OS::last_error();
        this->resultLock_.release();
        return 0;
    }

    int
    ResumeActivity::run(void)
    {
        do {  // Scope Lock

            ACE_READ_GUARD_REACTION(ACE_SYNCH_RW_MUTEX, mon, this->svc_descriptors(), { DAF_OS::last_error(ENOLCK); break; });

            const ACE_Service_Type *svc_type = 0;

            if (this->gestalt_->find(this->ident_.c_str(), &svc_type, false)) {
                DAF_OS::last_error(EEXIST);
            }
            else if (svc_type ? svc_type->fini_called() : true) {
                DAF_OS::last_error(EACCES);
            }
            else if (svc_type->active()) {
                DAF_OS::last_error(this->result_ = 0); // No-op if already active
            }
            else if (this->process_directive(ServiceGestalt::resume_directive(this->ident_))) {

                const_cast<ACE_Service_Type*>(svc_type)->active(false); // Set back to in-active

                switch (DAF_OS::last_error()) {
                case EINVAL: DAF_OS::last_error(ENOTSUP); break;
                }

            }
            else for (ServiceGestalt::service_map_type::iterator it = svc_descriptors().find(this->ident_); it != svc_descriptors().end();) {
                ACE_SET_BITS(it->second.loadflags_, ServiceGestalt::LOAD_ACTIVE); break;
            }

        } while (false);

        this->error_ = DAF_OS::last_error();
        this->resultLock_.release();
        return 0;
    }

    int
    RemoveActivity::run(void)
    {
        do { // Scope Lock

            ACE_WRITE_GUARD_REACTION(ACE_SYNCH_RW_MUTEX, mon, this->svc_descriptors(), { DAF_OS::last_error(ENOLCK); break; });

            const ACE_Service_Type *svc_type = 0;

            if (this->gestalt_->find(this->ident_.c_str(), &svc_type, false)) {
                DAF_OS::last_error(EEXIST); break;
            }
            else if (svc_type ? svc_type->fini_called() : true) {
                DAF_OS::last_error(EACCES); break;
            }

            this->process_directive(ServiceGestalt::remove_directive(this->ident_));

            switch (this->svc_descriptors().erase(this->ident_) ? DAF_OS::last_error() : (DAF_OS::last_error(ENOENT), 0)) { // remove our local entry regardless of any process_directive() error
            case 0: case EINVAL:
                this->result_ = 0;  // Dont say we failed here as Gestalt itself will ignore return from service->fini() anyway
            default:
                break;
            }

        } while (false);

        this->error_ = DAF_OS::last_error();
        this->resultLock_.release();
        return 0;
    }

} // namespace DAF
