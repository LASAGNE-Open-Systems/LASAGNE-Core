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
#define TAF_TAFSERVER_IMPL_CPP

#include "TAFServer_impl.h"

#include "TAFServer.h"
#include "ORBManager.h"

//#include "IORQueryService.h"
#include "IORQueryRepository.h"
#include "RepoQOS.h"

#include "ObjectStubRef_T.h"
#include "GestaltService_impl.h"
#include "TAFDebug.h"
#include "CDRStream.h"

#if defined(TAF_HAS_SECURITY)
# include <TAFSecurity/TAFSecurity.h>
#endif

#include <tao/PI/ORBInitInfo.h>
#include <tao/ORBInitializer_Registry.h>

#include <daf/RefCount.h>
#include <daf/PropertyManager.h>
#include <daf/ShutdownHandler.h>

#include <ace/Arg_Shifter.h>

#include <list>

namespace { // Anonymous namespace

    ACE_FACTORY_DEFINE(TAF, TAFServerImpl);

    const ACE_Static_Svc_Descriptor TAFServerImplDescriptor = {
        TAFServerImpl::svc_ident(),
        ACE_SVC_OBJ_T,
        &ACE_SVC_NAME(TAFServerImpl),
        (ACE_Service_Type::DELETE_THIS | ACE_Service_Type::DELETE_OBJ),
        false // Service inactive
    };

    const char * tafServerName(void)
    {
        static const struct TAFServerName : std::string {
            TAFServerName(const std::string &def_name)
            {
                try {
                    this->assign(DAF::trim_string(DAF::get_property(TAF_SERVERNAME, def_name, true), '\''));
                } catch( const DAF::NotFoundException & ) {
                    DAF::set_property(TAF_SERVERNAME, this->assign(DAF::format_args(def_name)));
                }
            }
        } taf_server_name_(std::string(TAFServerImpl::svc_ident()).append(":%P"));

        return taf_server_name_.c_str();
    }

} // End Anonymous namespace

namespace TAF
{
    TAFServer_impl::TAFServer_impl(void) : TAFServerInterfaceHandler()
        , TAF::GestaltService_impl(TAFServerImpl::svc_ident())
    {
    }

    TAFServer_impl::~TAFServer_impl(void)
    {
    }

    char *
    TAFServer_impl::svc_name(void)
    {
        return CORBA::string_dup(tafServerName());
    }

    int
    TAFServer_impl::parse_args(int argc, ACE_TCHAR *argv[])
    {
        ACE_UNUSED_ARG(argc); ACE_UNUSED_ARG(argv); return 0;
    }

    taf::EntityDescriptor *
    TAFServer_impl::entity_descriptor(void)
    {
        const CORBA::UShort ed_flags(CORBA::UShort((1U << taf::SVC_OBJECT) | (1U << taf::SVC_STATIC) | (1U << taf::SVC_ACTIVE) | (1U << taf::SVC_EXECUTE)));

        taf::EntityDescriptor_var ed(new taf::EntityDescriptor);

        ed->loadTime_.sec   = CORBA::ULongLong(this->loadTime_.sec());
        ed->loadTime_.usec  = CORBA::ULong(this->loadTime_.usec());

        const std::string server_args(DAF::get_property(TAF_SERVER_ARGS));

        ACE_ARGV  args(server_args.c_str());

        for (int arg_c = args.argc(); arg_c--;) {
            ACE_TCHAR **arg_v = args.argv();
            const std::string argv0(*arg_v++); if (arg_c) {
                ed->parameters_ = DAF::parse_args(arg_c, arg_v).c_str();
            }
            if (argv0.length() > 0) {
                int rpos = int(argv0.find_last_of('/')), lpos = int(argv0.find_last_of('\\')), pos = ace_max(rpos, lpos);
                if (pos > 0) {
                    ed->libpathname_ = argv0.substr(0, pos++).c_str();
                }
                else pos = 0;
                ed->objectclass_ = argv0.substr(pos).c_str();
            }
            break;
        }

        ed->obj_    = TAFServerImpl::_interface_activator_type::stub_reference(this)._retn();
        ed->ident_ = tafServerName();

        if (this->info(&ed->info_.out(), BUFSIZ)) {
            ed->info_ = "";
        }

        ed->flags_ = ed_flags;

        return ed._retn();
    }

