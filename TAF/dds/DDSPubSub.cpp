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
#define TAFDDS_DDSPUBSUB_CPP

#include "DDSPubSub.h"

#include <taf/TAFDebug.h>

#include <daf/PropertyManager.h>
#include <daf/ServiceGestalt.h>

#include <ace/ARGV.h>
#include <ace/Service_Object.h>
#include <ace/Service_Config.h>
#include <ace/Framework_Component.h>

#if defined(TAF_USES_DDSCORBA)
# define DDS_IMPLEMENTATION_CORBA ACE_TEXT("with CORBA support")
#else
# define DDS_IMPLEMENTATION_CORBA ACE_TEXT("without CORBA support")
#endif

TAF_BEGIN_DDS_NAMESPACE_DECL

namespace // Annomous Namespace
{
    typedef class DomainFactory : public ACE_Service_Object
    {
        DDS::DomainParticipantFactory_ref   dpf_;

    public:

        DomainFactory(void) : dpf_(0) {}
        virtual ~DomainFactory(void)  {}

        static const char * svc_ident(void)
        {
#if   defined(TAF_USES_OPENDDS)
        return ACE_TEXT("ODDS_DomainFactory");
#elif defined(TAF_USES_NDDS)
        return ACE_TEXT("NDDS_DomainFactory");
#elif defined(TAF_USES_COREDX)
        return ACE_TEXT("CDDS_DomainFactory");
#elif defined(TAF_USES_OPENSPLICE)
        return ACE_TEXT("OSPL_DomainFactory");
#else
        return ACE_TEXT("TAF_DomainFactory");
#endif
        }

    protected:

        virtual int init(int argc, char *argv[]);
        virtual int fini(void);

    } TAFDomainFactory;

    int
    DomainFactory::init(int argc, char *argv[])
    {
        ACE_UNUSED_ARG(argc); ACE_UNUSED_ARG(argv); // Maybe unused dependant on DDS_TYPE

        ACE_Service_Config_Guard svc_guard(ACE_Service_Config::global()); ACE_UNUSED_ARG(svc_guard);

#if defined(TAF_USES_OPENDDS)
        OpenDDS::DCPS::MonitorFactory::service_initialize();
#endif

        if ((this->dpf_ = TheDomainParticipantFactoryWithArgs(argc, argv)) != 0) {
            if (TAF::debug()) {
                ACE_DEBUG((LM_DEBUG, ACE_TEXT("%s version %d.%d.%d loaded %s.\n"),
                    DDS_IMPLEMENTATION_NAME,
                    DDS_MAJOR_VERSION,DDS_MINOR_VERSION,DDS_MICRO_VERSION,
                    DDS_IMPLEMENTATION_CORBA));
            }
            return 0;
        }

        return -1;
    }

    int
    DomainFactory::fini(void)
    {
        ACE_Service_Config_Guard svc_guard(ACE_Service_Config::global()); ACE_UNUSED_ARG(svc_guard);

#if   defined(TAF_USES_OPENDDS)
        (TheServiceParticipant)->shutdown();
#elif defined(TAF_USES_NDDS)
        DDS::DomainParticipantFactory::finalize_instance();
#elif defined(TAF_USES_COREDX)
        DDS::DomainParticipantFactory::destroy();
#elif defined(TAF_USES_OPENSPLICE)

#endif
        return 0;
    }

    ACE_FACTORY_DEFINE(DDSPubSub, TAFDomainFactory);

    const ACE_Static_Svc_Descriptor DomainFactoryDescriptor = {
        TAFDomainFactory::svc_ident(),
        ACE_SVC_OBJ_T,
        &ACE_SVC_NAME(TAFDomainFactory),
        (ACE_Service_Type::DELETE_THIS | ACE_Service_Type::DELETE_OBJ),
        true // Service initially active
    };

    inline u_long hash_holder_key(const std::string &s)
    {
        return u_long(ACE::hash_pjw(s.data(), s.length()));
    }

} // End of Annomous namespace

namespace TAFDDS
{
    /******************* DDS_DomainParticipantFactory  ******************************************/

    DDS_DomainParticipant_factory::DDS_DomainParticipant_factory(void)
    {
        const std::string serviceARGS(ThePropertyRepository()->get_property(TAF_SERVICE_ARGS));

        try {
            if (ACE_Service_Config::process_directive(DomainFactoryDescriptor, true) == 0) {
                if (ACE_Service_Config::initialize(DomainFactoryDescriptor.name_, serviceARGS.c_str()) == 0) {
                    return;
                }
            }
        } DAF_CATCH_ALL { /* Fall through to Error */ }

        ACE_DEBUG((LM_ERROR, ACE_TEXT("ERROR: TAFDDSParticipantFactory<ctor>-Loader: - ")
            ACE_TEXT("Unable to initialize %s instance.\n"),
            DDS_IMPLEMENTATION_NAME));
    }

