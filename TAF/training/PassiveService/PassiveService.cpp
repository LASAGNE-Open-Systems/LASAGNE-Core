// LASAGNE Style - define a "NAMESPACE_FILENAME_EXTENSION"
// element to indicate what is being compiled
#define LTM_PASSIVESERVICE_CPP

// Include our PassiveService declaration
// NOTE - needs to be minimalistic and stand-alone
#include "PassiveService.h"

// Include argument parsing machinery
#include <ace/Get_Opt.h>

ACE_FACTORY_DEFINE(PassiveService, LTM_PassiveService); // Define the dynamic deployment factory
ACE_STATIC_SVC_DEFINE(LTM_PassiveService                // Define the static deployment descriptor
    , LTM_PassiveService::svc_ident()                   // Define the container ident key
    , ACE_SVC_OBJ_T                                     // Define it to be base on ACE_Service_Object
    , &ACE_SVC_NAME(LTM_PassiveService)                 // Define the dynamic factory address
    , (ACE_Service_Type::DELETE_THIS | ACE_Service_Type::DELETE_OBJ) // Cleanup options on service destruction
    , false                  // Service not initially active on load, but rather after init() success
);

namespace LTM  // Open the LTM Namespace
{
    // Default Constructor Implementation - Used by deployment factories
    PassiveService::PassiveService(void) : ACE_Service_Object() // Often decorate with call to base
        , debug_(0)
    {
        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::PassiveService[%@]::PassiveService(void)\n"), this));
    }

    // Destructor Implementation - Cleanup resources - Used by deployment factories
    PassiveService::~PassiveService(void)
    {
        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::PassiveService[%@]::~PassiveService(void)\n"), this));
    }

    // LASAGNE style - common method of parsing the arguments
    int
    PassiveService::parse_args(int argc, ACE_TCHAR *argv[])
    {
        ACE_Get_Opt get_opts(argc, argv, ACE_TEXT("f:z::h"), 0); // NOTE: starting from argv[0]
        get_opts.long_option("filename",'f', ACE_Get_Opt::ARG_REQUIRED);
        get_opts.long_option("debug",'z', ACE_Get_Opt::ARG_OPTIONAL);
        get_opts.long_option("help",'h', ACE_Get_Opt::NO_ARG);  // Help

        for (int arg_number = 0;;arg_number++) {  // do forever (until end of arguments)
            switch (get_opts()) {
            case -1: return 0; // Indicates sucessful parsing of the command line

            case 'f':   // filename - required
                ACE_DEBUG((LM_INFO, ACE_TEXT("\t\t\tfilename=\"%s\"\n"), get_opts.opt_arg()));
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

            case 'h': // help - no-arguments
                ACE_DEBUG((LM_INFO, ACE_TEXT("usage: %s\n")
                    ACE_TEXT("-f Filename\n")
                    ACE_TEXT("-z Debug ON[level]\n")
                    , LTM_PassiveService::svc_ident()));
                break;

            case '?': default: // unknown option
                ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P | %t) %s failed to parse arguments - unknown argument #%d\n")
                    , LTM_PassiveService::svc_ident(), arg_number), -1); 
            }
        }

        return 0;
    }

    // Initializes service object when dynamic/static linking occurs.
    int
    PassiveService::init(int argc, ACE_TCHAR *argv[])
    {
        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::PassiveService[%@]::init(int argc, ACE_TCHAR *argv[])\n")
            ACE_TEXT("\t\tArgs=\"%s\"\n"), this, DAF::parse_args(argc, argv).c_str()));

        int parse_error = this->parse_args(argc, argv); 

        if (parse_error != 0) {
            return -1; // Returning non-zero from init() will immediately unload the service
        }

        return 0;
    }

    // Request to suspend the service object - Implementation behaviour is service context defined.
    int
    PassiveService::suspend(void)
    {
        if (this->debug_) {
            ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::PassiveService[%@]::suspend(void) - Ignored.\n"), this));
        }

        DAF_OS::last_error(ENOTSUP); return -1; // Say not supported
    }

    // Request to resume the service object - Implementation behaviour is service context defined.
    int
    PassiveService::resume(void)
    {
        if (this->debug_) {
            ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::PassiveService[%@]::resume(void) - Ignored.\n"), this));
        }

        ACE_NOTSUP_RETURN(-1); // Not supported
    }

    // About to unload the service - NOTE: should release any dynamic resources (i.e. what was reserved successfully in init)
    int
    PassiveService::fini(void)
    {
        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::PassiveService[%@]::fini(void)\n"), this));

        return 0;
    }

    // Supply a caller lifecycle owned string to describe this service - human readable
    int
    PassiveService::info(ACE_TCHAR **info_string, size_t length) const
    {
        static const char *info_desc =
        {
            "A Passive Service training example."
        };

        ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t) LTM::PassiveService[%@]::info(ACE_TCHAR **info_string, size_t length=%d)\n")
            ACE_TEXT("\t returns \"%s\".\n"), this, length, info_desc));

        return (info_string ? (*info_string = ACE::strnnew(info_desc, length), 0) : -1);
    }

} // Close the LTM Namespace
