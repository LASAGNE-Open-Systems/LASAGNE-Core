// LASAGNE Style - define a "NAMESPACE_FILENAME_EXTENSION"
// element to indicate what is being compiled
#define LTM_CORBACTIVESERVICE_CPP

// Include our CORBActiveService declaration
// NOTE - needs to be minimalistic and stand-alone
#include "CORBActiveService.h"

// Get access to the Primary TAF ORB Instance (TheTAFBaseContext)
#include <taf/ORBManager.h>

// Include argument parsing machinery
#include <ace/Get_Opt.h>
// Include reactor machinery including reactive timers
#include <ace/Reactor.h>

#include <sstream>

ACE_FACTORY_DEFINE(CORBActiveService, LTM_CORBActiveService); // Define the dynamic deployment factory
ACE_STATIC_SVC_DEFINE(LTM_CORBActiveService              // Define the static deployment descriptor
    , LTM_CORBActiveService::svc_ident()                 // Define the container ident key
    , ACE_SVC_OBJ_T                                     // Define it to be base on ACE_Service_Object
    , &ACE_SVC_NAME(LTM_CORBActiveService)               // Define the dynamic factory address
    , (ACE_Service_Type::DELETE_THIS | ACE_Service_Type::DELETE_OBJ) // Cleanup options on service destruction
    , false                  // Service not initially active on load, but rather after init() success
);

namespace LTM  // Open the LTM Namespace
{
    // Default Constructor Implementation - Used by deployment factories
    CORBActiveService::CORBActiveService(void) : DAF::TaskExecutor() // Often decorate with call to base
        , debug_        (0)
        , period_       (DEFAULT_TIMER_PERIOD)
        , threads_      (DEFAULT_THREAD_COUNT)
        , suspended_    (false)
        , svc_lock_     (1)   // Initially unlocked
        , svc_index_    (0)   // start svc at writer
        , use_naming_   (false) // Don't Use Naming by default
        , dcpsDomain_   (TAFDDS::DEFAULT_DOMAIN)
        , topicQueue_   (100) // Set queue capacity for topic instances before positive back-pressure is applied
    {
        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::CORBActiveService[%@]::CORBActiveService(void)\n"), this));
    }

    // Destructor Implementation - Cleanup resources - Used by deployment factories
    CORBActiveService::~CORBActiveService(void)
    {
        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::CORBActiveService[%@]::~CORBActiveService(void)\n"), this));

        this->module_closed(); // Force the thread pool to close - destroying any non-cooperative threads
    }