    DDS_DomainParticipant_factory::~DDS_DomainParticipant_factory(void)
    {
    }

    DDS_DomainParticipant_handle
    DDS_DomainParticipant_factory::create_participant(const DDS::DomainId_t &domain_id)
    {
        if (!is_valid_dcps_domain(domain_id)) {
            DAF_THROW_EXCEPTION(DAF::IllegalArgumentException);
        }

        ACE_GUARD_REACTION(ACE_SYNCH_MUTEX, mon, *this, DAF_THROW_EXCEPTION(DAF::ResourceExhaustionException));

        try {
            return DDS_DomainParticipant_holder::_narrow(this->locate_holder(domain_id).in());
        } catch (const DAF::NotFoundException&) {}

        TAFDDS::DomainParticipantQos qos;

#if defined(TAF_USES_COREDX)
        if ((TheDomainParticipantFactory)->get_default_participant_qos(&qos) != DDS::RETCODE_OK) {
#else
        if ((TheDomainParticipantFactory)->get_default_participant_qos(qos)  != DDS::RETCODE_OK) {
#endif
            DAF_THROW_EXCEPTION(DAF::InitializationException);
        }

        DDS::DomainParticipant_ref  participant((TheDomainParticipantFactory)->create_participant(domain_id,
            qos, 0, DDS::STATUS_MASK_NONE));

        if (participant) try {
            DDS_DomainParticipant_factory::value_type val(domain_id, new DDS_DomainParticipant_holder(*this, participant));
            return DDS_DomainParticipant_holder::_narrow(this->create_holder(val).in());
        } DAF_CATCH_ALL {
            // Failure to create participant and narrow...
        }

        std::string fmt("Unable to create %s Participant for Domain[%d]..\n");

#if defined(TAF_USES_OPENDDS)
            fmt.append("\t- Probable cause is InfoRepo/Discovery is not running.\n");
#elif defined(TAF_USES_NDDS)
            fmt.append("\t- Probable cause is the $(RTI_LICENSE_FILE) has expired or is not available.\n");
#elif defined(TAF_USES_COREDX)
            fmt.append("\t- Probable cause is the $(TWINOAKS_LICENSE_FILE) has expired or is not available.\n");
#elif defined(TAF_USES_OPENSPLICE)
            fmt.append("\t- Probable cause is the $(OSPL_URI) has not been set or your license has expired or is not available.\n");
#endif

        ACE_DEBUG((LM_ERROR, fmt.c_str(), DDS_IMPLEMENTATION_NAME, int(domain_id)));

        DAF_THROW_EXCEPTION(DAF::ObjectNotExistException);
    }

    DDS_DomainParticipant_holder::DDS_DomainParticipant_holder(DDS_DomainParticipant_factory &home_factory, const DDS::DomainParticipant_ref &ref)
        : DDS_DomainParticipant_factory::DDS_holder(home_factory, ref)
    {
    }

    DDS_DomainParticipant_holder::~DDS_DomainParticipant_holder(void)
    {
        (*this)->delete_contained_entities(); (TheDomainParticipantFactory)->delete_participant(this->handle_in());
    }

    DDS_Topic_handle
    DDS_DomainParticipant_holder::create_topic(DDS::String_ptr topic_name, const DDS::TypeSupport_ref &type_ref)
    {
        return this->topicFactory_.create_topic(*this, this->typeFactory_.register_type(*this, type_ref), topic_name);
    }

    /******************* DDS_TopicFactory  ******************************************/

    DDS_Topic_factory::~DDS_Topic_factory(void)
    {
        ACE_ASSERT(DDS_Topic_factory::size() == 0);
    }

    DDS_Topic_handle
    DDS_Topic_factory::create_topic(const DDS_DomainParticipant_handle &participant, const DDS_Type_handle &type, DDS::String_ptr topic_name)
    {
        if (topic_name ? DAF_OS::strlen(topic_name) == 0 : true) {
            DAF_THROW_EXCEPTION(DAF::IllegalArgumentException);
        }

        std::stringstream ss; ss << topic_name << '|' << (*type)->getTypename() << '|' << (*participant)->get_domain_id();

        const key_type topic_key(key_type(hash_holder_key(ss.str())));

        ACE_GUARD_REACTION(ACE_SYNCH_MUTEX, mon, *this, DAF_THROW_EXCEPTION(DAF::ResourceExhaustionException));

        try {
            return DDS_Topic_holder::_narrow(this->locate_holder(topic_key).in());
        } catch (const DAF::NotFoundException&) {}

        TAFDDS::TopicQos qos;

#if defined(TAF_USES_COREDX)
        if ((*participant)->get_default_topic_qos(&qos) != DDS::RETCODE_OK) {
#else
        if ((*participant)->get_default_topic_qos(qos) != DDS::RETCODE_OK) {
#endif
            DAF_THROW_EXCEPTION(DAF::InitializationException);
        }

        DDS::Topic_ref  topic((*participant)->create_topic(topic_name, (*type)->getTypename(), qos, 0, DDS::STATUS_MASK_NONE));

        if (topic == 0) {
            DAF_THROW_EXCEPTION(DAF::ObjectNotExistException);
        }

        DDS_Topic_factory::value_type val(topic_key, new DDS_Topic_holder(*this, participant, topic, type));

        return DDS_Topic_holder::_narrow(this->create_holder(val).in());
    }

    DDS_Topic_holder::DDS_Topic_holder( DDS_Topic_factory &home_factory,
                                        const DDS_DomainParticipant_handle &participant,
                                        const DDS::Topic_ref &ref,
                                        const DDS_Type_handle &type)
        : DDS_holder(home_factory, ref), participant_(participant), type_(type)
    {
    }

    DDS_Topic_holder::~DDS_Topic_holder(void)
    {
        (*this->participant_)->delete_topic(this->handle_in());
    }

    /******************* DDS_TypeFactory  ******************************************/

    DDS_Type_factory::~DDS_Type_factory(void)
    {
        ACE_ASSERT(DDS_Type_factory::size() == 0);
    }

    DDS_Type_handle
    DDS_Type_factory::register_type(const DDS_DomainParticipant_handle &participant, const DDS::TypeSupport_ref &ref)
    {
        std::stringstream ss; ss << ref->getTypename() << '|' << (*participant)->get_domain_id();

        const key_type type_key(key_type(hash_holder_key(ss.str())));

        ACE_GUARD_REACTION(ACE_SYNCH_MUTEX, mon, *this, DAF_THROW_EXCEPTION(DAF::ResourceExhaustionException));

        try {
            return DDS_Type_holder::_narrow(this->locate_holder(type_key).in());
        } catch (const DAF::NotFoundException&) {}

        DDS_Type_factory::value_type val(type_key, new DDS_Type_holder(*this, participant, ref));

        return DDS_Type_holder::_narrow(this->create_holder(val).in());
    }

    DDS_Type_holder::DDS_Type_holder(DDS_Type_factory &home_factory, const DDS_DomainParticipant_handle &participant, const DDS::TypeSupport_ref &ref)
        : DDS_holder(home_factory, ref), participant_(participant)
    {
        (*this)->registerTypename(this->participant_->handle_in());
    }

    DDS_Type_holder::~DDS_Type_holder(void)
    {
        (*this)->unregisterTypename(this->participant_->handle_in());
    }

    /******************* DDS_DomainParticipant  ******************************************/

    DDS::ReturnCode_t
    DDS_DomainParticipant::init(const DDS::DomainId_t &domain_id)
    {
        static DDS_DomainParticipant_factory participant_factory;

        if ((this->out() = participant_factory.create_participant(domain_id)._retn()) == 0) {
            ACE_ERROR_RETURN((LM_ERROR,
                ACE_TEXT("ERROR: DDS_DomainParticipant - Unable to create DomainParticipant for domain [%d].\n"), int(domain_id)),
                    DDS::RETCODE_ERROR);
        }
        return DDS::RETCODE_OK;
    }

    /******************* DDS_Subscriber_holder *******************************/

    DDS_Subscriber_holder::DDS_Subscriber_holder(const DDS_DomainParticipant_handle &participant, const DDS::Subscriber_ref &ref)
        : DAF::RefCountHandler_T<DDS::Subscriber_ref>(ref), participant_(participant)
    {
    }

    void
    DDS_Subscriber_holder::_finalize(_handle_inout_type p)
    {
        if (p) {
            (*this)->delete_contained_entities(); (*this->participant_)->delete_subscriber(p); p = 0;
        }
    }

    /******************* DDS_Publisher_holder ********************************/

    DDS_Publisher_holder::DDS_Publisher_holder(const DDS_DomainParticipant_handle &participant, const DDS::Publisher_ref &ref)
        : DAF::RefCountHandler_T<DDS::Publisher_ref>(ref), participant_(participant)
    {
    }

    void
    DDS_Publisher_holder::_finalize(_handle_inout_type p)
    {
        if (p) {
            (*this)->delete_contained_entities(); (*this->participant_)->delete_publisher(p); p = 0;
        }
    }
} // namespace TAFDDS

TAF_END_DDS_NAMESPACE_DECL
