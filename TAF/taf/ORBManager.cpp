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
#define TAF_ORBMANAGER_CPP

#include "ORBManager.h"

#include "POAPolicyList.h"
#include "PropertyLoader.h"

#include "TAFDebug.h"

#include <daf/PropertyManager.h>
#include <daf/ShutdownHandler.h>
#include <daf/ARGV.h>

//#include <tao/PI/PI.h>

#include <tao/AnyTypeCode/Objref_TypeCode_Static.h>

#if defined(TAF_HAS_STRATEGIES)
#include <tao/Strategies/advanced_resource.h>
#endif

#include <ace/Arg_Shifter.h>
#include <ace/Reactor.h>

#if defined(TAF_HAS_SECURITY)
# include <TAFSecurity/TAFSecurityLoader.h>
#endif

namespace // Use Anonymous namespace
{
    const char * tafORBName(void)
    {
        static const struct ORBNameString : std::string {
            ORBNameString(const std::string &def_name)
                : std::string(DAF::get_property(TAF_ORBNAME, def_name, true))
            {
                DAF::set_property(TAF_ORBNAME, *this);
            }
        } _tafORBName("TAFOrb:%P");

        return _tafORBName.c_str();
    }

    /*******************************************************************************/

    TAF::NamingContext rootContext_, baseContext_;

    /*******************************************************************************/

} // End Anonymous namespace

    /*******************************************************************************/

namespace TAF {

    ORB::ORBInitializer ORB::orbInitializer_; // Bootstrap Initializer instance

    ORB::ORB(void)
    {
        ACE_UNUSED_ARG(orbInitializer_);
    }

    ORB::~ORB(void)
    {
        this->module_closed(); this->wait();
        if (!CORBA::is_nil(this->orb_.in())) {
            this->orb_->destroy(); this->orb_ = 0;
        }
    }

    void
    ORB::pre_init(PortableInterceptor::ORBInitInfo_ptr info)
    {
        TAO_ORBInitInfo_var tao_info = TAO_ORBInitInfo::_narrow(info);

        if (CORBA::is_nil(tao_info.in())) {
            throw CORBA::INTERNAL();
        }
    }

    void
    ORB::post_init(PortableInterceptor::ORBInitInfo_ptr info)
    {
        TAO_ORBInitInfo_var tao_info = TAO_ORBInitInfo::_narrow(info);

        if (!CORBA::is_nil(tao_info.in())) {
            for (TAO_ORB_Core *orbCore = tao_info->orb_core(); orbCore;) {

                if (CORBA::is_nil((this->orb_ = CORBA::ORB::_duplicate(orbCore->orb())).in())) {
                    ACE_ERROR_BREAK((LM_ERROR,
                        ACE_TEXT("TAF (%P | %t) ERROR: Unable to initialize ORB Singleton for %s instance.\n"), tafORBName()));
                }

                {
                    CORBA::Object_var poaObj(TAFResolveInitialReferences(TAO_OBJID_ROOTPOA)); // Locate the Root POA
                    if (CORBA::is_nil(poaObj.in()) || CORBA::is_nil((this->rootPOA_ = PortableServer::POA::_narrow(poaObj.in())).in())) {
                        ACE_ERROR_BREAK((LM_ERROR,
                            ACE_TEXT("TAF (%P | %t) ERROR: Unable to initialize TheTAFRootPOA for %s instance.\n"), tafORBName()));
                    }

                    this->defaultPOA_ = this->rootPOA_;  // Set Default Initially to root POA.
                }

#if defined(TAF_HAS_UNIQUE_DEFAULT_POA) && (TAF_HAS_UNIQUE_DEFAULT_POA == 1)
                {
                    struct ORBDefaultPOAPolicy : TAF::POAPolicyList {
                        ORBDefaultPOAPolicy(const PortableServer::POA_var &poa) : TAF::POAPolicyList(poa) {
                            *this += PortableServer::TRANSIENT;                 // Lifespan Policy
                            *this += PortableServer::UNIQUE_ID;
                            *this += PortableServer::SYSTEM_ID;                 // ID Assignment Policy
                            *this += PortableServer::ORB_CTRL_MODEL;
                            *this += PortableServer::NO_IMPLICIT_ACTIVATION;    // No Implicit Activation
                            *this += PortableServer::RETAIN;                    // Servant Retention Policy
                            *this += PortableServer::USE_ACTIVE_OBJECT_MAP_ONLY;
                        }
                    } defaultPOAPolicy(this->rootPOA_);

                    for (PortableServer::POA_var defaultPOA(defaultPOAPolicy.createPOA(TAF_OBJID_DEFAULTPOA)); defaultPOA;) {
                        this->defaultPOA_ = defaultPOA._retn(); break;  // Overide the rootPOA
                    }
                }
#endif
                TAFRegisterInitialReference(TAF_OBJID_DEFAULTPOA, this->defaultPOA_.in());

                {
                    CORBA::Object_var tblObj(TAFResolveInitialReferences(TAO_OBJID_IORTABLE)); // Locate the IORTable
                    if (CORBA::is_nil(tblObj.in()) || CORBA::is_nil((this->IORTable_ = IORTable::Table::_narrow(tblObj.in())).in())) {
                        ACE_ERROR_BREAK((LM_ERROR,
                            ACE_TEXT("TAF (%P | %t) ERROR: Unable to initialize TheTAFIORTable for %s instance.\n"), tafORBName()));
                    }
                }

                /* Lets start the infrastructure here as the extension framework may wish to do ORB calls and we dont want them to block*/
                if (this->execute(TAF_MIN_THREADS, true) || (DAF_OS::thr_yield(), false)) { // We failed to start the ORB Seed Threads
                    ACE_DEBUG((LM_DEBUG,
                        ACE_TEXT("TAF (%P | %t) WARNING: Unable to initiate TheTAFOrb seed threads - Ignored.\n")));
                }

                /* Start the POA Manager to allow demultiplexing incomming requests */
                PortableServer::POAManager_var(this->rootPOA_->the_POAManager())->activate(); // Activate the POA Manager

                return; // All Good; ORB singleton now fully statefull
            }
        }

        throw CORBA::INTERNAL();
    }