    void
    TAFServer_impl::sendConsoleMsg(const char *msg)
    {
        if (msg && DAF_OS::strlen(msg) > 0) {
            ACE_DEBUG((LM_INFO, ACE_TEXT("\nINFO: %s[%s @ %D]\n'%s'\n")
                , TAFServerImpl::svc_ident()
                , tafServerName()
                , msg));
        }
    }

    void
    TAFServer_impl::shutdown(void)
    {
        if (!DAF::ShutdownHandler::has_shutdown()) {
            DAF::ShutdownHandler::send_shutdown(); // Collaborates with TAF::ORB::run()
            ACE_DEBUG((LM_INFO, ACE_TEXT("\nINFO: %s[%s @ %D] shutdown requested.\n")
                , TAFServerImpl::svc_ident()
                , tafServerName()));
        }
    }

    char *
    TAFServer_impl::getHostName(void)
    {
        return CORBA::string_dup(DAF_OS::gethostname().c_str());
    }

    char *
    TAFServer_impl::getRepoQOS(CORBA::Object_ptr obj, CORBA::Long_out time)
    {
        return CORBA::string_dup(TAF::getRepoQOS(obj, time).c_str());
    }

    int
    TAFServer_impl::init(int argc, ACE_TCHAR *argv[])
    {
        if (this->parse_args(argc, argv) == 0) do try {

            {
                const PortableServer::ObjectId_var tafServerOID(PortableServer::string_to_ObjectId(taf::TAFSERVER_OID));
                if (this->init_bind(TAFServerImpl::svc_ident(), tafServerOID.in())) {
                    break;
                }
            }

#if defined(TAF_HAS_SECURITY)
            if (TAF::isSecurityActive() && TAFSecurityAccessDecision().set_servant_access(this, true)) {
                ACE_DEBUG((LM_WARNING, ACE_TEXT("TAFServer_impl (%P | %t) TAFSecurityAccessDecision:")
                    ACE_TEXT(" Unable to allow insecure access for %s.\n")
                    , tafServerName()));
            }
#endif
            GestaltServiceLoader svcLoader(*this);

            int svc_loads = svcLoader.load_config_args(argc, argv);

            do {

                if (svc_loads > 0) {

                    int svc_failures = svcLoader.process_directives();

                    if (0 > svc_failures) { // Big failure in process_directives()
                        DAF_THROW_EXCEPTION(DAF::InvocationTargetException);
                    } else if (svc_failures) { // Some Load failures
                        ACE_ERROR_BREAK((LM_INFO, ACE_TEXT("TAFServerImpl (%P | %t) WARNING: ")
                            ACE_TEXT("%s failed to load %d service entries - Ignored.\n")
                            , tafServerName(), svc_failures));
                    }

                    break; // No Loading Problems
                }

                if (TAF::debug() > 1) {
                    ACE_DEBUG((LM_WARNING, ACE_TEXT("TAFServerImpl (%P | %t) WARNING: ")
                        ACE_TEXT("%s configured without any loadable service entities. - Ignored.\n")
                        , tafServerName()));
                }

            } while (false);

            return 0;

        } DAF_CATCH_ALL {
            /* FAll Through to Error */
        } while (false);

        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("TAFServer (%P | %t) ERROR:")
            ACE_TEXT(" Failed to initialize %s.\n")
            , tafServerName()), -1);
    }

    int
    TAFServer_impl::suspend(void)
    {
        DAF_OS::last_error(ENOTSUP); return -1;
    }

    int
    TAFServer_impl::resume(void)
    {
        DAF_OS::last_error(ENOTSUP); return -1;
    }

    int
    TAFServer_impl::fini(void)
    {
        this->fini_bind(); return 0;
    }

    int
    TAFServer_impl::info(ACE_TCHAR **info_string, size_t length) const
    {
        static const char *info_desc =
        {
            "The TAFServer is the primary process interface into the TAF framework services."
        };
        return (info_string ? (*info_string = ACE::strnnew(info_desc, length), 0) : -1);
    }
} // namespace TAF

/*********************************************************************************************************/

TAFServer::TAFServer(void)
{
}

TAFServer::TAFServer(int argc, ACE_TCHAR *argv[]) : TAF::ORBManager()
{
    if (this->init(argc, argv)) {
        DAF_THROW_EXCEPTION(DAF::InitializationException);
    }
}

TAFServer::~TAFServer(void)
{
    this->module_closed();
}

int
TAFServer::run(bool wait_for_completion)
{
    return this->isAvailable() ? TAF::ORBManager::run(this->orb_threads(), wait_for_completion) : -1;
}