    // LASAGNE style - common method of parsing the arguments
    int
    CORBActiveService::parse_args(int argc, ACE_TCHAR *argv[])
    {
        ACE_Get_Opt get_opts(argc, argv, ACE_TEXT("f:t:p:d:z::n"), 0); // NOTE: starting from argv[0]
        get_opts.long_option("filename", 'f', ACE_Get_Opt::ARG_REQUIRED);
        get_opts.long_option("threads", 't', ACE_Get_Opt::ARG_REQUIRED);
        get_opts.long_option("period", 'p', ACE_Get_Opt::ARG_REQUIRED);
        get_opts.long_option("domain", 'd', ACE_Get_Opt::ARG_REQUIRED);
        get_opts.long_option("debug", 'z', ACE_Get_Opt::ARG_OPTIONAL);
        get_opts.long_option("use_naming", 'n', ACE_Get_Opt::NO_ARG);

        for (int arg_number = 0;; arg_number++) {  // do forever (until end of arguments)
            switch (get_opts()) {
            case -1: return 0; // Indicates sucessful parsing of the command line

            case 'f':   // filename - required
                ACE_DEBUG((LM_INFO, ACE_TEXT("\t\t\tfilename=\"%s\"\n"), get_opts.opt_arg()));
                break;

            case 't':   // threads - required
                for (const char *threads_val = get_opts.opt_arg(); threads_val;) {
                    if (::isdigit(int(*threads_val))) { // Ensure it is within reasonable range[4-1000]
                        this->threads_ = size_t(ace_range(4, 1000, DAF_OS::atoi(threads_val)));
                    }
                    break;
                }
                ACE_DEBUG((LM_INFO, ACE_TEXT("\t\t\tthreads=%d\n"), int(this->threads_)));
                break;

            case 'p':   // period - required
                for (const char *period_val = get_opts.opt_arg(); period_val;) {
                    if (::isdigit(int(*period_val))) { // Ensure it is within reasonable range[1-10]
                        this->period_.set(ace_range(1, 10, DAF_OS::atoi(period_val)), 0);
                    }
                    break;
                }
                ACE_DEBUG((LM_INFO, ACE_TEXT("\t\t\tperiod=%d (seconds)\n"), int(this->period_.sec())));
                break;

            case 'd':   // DCPS domain
            {
                const ACE_TCHAR *domain_opt = get_opts.opt_arg();

                if (domain_opt && ::isdigit(int(*domain_opt))) {
                    int dcps_domain(DAF_OS::atoi(domain_opt));

                    if (is_valid_dcps_domain(dcps_domain)) {
                        this->dcpsDomain_ = DDS::DomainId_t(dcps_domain);
                        ACE_DEBUG((LM_INFO, ACE_TEXT("\t\t\tdomain=%d (dcps)\n"), int(this->dcpsDomain_)));
                        break;
                    }
                }
            }

            ACE_ERROR_RETURN((LM_ERROR,
                ACE_TEXT("DCPS Domain (-d) must be between 0 and 230 for compatability with RTPS specification.\n")), -1);

            case 'z':   // debug - optional
                this->debug_ = 1; // Set debug initially to enabled (lowest level)

                for (const char *debug_lvl = get_opts.opt_arg(); debug_lvl;) {
                    if (::isdigit(int(*debug_lvl))) { // Ensure it is within range[1-10]
                        this->debug_ = ace_range(1, 10, DAF_OS::atoi(debug_lvl));
                    }
                    break;
                }

                ACE_DEBUG((LM_INFO, ACE_TEXT("\t\t\tdebug=%d\n"), this->debug_));
                break;

            case 'n':   // use_naming - no argument
                this->use_naming_ = true;
                ACE_DEBUG((LM_INFO, ACE_TEXT("\t\t\tuse_naming=true\n")));
                break;

            case '?': default:  // Unknown argument
                if (this->debug_) {
                    ACE_DEBUG((LM_WARNING, ACE_TEXT("(%P | %t) %s encountered unknown argument #%d\n")
                        ACE_TEXT("\tusage:\n")
                        ACE_TEXT("\t-t Threads (svc)\n")
                        ACE_TEXT("\t-p Period (seconds)\n")
                        ACE_TEXT("\t-d DCPS Domain [0-230]\n")
                        ACE_TEXT("\t-f Filename\n")
                        ACE_TEXT("\t-n Use-Naming\n")
                        ACE_TEXT("\t-z Debug ON[level]\n")
                        , LTM_CORBActiveService::svc_ident(), arg_number));
                }
                break;
            }
        }

        return 0;
    }