    int
    ORB::run(size_t orb_threads)
    {
        // We started TAF_MIN_THREADS ORB Threads in the init() to allow POA to start up
        if (orb_threads > TAF_MIN_THREADS) { // Ensure positive
            if (this->execute(orb_threads - TAF_MIN_THREADS, true)) {  // Activate the ORB threads (minus the seed).
                ACE_ERROR_RETURN((LM_ERROR,
                    ACE_TEXT("ERROR: Failed to initialize %d threads for %s instance.\n")
                    , orb_threads, tafORBName()), -1);
            }
            DAF_OS::thr_yield(); // Yield a little for threads to start
        }

        return 0;
    }

    int
    ORB::svc(void)
    {
        for (int i = 0; this->isAvailable(); i++) try {
            if (TheTAFOrb() ? TheTAFOrbCore()->has_shutdown() : true) {
                break;
            }
            TheTAFOrb()->run(); break;
        } DAF_CATCH_ALL {
            if (TAF::debug()) {
                ACE_DEBUG((LM_DEBUG,
                    ACE_TEXT("TAF (%P | %t) ERROR: ORB::svc - unexpected exit #%d - Ignored.\n")
                    , i + 1));
            }
        }

        return 0;
    }

    const TAF::NamingContext &
    ORB::rootContext(ACE_Time_Value *timeout) const
    {
        static ACE_SYNCH_MUTEX rootLock_;

        do {

            ACE_GUARD_REACTION(ACE_SYNCH_MUTEX, rootLock, rootLock_, break);

            if (CORBA::is_nil(rootContext_.in())) {
                baseContext_ = CosNaming::NamingContext::_nil();  // Set nil baseContext_
                try {
                    CORBA::Object_var root_Obj(TAFResolveInitialReferences(TAO_OBJID_NAMESERVICE, timeout));
                    CosNaming::NamingContext_var root_cxt(CosNaming::NamingContext::_narrow(root_Obj.in()));
                    if (CORBA::is_nil(root_cxt.in())) {
                        break;
                    }
                    return rootContext_ = root_cxt._retn();
                } DAF_CATCH_ALL {
                    break;
                }
            } else try {
                if (rootContext_->_is_a(CosNaming::_tc_NamingContext->id())) {
                    return rootContext_;
                }
            } DAF_CATCH_ALL {}

            baseContext_ = rootContext_ = CosNaming::NamingContext::_nil();

        } while (false);

        if (TAF::debug()) {
            ACE_ERROR((LM_ERROR,
                ACE_TEXT("ERROR: Unable to initialize naming root for %s instance.\n"), tafORBName()));
        }

        throw CosNaming::NamingContext::NotFound();
    }

