#ifndef LTM_ACTIVESERVICE_H  /* LASAGNE Convention - "NAMESPACE_FILENAME_EXTENSION" */
#define LTM_ACTIVESERVICE_H  /* Include Guard - Stops being multiply included */

// Include base level TAF framework: NOTES:
// - Case sensative for LINUX compatability on all includes
// - Use "/" Seperator (NOT "\" as WINDOWS only! and is often interpreted as an esc)
// - <..> means to search include path for file (and not current directory)
#include <taf/TAF.h>

// Include base level lifecycle controlled thread pool
#include <daf/TaskExecutor.h>

// Include base level ACE_Service_Config for deployment factory mechanisms
#include <ace/Service_Config.h>

// Include generated exports file - See prebuild directive in PassiveService.mpc
// ".." means to search current directory (ie, where .sln/makefile is) first.
#include "ActiveService_export.h"

namespace LTM   // Open the LTM namespace
{
    /*
        Declare the PassiveService, decorated with the exports,
        and publically inherit from the base level ACE_Service_Object.
        This ACE_Service_Object allows services to be manipulated with
        a Gestalt (a patterned service container) through its semi-abstract
        service interface of init/fini/suspend/resume/info
    */


    class ActiveService_Export ActiveService : public DAF::TaskExecutor
    {
        int parse_args(int argc, ACE_TCHAR *argv[]); // Our private parse_args

    public:

        enum {
            DEFAULT_TIMER_PERIOD = time_t(1),  // Default to 1 Second;
            DEFAULT_THREAD_COUNT = size_t(2)   // Default to creating 2 service (svc) threads
        };

        // Default constructor - LASAGNE style puts in "void" arguments as decorator placeholder
        ActiveService(void); // Used by Configurator deployment factories

        // Destructor - virtual ensures proper destruction (i.e. polymorphic destruction)
        virtual ~ActiveService(void);

        // LASAGNE Style - define a "static" svc_ident() that can be used by factories as key to service
        static const char * svc_ident(void)
        {
            return "LTM_ActiveService"; // Visable inline implementation in declaration
        }

    protected:  // LASAGNE style; switch to protected interface

        // The Service interface uses inversion of control (IOC) so not called directly on this interface

        // LASAGNE style - if polymorphic, decorate with "virtual" even if it is implied through inheritance.

        /// Initializes service object when dynamic/static linking occurs.
        virtual int init(int argc, ACE_TCHAR *argv[]);

        /// Suspends object.
        virtual int suspend(void);

        /// Resume object.
        virtual int resume(void);

        /// Terminates object when dynamic unlinking occurs.
        virtual int fini(void);

        /// Returns information on a service object.
        virtual int info(ACE_TCHAR **info_string, size_t length = 0) const;

    protected: // Reactive Service (Timer)

        /**
        * Called when timer expires.  @a current_time represents the current
        * time that the Event_Handler was selected for timeout
        * dispatching and @a act is the asynchronous completion token that
        * was passed in when <schedule_timer> was invoked.
        * NOTE: Thread comming from ACE_Reactor's thread pool
        */
        virtual int handle_timeout(const ACE_Time_Value &current_time, const void *act = 0);

    protected: // Active Service (svc)


        virtual int svc(void);

    private:

        int debug_;     // Our local service level debug value [0-10]

        ACE_Time_Value  period_;    // Periodic timer value
        size_t          threads_;   // Service Threads

    }; // closing of the ReactiveService class declaration

} // close the LTM namespace

typedef class LTM::ActiveService   LTM_ActiveService; // Typedef out namespace for service factory macros

// Declare deployment factory
ACE_FACTORY_DECLARE(ActiveService, LTM_ActiveService);
// Declare deployment static load descriptor - Used for static loads if required
ACE_STATIC_SVC_DECLARE_EXPORT(ActiveService, LTM_ActiveService);

#endif // LTM_ACTIVESERVICE_H
