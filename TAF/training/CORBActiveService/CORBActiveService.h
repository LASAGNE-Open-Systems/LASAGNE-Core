#ifndef LTM_CORBACTIVESERVICE_H  /* LASAGNE Convention - "NAMESPACE_FILENAME_EXTENSION" */
#define LTM_CORBACTIVESERVICE_H  /* Include Guard - Stops being multiply included */

// Include base level TAF framework: NOTES:
// - Case sensative for LINUX compatability on all includes
// - Use "/" Seperator (NOT "\" as WINDOWS only! and is often interpreted as an esc)
// - <..> means to search include path for file (and not current directory)
#include <taf/TAF.h>

// Include the CORBA Interface Handler machinery
#include <taf/CORBAInterfaceHandler_T.h>

// Include a waiter preference semaphore to use as svc_lock
#include <daf/Semaphore.h>

// Include base level lifecycle controlled thread pool
#include <daf/TaskExecutor.h>

// Include a buffered synchronising queue if T's
#include <daf/SemaphoreControlledQueue_T.h>

// Include base level ACE_Service_Config for deployment factory mechanisms
#include <ace/Service_Config.h>

// Include our CORBActiveDataSupport DDS declarations
#include "CORBActiveDataSupport.h"

#include "CORBActiveServiceS.h"

namespace LTM   // Open the LTM namespace
{
    // Define the CORBAActive DDS-Entities
    typedef class TAFDDS::DDS_DomainParticipant                                         CORBActiveParticipant;
    typedef class TAFDDS::DDS_Subscriber<TAFDDS::SubscriberListener>                    CORBActiveSubscriber;
    typedef class TAFDDS::DDS_Publisher<TAFDDS::PublisherListener>                      CORBActivePublisher;

    // Define the LTMTopicDetails DDS-Entities
    typedef class TAFDDS::TOPICReaderListener<LTMTopicDetailsDataSupport>               LTMTopicDetailsListener;
    typedef class TAFDDS::DDS_Topic<LTMTopicDetailsDataSupport>                         LTMTopicDetailsTopic;
    typedef class TAFDDS::DDS_Reader<LTMTopicDetailsTopic, LTMTopicDetailsListener>     LTMTopicDetailsReader;
    typedef class TAFDDS::DDS_Writer<LTMTopicDetailsTopic>                              LTMTopicDetailsWriter;

    // Define a buffered Queue of instances of LTMTopicDetailsCORBAType (defined in CORBActiveDataSupport.h)
    typedef class DAF::SemaphoreControlledQueue<LTM::LTMTopicDetailsCORBAType>          LTMTopicDetailsQueue;

    // Define the Interface Handler for our CORBActiveService idl interface
    typedef class TAF::CORBAInterfaceHandler_T<POA_ltm::CORBActiveService>              CORBActiveServiceHandler;

    /*
        Declare the CORBActiveService, decorated with the exports,
        and publicly virtual inherit from the base level TAF::CORBAInterfaceHandler (CORBActiveServiceHandler)
        and private virtual TopicReader.  Use the DAF::TaskExecutor as the underlying active thread engine
        which itself is an ACE_Service_Object. This allows active services to be manipulated with
        a Gestalt (a patterned service container) through its semi-abstract
        service interface of init/fini/suspend/resume/info
    */

    /** \class CORBActiveService
    *\brief  A DDS/CORBA mix-in Training Example that uses the Active Object pattern and CORBA interface
    *
    * This Example uses multiple Active Object patterns to provide dds-reader/dds-writer capabilities
    * through the DAF::TaskExecutor as the thread engine
    * NOTE: Must "virtually" inherit from public handler and private reader as we use IOC (diamond)
    */

    class CORBActiveService_Export CORBActiveService : virtual public CORBActiveServiceHandler
        , virtual LTMTopicDetailsReader // Must "virtually" inherit from private reader (diamond)
        , public DAF::TaskExecutor
    {

        int parse_args(int argc, ACE_TCHAR *argv[]); // Our private parse_args

    public:

        enum {
            DEFAULT_TIMER_PERIOD = time_t(1),  // Default to 1 Second;
            DEFAULT_THREAD_COUNT = size_t(3)   // Default to creating 3 service (svc) threads
        };

        // Default constructor - LASAGNE style puts in "void" arguments as decorator placeholder
        CORBActiveService(void); // Used by Configurator deployment factories

        // Destructor - virtual ensures proper destruction (i.e. polymorphic destruction)
        virtual ~CORBActiveService(void);

        int debug(void) const { return this->debug_; } // Allow access to our debug state

        // LASAGNE Style - define a "static" svc_ident() that can be used by factories as key to service
        static const char * svc_ident(void)
        {
            return ltm::CORBActiveService_OID; // Visable inline implementation in declaration
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
        virtual void on_subscription_matched(DDS::DataReader_ptr, const DDS::SubscriptionMatchedStatus&)    throw () {}
        virtual void on_liveliness_changed(DDS::DataReader_ptr, const DDS::LivelinessChangedStatus&)        throw () {}
        virtual void on_sample_lost(DDS::DataReader_ptr, const DDS::SampleLostStatus&)                      throw ()
        {
            if (this->debug()) std::cout << '-';
        }

        CORBActiveParticipant   dcpsParticipant_;
        CORBActiveSubscriber    dcpsSubscriber_;

    protected:  // CORBA Interface

        virtual CORBA::Long pushTopic(const LTMTopicDetailsCORBAType & td);

    private:

        LTMTopicDetailsTopic    topic_;

    private:

        int debug_;     // Our local service level debug value [0-10]

        ACE_Time_Value  period_;        // Periodic timer value
        size_t          threads_;       // Service Threads
        volatile bool   suspended_;     // Service is suspended
        DAF::Semaphore  svc_lock_;      // Lock to sequentially start reader/writer svc
        int             svc_index_;     // Index of svc thread zero == writer non-zero == reader(s)
        bool            use_naming_;    // Name ourselves in Naming Service

    private:

        DDS::DomainId_t         dcpsDomain_;    // DCPS Domain for publishing
        LTMTopicDetailsQueue    topicQueue_;    // Buffered Queue of LTMTopicDetailsCORBAType

    }; // closing of the ReactiveService class declaration

} // close the LTM namespace

typedef class LTM::CORBActiveService   LTM_CORBActiveService; // Typedef out namespace for service factory macros

// Declare deployment factory
ACE_FACTORY_DECLARE(CORBActiveService, LTM_CORBActiveService);
// Declare deployment static load descriptor - Used for static loads if required
ACE_STATIC_SVC_DECLARE_EXPORT(CORBActiveService, LTM_CORBActiveService);

#endif // LTM_ACTIVESERVICE_H