    const TAF::NamingContext &
    ORB::baseContext(ACE_Time_Value *timeout) const
    {
        static ACE_SYNCH_MUTEX baseLock_;

        do {

            ACE_GUARD_REACTION(ACE_SYNCH_MUTEX, baseLock, baseLock_, break);

            if (CORBA::is_nil(baseContext_.in())) {
                TAF::NamingContext root_cxt(TheTAFRootContext(timeout));  // Start Out At Root Context - May Throw
                try {
                    return baseContext_ = root_cxt.bind_context(DAF::get_property(TAF_BASECONTEXT));
                } DAF_CATCH_ALL {
                    return baseContext_ = root_cxt._retn();
                }
            } else try { /* We Have A Base Context so lets test it */
                if (baseContext_->_is_a(CosNaming::_tc_NamingContext->id())) {
                    return baseContext_;
                }
            } DAF_CATCH_ALL {}

            baseContext_ = rootContext_ = CosNaming::NamingContext::_nil();

        } while (false);

        if (TAF::debug()) {
            ACE_ERROR((LM_ERROR, /* Probably wont Error Here unless bind fails */
                ACE_TEXT("ERROR: Unable to bind base context to naming root for %s instance.\n"), tafORBName()));
        }

        throw CosNaming::NamingContext::NotFound();
    }

    int
    ORB::init(int argc, ACE_TCHAR *argv[])
    {
        // Extra ORB Parameters added to passed arguments */
        static const ACE_TCHAR *orb_defaults[] = { /* MUST Only Be VALID ORB Parameters */
            "-ORBDottedDecimalAddresses", "1",   /* Make all OS types consistently use Addresses (not DNS) */
            "-ORBObjRefStyle", "IOR",     /* Needs full IOR profiles to correctly determine multiple nic's */
            0  // Signifies End of arguments - Needed by ACE_ARGV::add(ACE_TCHAR *argv[])
        };

        std::string ext_params;  // Holder for temporary Extension arguments

#if defined(TAF_HAS_SECURITY)
        if (TAF::isSecurityEnabled()) try {
            if (TAFSecurityLoader_ref(new TAFSecurityLoader(argc, argv, true))->init(ext_params)) {
                throw "Security-Load-Failed";
            }
        } DAF_CATCH_ALL {
            DAF::set_property(TAF_SECURITYDISABLE, "1");
            ACE_DEBUG((LM_WARNING, ACE_TEXT("TAFSecurity (%P | %t) WARNING:")
                ACE_TEXT(" Unable to initialize %s instance with SSLIOP - Security enforcement disabled!\n")
                , tafORBName()));
        }
#endif

        DAF_ARGV orb_args(true); // Argument split for ORB Only Arguments + program name

        {
            /****************** Organise ORB Arguments ********************/
            // 1) argv[0] - Program Name
            // 2) Default ORB Arguments
            // 3) Extension Arguments - (May overide Defaults)
            // 4) User Arguments (May override Defaults and Extension arguments)

            DAF_ARGV svc_args(false); ACE_Arg_Shifter arg_shifter(argc, argv);   // Split Arguments into ORB and OTHER (service)

            // Put In All Leading Parameters (Program Name)
            for (; arg_shifter.is_parameter_next(); arg_shifter.ignore_arg()) {
                orb_args.add(arg_shifter.get_current(), true); svc_args.add(arg_shifter.get_current(), true);
            }

            orb_args.add(orb_defaults, true); // Default Our Extra ORB Bits First

            if (ext_params.length()) {  // Do The Extension Arguments

                for (ACE_ARGV ext_argv(ext_params.c_str(), false); ext_argv.argc();) {

                    int ext_argc_ = ext_argv.argc(); ACE_TCHAR **ext_argv_ = ext_argv.argv();

                    for (ACE_Arg_Shifter ext_shifter(ext_argc_, ext_argv_); ext_shifter.is_anything_left();) {
                        if (ext_shifter.is_option_next()) {
                            if (0 > ext_shifter.cur_arg_strncasecmp(CORBA_ORB_ARGUMENT_SPECIFIER)) {  // If not a '-ORB' option
                                svc_args.add(ext_shifter.get_current(), true);
                                for (ext_shifter.ignore_arg(); ext_shifter.is_parameter_next(); ext_shifter.ignore_arg()) {
                                    svc_args.add(ext_shifter.get_current(), true);
                                }
                                continue;
                            }
                        }
                        // Add ORB Extension Arguments
                        orb_args.add(ext_shifter.get_current(), true); ext_shifter.consume_arg();
                    }

                    break;
                }
            }

            while (arg_shifter.is_anything_left()) {                  // Add Other Arguments
                if (arg_shifter.is_option_next()) {
                    if (0 > arg_shifter.cur_arg_strncasecmp(CORBA_ORB_ARGUMENT_SPECIFIER)) {  // If not a '-ORB' option
                        svc_args.add(arg_shifter.get_current(), true);
                        for (arg_shifter.ignore_arg(); arg_shifter.is_parameter_next(); arg_shifter.ignore_arg()) {
                            svc_args.add(arg_shifter.get_current(), true);
                        }
                        continue;
                    }
                }
                orb_args.add(arg_shifter.get_current(), true); arg_shifter.consume_arg();
            }

            // Set The Resultant Service Arguments property
            DAF::set_property(TAF_SERVICE_ARGS, DAF::parse_argv(svc_args, true));
        }

        // Set The ORB_init args property
        const std::string orb_init_args(DAF::parse_argv(orb_args, true)); DAF::set_property(TAF_ORBINITARGS, orb_init_args);

        do try {

            TAO::debug_orb(DAF::get_numeric_property<int>(TAO_ORBDEBUGGING, TAO::debug_orb(), true));

            int orb_argc_ = orb_args.argc(); ACE_TCHAR **orb_argv_ = orb_args.argv();

            /* This causes ORB::post_init() to be called from the ORBInitializer to become statefull */
            if (CORBA::is_nil((CORBA::ORB_var(CORBA::ORB_init(orb_argc_, orb_argv_, tafORBName())).in()))) {
                break;
            }

            return 0;

        } catch (const CORBA::Exception &ex) {
            ex._tao_print_exception("TAF::ORB::init()");
        } DAF_CATCH_ALL {
        } while (false);

        ACE_ERROR_RETURN((LM_ERROR,
            ACE_TEXT("TAF (%P | %t) ERROR: Unable to initialize %s instance with Parameters:\n'%s'\n")
            , tafORBName(), orb_init_args.c_str()), -1);
    }

