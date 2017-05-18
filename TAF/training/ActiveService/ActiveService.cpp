// LASAGNE Style - define a "NAMESPACE_FILENAME_EXTENSION"
// element to indicate what is being compiled
#define LTM_ACTIVESERVICE_CPP

// Include our ActiveService declaration
// NOTE - needs to be minimalistic and stand-alone
#include "ActiveService.h"

// Include argument parsing machinery
#include <ace/Get_Opt.h>
// Include reactor machinery including reactive timers
#include <ace/Reactor.h>

ACE_FACTORY_DEFINE(ActiveService, LTM_ActiveService);   // Define the dynamic deployment factory
ACE_STATIC_SVC_DEFINE(LTM_ActiveService                 // Define the static deployment descriptor
    , LTM_ActiveService::svc_ident()                    // Define the container ident key
    , ACE_SVC_OBJ_T                                     // Define it to be base on ACE_Service_Object
    , &ACE_SVC_NAME(LTM_ActiveService)                  // Define the dynamic factory address
    , (ACE_Service_Type::DELETE_THIS | ACE_Service_Type::DELETE_OBJ) // Cleanup options on service destruction
    , false                  // Service not initially active on load, but rather after init() success
);

namespace LTM  // Open the LTM Namespace
{
    // Default Constructor Implementation - Used by deployment factories
    ActiveService::ActiveService(void) : DAF::TaskExecutor() // Often decorate with call to base
        , debug_    (0)
        , period_   (DEFAULT_TIMER_PERIOD)
        , threads_  (DEFAULT_THREAD_COUNT)
    {
        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::ActiveService[%@]::ActiveService(void)\n"), this));
    }

    // Destructor Implementation - Cleanup resources - Used by deployment factories
    ActiveService::~ActiveService(void)
    {
        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::ActiveService[%@]::~ActiveService(void)\n"), this));

        this->module_closed(); // Force the thread pool to close - destroying any non-cooperative threads
    }

    // LASAGNE style - common method of parsing the arguments
    int
    ActiveService::parse_args(int argc, ACE_TCHAR *argv[])
    {
        ACE_Get_Opt get_opts(argc, argv, ACE_TEXT("f:t:p:z::"), 0); // NOTE: starting from argv[0]
        get_opts.long_option("filename", 'f', ACE_Get_Opt::ARG_REQUIRED);
        get_opts.long_option("threads", 't', ACE_Get_Opt::ARG_REQUIRED);
        get_opts.long_option("period", 'p', ACE_Get_Opt::ARG_REQUIRED);
        get_opts.long_option("debug", 'z', ACE_Get_Opt::ARG_OPTIONAL);

        for (int arg_number = 0;; arg_number++) {  // do forever (until end of arguments)
            switch (get_opts()) {
            case -1: return 0; // Indicates sucessful parsing of the command line

            case 'f':   // filename - required
                ACE_DEBUG((LM_INFO, ACE_TEXT("\t\t\tfilename=\"%s\"\n"), get_opts.opt_arg()));
                break;

            case 't':   // threads - required
                for (const char *threads_val = get_opts.opt_arg(); threads_val;) {
                    if (::isdigit(int(*threads_val))) { // Ensure it is within reasonable range[1-1000]
                        this->threads_ = size_t(ace_range(1, 1000, DAF_OS::atoi(threads_val)));
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

            case '?': default:  // Unknown argument
                if (this->debug_) {
                    ACE_DEBUG((LM_WARNING, ACE_TEXT("(%P | %t) %s encountered unknown argument #%d\n")
                        ACE_TEXT("\tusage:\n")
                        ACE_TEXT("\t-t Threads (svc)\n")
                        ACE_TEXT("\t-p Period (seconds)\n")
                        ACE_TEXT("\t-f Filename\n")
                        ACE_TEXT("\t-z Debug ON[level]\n")
                        , LTM_ActiveService::svc_ident(), arg_number));
                }
                break;
            }
        }

        return 0;
    }

    // Initializes service object when dynamic/static linking occurs.
    int
    ActiveService::init(int argc, ACE_TCHAR *argv[])
    {
        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::ActiveService[%@]::init(int argc, ACE_TCHAR *argv[])\n")
            ACE_TEXT("\t\tArgs=\"%s\"\n"), this, DAF::parse_args(argc, argv).c_str()));

        int parse_error = this->parse_args(argc, argv);

        if (parse_error == 0) { // Do We have any parsing errors?

            // NOTE: an ACE_Service_Object inherits from ACE_Event_Handler so we are already potentially reactive
            long timer_id = ACE_Reactor::instance()->schedule_timer(
                this,           // We are the event Handler
                0,              // Asynchronous completion token to be carried with the timer
                this->period_,  // Initial delay before first timer
                this->period_); // Periodic interval

            if (timer_id != -1) { // We Created the timer ok

                if (this->execute(this->threads_) == 0) { // We Created the threads ok
                    return 0; // now let us run.
                }

                this->module_closed(); // Close out the thread engine
            }

            ACE_Reactor::instance()->cancel_timer(this);  // clean up any timers
        }

        // NOTE: We could just call this->fini() here in this case as all resources are dynamic

        return -1; // Returning non-zero from init() will immediately unload the service
    }