int
TAFServer::init(int argc, ACE_TCHAR *argv[])
{
    try {
        ACE_Service_Config::process_directive(TAFServerImplDescriptor);
        if (TAF::ORBManager::init(argc, argv) == 0) {
            if (ACE_Service_Config::initialize(TAFServerImpl::svc_ident(), DAF::get_property(TAF_SERVICE_ARGS).c_str()) == 0) {
                return 0;
            }
        }
    } DAF_CATCH_ALL { /* We had some error */ }

    ACE_ERROR_RETURN((LM_ERROR,
        ACE_TEXT("TAFServer (%P | %t) ERROR: Unable to initialize %s with service implementation.\n")
        , TAFServer::svc_ident()), -1);
}

int
TAFServer::fini(void)
{
    return this->module_closed();
}

int
TAFServer::info(ACE_TCHAR **info_string, size_t length) const
{
    static const char *info_desc =
    {
        "The TAFServer is the external framework hosting entry point into the TAF framework services."
    };
    return (info_string ? (*info_string = ACE::strnnew(info_desc, length), 0) : -1);
}

int
TAFServer::close(u_long flags)
{
    if (flags) {
        DAF::ShutdownHandler::send_shutdown();
        {
            ACE_Service_Config_Guard svc_guard(TheTAFOrbCore()->configuration()); ACE_UNUSED_ARG(svc_guard);
            if (ACE_Service_Config::remove(TAFServerImpl::svc_ident())) {  // Remove our sentinal service
                ACE_DEBUG((LM_ERROR,
                    ACE_TEXT("TAF (%P | %t) TAFServer::close(%d) : Unable to remove service '%s'.\n")
                    , int(flags), TAFServerImpl::svc_ident()));
            }
        }
    }

    return TAF::ORBManager::close(flags);
}

/********************************************************************************************/

namespace {

    ACE_FACTORY_DEFINE(TAF, TAFServer);

    const ACE_Static_Svc_Descriptor TAFServerDescriptor = {
        TAFServer::svc_ident(),
        ACE_SVC_OBJ_T,
        &ACE_SVC_NAME(TAFServer),
        (ACE_Service_Type::DELETE_THIS | ACE_Service_Type::DELETE_OBJ),
        true // Service active
    };

    class TAFServerInitializer : public ACE_Task_Base, public DAF::RefCount
    {
        virtual int svc(void);

    public:

        TAFServerInitializer(const std::string &args);
        virtual ~TAFServerInitializer(void);

        int wait_result(time_t timeout);

        bool isActive(void) const
        {
            return this->initActive_;
        }

        bool isAvailable(void) const
        {
            return this->thr_count() == 0;
        }

        DAF_DEFINE_REFCOUNTABLE(TAFServerInitializer);

    protected:

        virtual int close(u_long flags);

    private:

        volatile bool           initActive_;
        volatile int            initResult_;
        mutable ACE_Semaphore   initLock_;
    };

    DAF_DECLARE_REFCOUNTABLE(TAFServerInitializer);

    TAFServerInitializer::TAFServerInitializer(const std::string &args) : ACE_Task_Base()
        , initActive_   (false)
        , initResult_   (-1)
        , initLock_     (0)
    {
        ACE_Service_Config_Guard svc_guard(ACE_Service_Config::global()); ACE_UNUSED_ARG(svc_guard);

        if (ACE_Service_Config::process_directive(TAFServerDescriptor, false) == 0) {
            if (ACE_Service_Config::initialize(TAFServer::svc_ident(), args.c_str()) == 0) {
                DAF_OS::thr_yield(); return; // Allow a little for TAFServer to load-up
            }
        }

        DAF_THROW_EXCEPTION(DAF::InitializationException);
    }

    TAFServerInitializer::~TAFServerInitializer(void)
    {
        this->module_closed(); this->wait();
    }

    int
    TAFServerInitializer::close(u_long flags)
    {
        if (this->initActive_ && flags) {
            this->initActive_ = false;
            DAF::ShutdownHandler::send_shutdown(true);
            {
                ACE_Service_Config_Guard svc_guard(ACE_Service_Config::global()); ACE_UNUSED_ARG(svc_guard);
                ACE_Service_Config::remove(TAFServer::svc_ident()); // Remove our Loader service
            }

            this->wait();
        }

        return 0;
    }

    int
    TAFServerInitializer::wait_result(time_t timeout) // timeout in millisecs
    {
        if (this->activate() ? false : this->initActive_ = true) {
            for (ACE_Time_Value tm(DAF_OS::gettimeofday(timeout)); this->initLock_.acquire(tm) == 0;) {
                int result = this->initResult_; this->initLock_.release(); return result;
            }
        }
        return -1;
    }