    int
    ORB::close(u_long flags)
    {
        if (flags) do {   /* Clean-up ORB Resources */

            if (CORBA::is_nil(TheTAFOrb().in()) || CORBA::is_nil(TheTAFOrbCore())) {
                break;
            } else if (TheTAFOrbCore()->has_shutdown()) {
                break;
            } else try {
                TheTAFOrb()->shutdown(true);
            } DAF_CATCH_ALL {
                /* Ignore Exceptions - We are shutting down */
            }

            baseContext_        = CosNaming::NamingContext::_nil();
            rootContext_        = CosNaming::NamingContext::_nil();

            this->IORTable_     = IORTable::Table::_nil();      // Reset Local IORTable cache
            this->defaultPOA_   = PortableServer::POA::_nil();  // Reset Local DefaultPOA cache
            this->rootPOA_      = PortableServer::POA::_nil();  // Reset Local RootPOA cache

        } while (false);

        return DAF::TaskExecutor::close(flags);
    }

    CORBA::Object_var
    ORB::resolve_initial_references(const std::string &id, ACE_Time_Value *timeout)
    {
        struct ORBResolveTimeout : ACE_Time_Value {
            ORBResolveTimeout(ACE_Time_Value *timeout) : ACE_Time_Value(TAO_DEFAULT_SERVICE_RESOLUTION_TIMEOUT, 0)
            {
                if (DAF::is_nil(timeout)) {
                    this->set(DAF::get_numeric_property<time_t>(TAF_RESOLVETIMEOUT, this->sec(), false), 0);
                }
                else this->set(timeout->sec(), timeout->usec());
            }
        } orb_resolve_timeout(timeout);

        return TheTAFOrb()->resolve_initial_references(id.c_str(), &orb_resolve_timeout);
    }

    /*******************************************************************************/

    ORBManager::ORBManager(void) : orbThreads_(DEFAULT_ORBTHREADS)
    {
        this->instance_i() = this;
    }

    ORBManager::ORBManager(int argc, ACE_TCHAR *argv[]) : orbThreads_(DEFAULT_ORBTHREADS)
    {
        this->instance_i() = this; if (argc && ORBManager::init(argc, argv)) {
            throw CORBA::BAD_OPERATION();
        }
    }

