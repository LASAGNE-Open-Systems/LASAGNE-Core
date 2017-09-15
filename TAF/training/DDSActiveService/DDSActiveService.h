#ifndef LTM_DDSACTIVESERVICE_H  /* LASAGNE Convention - "NAMESPACE_FILENAME_EXTENSION" */
#define LTM_DDSACTIVESERVICE_H  /* Include Guard - Stops being multiply included */

// Include base level TAF framework: NOTES:
// - Case sensative for LINUX compatability on all includes
// - Use "/" Seperator (NOT "\" as WINDOWS only! and is often interpreted as an esc)
// - <..> means to search include path for file (and not current directory)
#include <taf/TAF.h>

// Include a waiter preference semaphore to use as svc_lock
#include <daf/Semaphore.h>

// Include base level lifecycle controlled thread pool
#include <daf/TaskExecutor.h>

// Include a buffered synchronising queue if T's
#include <daf/SemaphoreControlledQueue_T.h>

// Include base level ACE_Service_Config for deployment factory mechanisms
#include <ace/Service_Config.h>

// Include our DDSActiveDataSupport declarations
#include "DDSActiveDataSupport.h"

namespace LTM   // Open the LTM namespace
{
    // Define the DDSActive DDS-Entities
    typedef class TAFDDS::DDS_DomainParticipant                                         DDSActiveParticipant;
    typedef class TAFDDS::DDS_Subscriber<TAFDDS::SubscriberListener>                    DDSActiveSubscriber;
    typedef class TAFDDS::DDS_Publisher<TAFDDS::PublisherListener>                      DDSActivePublisher;

    // Define the LTMTopicDetails DDS-Entities
    typedef class TAFDDS::TOPICReaderListener<LTMTopicDetailsDataSupport>               LTMTopicDetailsListener;
    typedef class TAFDDS::DDS_Topic<LTMTopicDetailsDataSupport>                         LTMTopicDetailsTopic;
    typedef class TAFDDS::DDS_Reader<LTMTopicDetailsTopic, LTMTopicDetailsListener>     LTMTopicDetailsReader;
    typedef class TAFDDS::DDS_Writer<LTMTopicDetailsTopic>                              LTMTopicDetailsWriter;

    // Define a buffered Queue of instances of LTMTopicDetailsCORBAType (defined in DDSServiceDataSupport.h)
    typedef class DAF::SemaphoreControlledQueue<LTM::LTMTopicDetailsCORBAType>          LTMTopicDetailsQueue;

    /*
        Declare the DDSActiveService, decorated with the exports,
        and privately virtual inherit from the base level TopicReader.  Use the
        DAF::TaskExecutor as the underlying active thread engine which itself
        is an ACE_Service_Object. This allows active services to be manipulated with
        a Gestalt (a patterned service container) through its semi-abstract
        service interface of init/fini/suspend/resume/info
    */

    /** \class DDSActiveService
    *\brief  A DDS Training Example that uses the Active Object pattern
    *
    * This Example uses multiple Active Object patterns to provide dds-reader/dds-writer capabilities
    * through the DAF::TaskExecutor as the thread engine
    * NOTE: Must "virtually" inherit from private reader as we use IOC (possible diamond)
    */

    class DDSActiveService_Export DDSActiveService : virtual LTMTopicDetailsReader
        , public DAF::TaskExecutor
    {

        int parse_args(int argc, ACE_TCHAR *argv[]); // Our private parse_args

    public:

        enum {
            DEFAULT_TIMER_PERIOD = time_t(1),  // Default to 1 Second;
            DEFAULT_THREAD_COUNT = size_t(3)   // Default to creating 3 service (svc) threads
        };

        // Default constructor - LASAGNE style puts in "void" arguments as decorator placeholder
        DDSActiveService(void); // Used by Configurator deployment factories

        // Destructor - virtual ensures proper destruction (i.e. polymorphic destruction)
        virtual ~DDSActiveService(void);

        int debug(void) const { return this->debug_; } // Allow access to our debug state

        // LASAGNE Style - define a "static" svc_ident() that can be used by factories as key to service
        static const char * svc_ident(void)
        {
            return "LTM_DDSActiveService"; // Visable inline implementation in declaration
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

    protected: // DDS Connector

        virtual int svc_reader(int index); // Read topic instances from topicQueue and put them out to console
        virtual int svc_writer(int index); // Generate topic instances and publish them.

        /// Receiving topic data from underlying LTMTopicDetailsReader
        virtual DDS::ReturnCode_t on_data_available(const LTMTopicDetailsTopicType &td);

        /* Shutup these messages from DDS Reader */
        virtual void on_subscription_matched(DDS::DataReader_ptr, const DDS::SubscriptionMatchedStatus&)    {}
        virtual void on_liveliness_changed(DDS::DataReader_ptr, const DDS::LivelinessChangedStatus&)        {}
        virtual void on_sample_lost(DDS::DataReader_ptr, const DDS::SampleLostStatus&)
        {
            if (this->debug()) std::cout << '-';
        }

        DDSActiveParticipant    dcpsParticipant_;
        DDSActiveSubscriber     dcpsSubscriber_;

    private:

        LTMTopicDetailsTopic    topic_;

    private:

        int debug_;     // Our local service level debug value [0-10]

        ACE_Time_Value  period_;    // Periodic timer value
        size_t          threads_;   // Service Threads
        volatile bool   suspended_; // Service is suspended
        DAF::Semaphore  svc_lock_;  // Lock to sequentially start reader/writer svc
        int             svc_index_; // Index of svc thread zero == writer non-zero == reader(s)

    private:

        DDS::DomainId_t         dcpsDomain_;    // DCPS Domain for publishing
        LTMTopicDetailsQueue    topicQueue_;    // Buffered Queue of LTMTopicDetailsCORBAType

    }; // closing of the DDSActiveService class declaration

} // close the LTM namespace

typedef class LTM::DDSActiveService   LTM_DDSActiveService; // Typedef out namespace for service factory macros

// Declare deployment factory
ACE_FACTORY_DECLARE(DDSActiveService, LTM_DDSActiveService);
// Declare deployment static load descriptor - Used for static loads if required
ACE_STATIC_SVC_DECLARE_EXPORT(DDSActiveService, LTM_DDSActiveService);

#endif // LTM_ACTIVESERVICE_H
