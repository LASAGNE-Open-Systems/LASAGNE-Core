// LASAGNE Style - define a "NAMESPACE_FILENAME_EXTENSION"
// element to indicate what is being compiled
#define LTM_REACTIVESERVICE_CPP

// Include our ReactiveService declaration
// NOTE - needs to be minimalistic and stand-alone
#include "ReactiveService.h"

// Include argument parsing machinery
#include <ace/Get_Opt.h>
// Include reactor machinery including reactive timers
#include <ace/Reactor.h>

ACE_FACTORY_DEFINE(ReactiveService, LTM_ReactiveService);   // Define the dynamic deployment factory
ACE_STATIC_SVC_DEFINE(LTM_ReactiveService                   // Define the static deployment descriptor
    , LTM_ReactiveService::svc_ident()                      // Define the container ident key
    , ACE_SVC_OBJ_T                                         // Define it to be base on ACE_Service_Object
    , &ACE_SVC_NAME(LTM_ReactiveService)                    // Define the dynamic factory address
    , (ACE_Service_Type::DELETE_THIS | ACE_Service_Type::DELETE_OBJ) // Cleanup options on service destruction
    , false                  // Service not initially active on load, but rather after init() success
);

namespace LTM  // Open the LTM Namespace
{
    // Default Constructor Implementation - Used by deployment factories
    ReactiveService::ReactiveService(void) : ACE_Service_Object() // Often decorate with call to base
        , debug_    (0)
        , period_   (DEFAULT_TIMER_PERIOD)
    {
        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::ReactiveService[%@]::ReactiveService(void)\n"), this));
    }

    // Destructor Implementation - Cleanup resources - Used by deployment factories
    ReactiveService::~ReactiveService(void)
    {
        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::ReactiveService[%@]::~ReactiveService(void)\n"), this));
    }

    // LASAGNE style - common method of parsing the arguments
    int
    ReactiveService::parse_args(int argc, ACE_TCHAR *argv[])
    {
        ACE_Get_Opt get_opts(argc, argv, ACE_TEXT("f:p:z::"), 0); // NOTE: starting from argv[0]
        get_opts.long_option("filename", 'f', ACE_Get_Opt::ARG_REQUIRED);
        get_opts.long_option("period", 'p', ACE_Get_Opt::ARG_REQUIRED);
        get_opts.long_option("debug", 'z', ACE_Get_Opt::ARG_OPTIONAL);

        for (int arg_number = 0;; arg_number++) {  // do forever (until end of arguments)
            switch (get_opts()) {
            case -1: return 0; // Indicates sucessful parsing of the command line

            case 'f':   // filename - required
                ACE_DEBUG((LM_INFO, ACE_TEXT("\t\t\tfilename=\"%s\"\n"), get_opts.opt_arg()));
                break;

            case 'p':   // period - required
                for (const char *period_val = get_opts.opt_arg(); period_val;) {
                    if (::isdigit(int(*period_val))) { // Ensure it is within reasonable range[1-10]
                        this->period_.set(ace_range(1, 10, DAF_OS::atoi(period_val)),0);
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
                        ACE_TEXT("\t-p Period (seconds)\n")
                        ACE_TEXT("\t-f Filename\n")
                        ACE_TEXT("\t-z Debug ON[level]\n")
                        , LTM_ReactiveService::svc_ident(), arg_number));
                }
                break;
            }
        }

        return 0;
    }

    // Initializes service object when dynamic/static linking occurs.
    int
    ReactiveService::init(int argc, ACE_TCHAR *argv[])
    {
        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::ReactiveService[%@]::init(int argc, ACE_TCHAR *argv[])\n")
            ACE_TEXT("\t\tArgs=\"%s\"\n"), this, DAF::parse_args(argc, argv).c_str()));

        int parse_error = this->parse_args(argc, argv);

        if (parse_error == 0) { // Do We have any parsing errors?

            // NOTE: an ACE_Service_Object inherits from ACE_Event_Handler so we are already potentially reactive
            long timer_id = ACE_Reactor::instance()->schedule_timer(
                this,           // We are the event Handler
                0,              // Asynchronous completion token to be carried with the timer
                this->period_,  // Initial delay before first timer
                this->period_); // Periodic interval

            if (timer_id != -1) {
                return 0;
            }

            ACE_Reactor::instance()->cancel_timer(this);  // clean up any timers
        }

        return -1; // Returning non-zero from init() will immediately unload the service
    }

    // Request to suspend the service object - Implementation behaviour is service context defined.
    int
    ReactiveService::suspend(void)
    {
        if (this->debug_) {
            ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::ReactiveService[%@]::suspend(void) - Ignored.\n"), this));
        }

        ACE_NOTSUP_RETURN(-1); // Not supported
    }

    // Request to resume the service object - Implementation behaviour is service context defined.
    int
    ReactiveService::resume(void)
    {
        if (this->debug_) {
            ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::ReactiveService[%@]::resume(void) - Ignored.\n"), this));
        }

        ACE_NOTSUP_RETURN(-1); // Not supported
    }

    // About to unload the service - NOTE: should release any dynamic resources (i.e. what was reserved successfully in init)
    int
    ReactiveService::fini(void)
    {
        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::ReactiveService[%@]::fini(void)\n"), this));

        ACE_Reactor::instance()->cancel_timer(this); // Cancel all timers registered for this handler

        return 0;
    }

    // Supply a caller lifecycle owned string to describe this service - human readable
    int
    ReactiveService::info(ACE_TCHAR **info_string, size_t length) const
    {
        static const char *info_desc =
        {
            "A Reactive Service training example."
        };

        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::ReactiveService[%@]::info(ACE_TCHAR **info_string, size_t length=%d)\n")
            ACE_TEXT("\t returns \"%s\".\n"), this, length, info_desc));

        return (info_string ? (*info_string = ACE::strnnew(info_desc, length), 0) : -1);
    }

    // Called by reactor when a timer expires.
    int
    ReactiveService::handle_timeout(const ACE_Time_Value &expected_time, const void *act)
    {
        ACE_UNUSED_ARG(act);

        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %05t) %s[%@]::handle_timeout(%#T )\n")
            , LTM_ReactiveService::svc_ident()
            , this
            , &expected_time));

        return 0; // Returning -1 would cancel this timer
    }

} // Close the LTM Namespace