    // Initializes service object when dynamic/static linking occurs.
    int
    CORBActiveService::init(int argc, ACE_TCHAR *argv[])
    {
        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::CORBActiveService[%@]::init(int argc, ACE_TCHAR *argv[])\n")
            ACE_TEXT("\t\tArgs=\"%s\"\n"), this, DAF::parse_args(argc, argv).c_str()));

        int parse_error = this->parse_args(argc, argv);

        if (parse_error == 0) do { // Do We have any parsing errors?

            if (this->use_naming_) try {  // Add a binder - before we init_bind()
                this->addIORBinding(new TAF::NamingServiceBinder(TheTAFBaseContext()));
            } catch (const CORBA::Exception &ex) {
                ex._tao_print_exception("CORBActiveService: Failed to locate NamingService (option -n)");
            }

            // Initialize these DDS-Entities in order of dependance

            if (this->dcpsParticipant_.init(this->dcpsDomain_) != DDS::RETCODE_OK) {
                ACE_ERROR_BREAK((LM_ERROR,
                    ACE_TEXT("ERROR: %s - Unable to create participant for DDS domain '%d'; Exiting.\n")
                    , LTM_CORBActiveService::svc_ident(), int(this->dcpsDomain_)));
            }
            else if (this->topic_.init(this->dcpsParticipant_, ltm::LTMTopicDetailsName) != DDS::RETCODE_OK) {
                ACE_ERROR_BREAK((LM_ERROR,
                    ACE_TEXT("ERROR: %s - Unable to initialize DDS Topic \"%s\".\n")
                    , LTM_CORBActiveService::svc_ident(), ltm::LTMTopicDetailsName));
            }
            else if (this->dcpsSubscriber_.init(this->dcpsParticipant_) != DDS::RETCODE_OK) {
                ACE_ERROR_BREAK((LM_ERROR,
                    ACE_TEXT("ERROR: %s - Unable to create subscriber for DDS topic \"%s\"; Exiting.\n")
                    , LTM_CORBActiveService::svc_ident(), ltm::LTMTopicDetailsName));
            }
            else if (LTMTopicDetailsReader::init(this->dcpsSubscriber_, this->topic_) != DDS::RETCODE_OK) {
                ACE_ERROR_BREAK((LM_ERROR,
                    ACE_TEXT("ERROR: %s - Unable to initialize DDS Reader of topic \"%s\".\n")
                    , LTM_CORBActiveService::svc_ident(), ltm::LTMTopicDetailsName));
            }
            else if (this->init_bind(this->svc_ident())) { // Bind our CORBA interface with defaults.
                ACE_ERROR_BREAK((LM_ERROR,
                    ACE_TEXT("ERROR: %s - Unable to initialize CORBA Interface bindings.\n")
                    , LTM_CORBActiveService::svc_ident()));
            }

            // NOTE: an ACE_Service_Object inherits from ACE_Event_Handler so we are already potentially reactive
            long timer_id = ACE_Reactor::instance()->schedule_timer(
                this,           // We are the event Handler
                0,              // Asynchronous completion token to be carried with the timer
                this->period_,  // Initial delay before first timer
                this->period_); // Periodic interval

            if (timer_id != -1) { // We Created the timer ok

                if (this->execute(this->threads_) == 0) { // We Created the threads ok
                    return 0;
                }

                this->module_closed(); // Close out the thread engine
            }

            ACE_Reactor::instance()->cancel_timer(this);  // clean up any timers

        } while (false); // Structured goto

        // NOTE: We could just call this->fini() here in this case as all resources are dynamic

        return -1; // Returning non-zero from init() will immediately unload the service
    }

    // Request to suspend the service object - Implementation behaviour is service context defined.
    int
    CORBActiveService::suspend(void)
    {
        if (this->debug()) {
            ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::CORBActiveService[%@]::suspend(void).\n"), this));
        }

        this->suspended_ = true;
        return 0;
    }

    // Request to resume the service object - Implementation behaviour is service context defined.
    int
    CORBActiveService::resume(void)
    {
        if (this->debug()) {
            ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::CORBActiveService[%@]::resume(void).\n"), this));
        }

        this->suspended_ = false;
        return 0;
    }

    // About to unload the service - NOTE: should release any dynamic resources (i.e. what was reserved successfully in init)
    int
    CORBActiveService::fini(void)
    {
        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::CORBActiveService[%@]::fini(void)\n"), this));

        ACE_Reactor::instance()->cancel_timer(this); // Cancel all timers registered for this handler

        this->topicQueue_.interrupt(); // Interrupt the topic queue -> generates a runtime_error for threads

        this->fini_bind();      // Close our interface Bindings

        this->module_closed();  // Tell thread pool to close

        return 0;
    }

    // Supply a caller lifecycle owned string to describe this service - human readable
    int
    CORBActiveService::info(ACE_TCHAR **info_string, size_t length) const
    {
        static const char *info_desc =
        {
            "A CORBA mix-in Active Service training example."
        };

        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::CORBActiveService[%@]::info(ACE_TCHAR **info_string, size_t length=%d)\n")
            ACE_TEXT("\t returns \"%s\".\n"), this, length, info_desc));

        return (info_string ? (*info_string = ACE::strnnew(info_desc, length), 0) : -1);
    }

    // Called by reactor when a timer expires.
    int
    CORBActiveService::handle_timeout(const ACE_Time_Value &expected_time, const void *act)
    {
        ACE_UNUSED_ARG(act);

        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t)\t%s[%@]::handle_timeout(%#T )\n")
            , LTM_CORBActiveService::svc_ident()
            , this
            , &expected_time));

        return 0; // Returning -1 would cancel this timer
    }

    /* DDS Connector Interface */

    DDS::ReturnCode_t
    CORBActiveService::on_data_available(const LTMTopicDetailsTopicType &td)
    {
        if (this->suspended_) {
            return DDS::RETCODE_NOT_ENABLED; // Dont queue topic if service suspended
        }

        if (this->topicQueue_.put(LTMTopicDetailsCORBAdapter(td))) {
            return DDS::RETCODE_ERROR;
        }

        return DDS::RETCODE_OK;
    }

    /* DDS Processing Interface (ActiveObject) */

    int
    CORBActiveService::svc(void)
    {
        if (this->isAvailable()) { // while thread pool is still available

            if (this->svc_lock_.acquire() == 0) { // Let threads go sequentially
                int index = this->svc_index_++;
                switch (index) {
                case 0:
                case 2:
                    this->svc_writer(index);
                    break;

                default:
                    return this->svc_reader(index);
                }
            }
        }

        this->topicQueue_.interrupt(); // Interrupt the topic queue -> generates a runtime_error for reader threads
        return 0;
    }

    int
    CORBActiveService::svc_reader(int index)
    {
        if (this->debug()) {
            ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t)\t%s[%@]::svc_reader(%d) - Starting\n")
                , LTM_CORBActiveService::svc_ident()
                , this
                , index));
        }

        this->svc_lock_.release(); // Let Other svc routines go.

        try {

            while (this->isAvailable()) { // while thread pool is still available

                LTMTopicDetailsCORBAType td = this->topicQueue_.take();

                const ACE_Time_Value td_time(time_t(td.topicTime_.sec_), suseconds_t(td.topicTime_.usec_));

                ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t)\t%s::svc_reader[%02d] \"%s%#T\"\n")
                    , LTM_CORBActiveService::svc_ident()
                    , index
                    , td.topicData_.in()
                    , &td_time));
            }
        }

        catch (const std::runtime_error &) {
        }

        if (this->debug()) {
            ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t)\t%s[%@]::svc_reader(%d) - Ending\n")
                , LTM_CORBActiveService::svc_ident()
                , this
                , index));
        }

        return 0;
    }

    int
    CORBActiveService::svc_writer(int index)
    {
        if (this->debug()) {
            ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t)\t%s[%@]::svc_writer(%d) - Starting\n")
                , LTM_CORBActiveService::svc_ident()
                , this
                , index));
        }

        this->svc_lock_.release(); // Let Other svc routines go.

        CORBActivePublisher     dcpsPublisher;
        LTMTopicDetailsWriter   td_writer;

        if (this->isAvailable()) {

            // Initialize the publisher
            if (dcpsPublisher.init(this->dcpsParticipant_) != DDS::RETCODE_OK) {
                ACE_ERROR_RETURN((LM_ERROR,
                    ACE_TEXT("ERROR: %s - Unable to create publisher for DDS topic \"%s\"; Writer exiting.\n")
                    , LTM_CORBActiveService::svc_ident(), ltm::LTMTopicDetailsName), -1);
            }
            // Initialize the writer
            else if (td_writer.init(dcpsPublisher, this->topic_) != DDS::RETCODE_OK) {
                ACE_ERROR_RETURN((LM_ERROR,
                    ACE_TEXT("ERROR: %s - Unable to initialize DDS Writer of topic \"%s\"; Writer exiting.\n")
                    , LTM_CORBActiveService::svc_ident(), ltm::LTMTopicDetailsName), -1);
            }

            // Now create instances of LTMTopicDetails and publish them to the domain

            DAF_OS::srand(u_int(DAF_OS::gettimeofday().usec())); // Re-jig the random number generator seed

            for (ACE_Time_Value delay(1); this->isAvailable(); DAF_OS::sleep(delay)) {

                delay.set(DAF_OS::rand(0.1, 1.0)); // Set up a random delay

                const ACE_Time_Value td_time(DAF_OS::gettimeofday());

#if 0
                const LTMTopicDetailsCORBAType td = { // Constant initialiser
                    CORBA::ULongLong(td_time.sec()),
                    CORBA::ULong(td_time.usec()),
                    CORBA::ULong(DAF_OS::thread_ID()),
                    "DDS Topic Data"
                };
#else
                LTMTopicDetailsCORBAType td;
                {
                    td.topicTime_.sec_ = CORBA::ULongLong(td_time.sec());
                    td.topicTime_.usec_ = CORBA::ULong(td_time.usec());
                    td.topicID_ = CORBA::ULong(DAF_OS::thread_ID());
                    td.topicData_ = CORBA::string_dup("DDS Topic Data");
                }
#endif

                try {

                    DDS::ReturnCode_t dds_error = td_writer.publish(LTMTopicDetailsTopicAdapter(td));

                    if (dds_error != DDS::RETCODE_OK) {

                        ACE_DEBUG((LM_ERROR,
                            ACE_TEXT("ERROR: %s - Failure in publishing of topic %s with DDS::ReturnCode_t=%d.\n")
                            , LTM_CORBActiveService::svc_ident()
                            , ltm::LTMTopicDetailsName
                            , int(dds_error)));

                        break;
                    }

                } DAF_CATCH_ALL {
                    ACE_DEBUG((LM_ERROR,
                        ACE_TEXT("ERROR: %s - Unable to Publish instance of topic \"%s\".\n")
                        , LTM_CORBActiveService::svc_ident()
                        , ltm::LTMTopicDetailsName));
                    break;
                }
            }
        }

        if (this->debug()) {
            ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t)\t%s[%@]::svc_writer(%d) - Ending\n")
                , LTM_CORBActiveService::svc_ident()
                , this
                , index));
        }

        return 0;
    }

    /* CORBA Interface */

    CORBA::Long
    CORBActiveService::pushTopic(const LTMTopicDetailsCORBAType & td)
    {
        try {
            return CORBA::Long(this->topicQueue_.put(td));
        } catch (const CORBA::Exception &) {
            throw;
        } DAF_CATCH_ALL {
            throw CORBA::UNKNOWN();
        }
    }

} // Close the LTM Namespace