    int
    TAFServerInitializer::svc(void)
    {
        DAF_OS::thread_0_SIGSET_T(); // Ignore all signals to avoid ERROR:

        try {
            ACE_Service_Config_Guard svc_guard(ACE_Service_Config::global()); ACE_UNUSED_ARG(svc_guard);
            for (TAFServer * tafServer(ACE_Dynamic_Service<TAFServer>::instance(TAFServer::svc_ident())); tafServer;) {
                this->initResult_ = 0; this->initLock_.release(); DAF_OS::thr_yield(); // Let the wait_result() waiter go
                {
                    const DAF::ShutdownHandler shutdown_handler; return tafServer->run(true);  ACE_UNUSED_ARG(shutdown_handler);
                }
            }
        }
        catch (const CORBA::Exception &ex) {
            ex._tao_print_exception(TAFServer::svc_ident());
        } DAF_CATCH_ALL{
        }

        int result = -1; this->initLock_.release(); return result;
    }

    static TAFServerInitializer_ref tafServerInitRepo_(0);

} // ananomous


extern "C" {

    TAF_Export int _init_TAFServer(int argc, char *argv[])
    {
        static time_t tafserver_loader_timeout_(TAFServer::TAFSERVER_LOAD_TIMEOUT); // Timeout in Seconds

        static struct TAFServerLoaderTimeout {
            TAFServerLoaderTimeout(const char *_tafserver_load_timeout = DAF_OS::getenv(TAF_SERVERLOADTIMEOUT)) {
                if (_tafserver_load_timeout) {
                    const std::string tm(DAF::trim_string(_tafserver_load_timeout)); if (tm.length() && ::isdigit(int(tm[0]))) {
                        tafserver_loader_timeout_ = time_t(ace_range(30, 300, DAF_OS::atoi(tm.c_str())));
                    }
                }
            }
        } tafLoaderTimeout_; ACE_UNUSED_ARG(tafLoaderTimeout_);  // Stop any unused warning

        ACE_ARGV initArgs(true); initArgs.add(TAFServer::svc_ident()); // Put Us In As The Program Name - Loader

        ACE_Arg_Shifter arg_shifter(argc, argv);
        // Ignore Any Leading ProgramName if it exists
        for (int args = 0; arg_shifter.is_anything_left();) {
            if (args++ || arg_shifter.is_option_next()) {
                if (arg_shifter.is_option_next()) { // Look for only options
                    if (0 == arg_shifter.cur_arg_strncasecmp("-" TAF_SERVERLOADTIMEOUT)) {
                        arg_shifter.consume_arg(); if (arg_shifter.is_parameter_next()) { args++;
                            TAFServerLoaderTimeout(arg_shifter.get_current()); arg_shifter.consume_arg();
                        }
                        continue;
                    }
                }
                initArgs.add(arg_shifter.get_current());
            }
            arg_shifter.ignore_arg();
        }

        int result = -1;

        try {

            for (TAFServerInitializer_ref repo(tafServerInitRepo_._retn()); repo;) {
                repo->module_closed(); break;
            }

            DAF::ShutdownHandler::send_shutdown(false); // Reset Shutdown State

            TAFServerInitializer_ref ref(new TAFServerInitializer(DAF::parse_argv(initArgs)));

            if ((result = ref->wait_result(tafserver_loader_timeout_ * ACE_ONE_SECOND_IN_MSECS)) == 0) {
                tafServerInitRepo_ = ref._retn(); return 0;
            }

        } DAF_CATCH_ALL {
        }

        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P | %t) ERROR: Unable to Initialize %s [return=%d]\n")
            , TAFServer::svc_ident(), result), result);
    }

    TAF_Export int _wait_TAFServer(unsigned secs)
    {
        if (secs) {
            const ACE_Time_Value tm(DAF_OS::gettimeofday(DAF_MSECS_ONE_SECOND * secs));
            if (DAF::ShutdownHandler::wait_shutdown(&tm)) {
                DAF::ShutdownHandler::send_shutdown();
            }
        }

        DAF::ShutdownHandler::wait_shutdown(); return _fini_TAFServer();
    }

    TAF_Export int _fini_TAFServer(void)
    {
        for (TAFServerInitializer_ref repo(tafServerInitRepo_._retn()); repo;) {
            return repo->module_closed();
        }
        return 0;
    }
}