    // Request to suspend the service object - Implementation behaviour is service context defined.
    int
    ActiveService::suspend(void)
    {
        if (this->debug_) {
            ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::ActiveService[%@]::suspend(void) - Ignored.\n"), this));
        }

        ACE_NOTSUP_RETURN(-1); // Not supported
    }

    // Request to resume the service object - Implementation behaviour is service context defined.
    int
    ActiveService::resume(void)
    {
        if (this->debug_) {
            ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::ActiveService[%@]::resume(void) - Ignored.\n"), this));
        }

        ACE_NOTSUP_RETURN(-1); // Not supported
    }

    // About to unload the service - NOTE: should release any dynamic resources (i.e. what was reserved successfully in init)
    int
    ActiveService::fini(void)
    {
        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::ActiveService[%@]::fini(void)\n"), this));

        ACE_Reactor::instance()->cancel_timer(this); // Cancel all timers registered for this handler

        this->module_closed();  // Tell thread pool to close

        return 0; // returning non-zero will cause service to immediately unload from service container
    }

    // Supply a caller lifecycle owned string to describe this service - human readable
    int
    ActiveService::info(ACE_TCHAR **info_string, size_t length) const
    {
        static const char *info_desc =
        {
            "An Active Service training example."
        };

        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::ActiveService[%@]::info(ACE_TCHAR **info_string, size_t length=%d)\n")
            ACE_TEXT("\t returns \"%s\".\n"), this, length, info_desc));

        return (info_string ? (*info_string = ACE::strnnew(info_desc, length), 0) : -1);
    }

    // Called by reactor when a timer expires.
    int
    ActiveService::handle_timeout(const ACE_Time_Value &expected_time, const void *act)
    {
        ACE_UNUSED_ARG(act);

        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t)\t%s[%@]::handle_timeout(%#T )\n")
            , LTM_ActiveService::svc_ident()
            , this
            , &expected_time));

        return 0; // Returning -1 would cancel this timer
    }

    int
    ActiveService::svc(void)
    {
        DAF_OS::srand(u_int(DAF_OS::gettimeofday().usec())); // Re-jig the random number generator seed

        for (int i = 0; this->isAvailable(); i++) { // while thread pool is still available

            ACE_Time_Value delay; delay.set(DAF_OS::rand(0.5, 2.5)); // Set up a random delay

            ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t)\t%s[%@]::svc(%04d) - delay=%u msec.\n")
                , LTM_ActiveService::svc_ident()
                , this, i
                , unsigned(delay.msec())));

            DAF_OS::sleep(delay); // sleep for a random amount of time
        }

        return 0;
    }

} // Close the LTM Namespace