    ORBManager::~ORBManager(void)
    {
        this->module_closed(); this->wait(); this->instance_i() = 0;
    }

    size_t
    ORBManager::set_orb_threads(size_t threads)
    {
        return this->orbThreads_ = ace_range(TAF_MIN_THREADS, TAF_MAX_ORBTHREADS, threads);
    }

    CORBA::ORB_ptr
    ORBManager::operator -> () const
    {
        return TheTAFOrb().operator -> ();
    }

    ORBManager::operator CORBA::ORB_ptr const & () const
    {
        return TheTAFOrb().operator CORBA::ORB_ptr const & ();
    }

    int
    ORBManager::svc(void)
    {
        for (int i = 0; this->isAvailable(); i++) try {
            ACE_Reactor::run_event_loop(); break;
        } DAF_CATCH_ALL {
            if (TAF::debug()) {
                ACE_DEBUG((LM_DEBUG,
                    ACE_TEXT("TAF (%P | %t) ERROR: ORBManager::svc - unexpected exit #%d - Ignored\n")
                    , i + 1));
            }
        }

        return 0;
    }

    int
    ORBManager::run(size_t orb_threads, bool wait_for_completion)
    {
        if (this->isAvailable() && TheTAFOrbInstance()->run(this->set_orb_threads(orb_threads)) == 0) {
            return (wait_for_completion ? DAF::ShutdownHandler::wait_shutdown() : 0);
        }
        ACE_ERROR_RETURN((LM_INFO,
            ACE_TEXT("TAF (%P | %t) ERROR: ORBManager:")
            ACE_TEXT(" Unable to run the ORB without successfully initializing.\n")), -1);
    }

    int
    ORBManager::init(int argc, ACE_TCHAR *argv[])
    {
        DAF::set_property(TAF_TIMESTAMP, DAF::format_args(TAF_TIMESTAMP_FORMAT, true, false));
        DAF::set_property(TAF_DATESTAMP, DAF::format_args(TAF_DATESTAMP_FORMAT, true, false));

        DAF::set_property(TAF_SERVER_ARGS,  DAF::parse_args(argc, argv, false));

        // NOTE: This 'tafARGProperties' needs to be kept in scope through init_orb()
        TAF::PropertyLoader tafARGProperties(argc, argv, true);

        if (this->isAvailable() && tafARGProperties.load_properties(argc, argv) == 0) try {

            ACE::debug(DAF::get_numeric_property<int>(ACE_DEBUGGING, ACE::debug(), true));
            DAF::debug(DAF::get_numeric_property<int>(DAF_DEBUGGING, DAF::debug(), true));
            TAO::debug(DAF::get_numeric_property<int>(TAO_DEBUGGING, TAO::debug(), true));
            TAF::debug(DAF::get_numeric_property<int>(TAF_DEBUGGING, TAF::debug(), true));

            this->set_orb_threads(DAF::get_numeric_property<size_t>(TAF_ORBTHREADS,  DEFAULT_ORBTHREADS));

            size_t ace_threads(DAF::get_numeric_property<size_t>(ACE_BASETHREADS, DEFAULT_ACETHREADS));

            if (this->execute(ace_range(TAF_MIN_THREADS, TAF_MAX_ACETHREADS, ace_threads), true) == 0) {
                if (TheTAFOrbInstance()->init(argc, argv) == 0) { // Load the ORB
                    return 0;
                }
            }

        } catch (const CORBA::Exception &ex) {
            ex._tao_print_exception("ORBManager::init() failure");
        } DAF_CATCH_ALL {
        }

        ACE_ERROR_RETURN((LM_ERROR,
            ACE_TEXT("ERROR: Unable to configure the %s instance\n - Exiting.\n")
            , tafORBName()), -1);
    }

    int
    ORBManager::fini(void)
    {
        DAF::ShutdownHandler::send_shutdown(); return 0;
    }

    int
    ORBManager::close(u_long flags)
    {
        if (flags) {
            this->instance()->module_closed(); ACE_Reactor::end_event_loop();
        }
        return DAF::TaskExecutor::close(flags);
    }

    /**********************************************************************************************/

} // namespace TAF

namespace TAF  // Helper Functions to Singleton
{
    bool    isSecurityEnabled(void)
    {
#if defined(TAF_HAS_SECURITY)
        return DAF::get_numeric_property<bool>(TAF_SECURITYDISABLE, false, true) ? false : true;
#else
        return false;
#endif
    }
}
