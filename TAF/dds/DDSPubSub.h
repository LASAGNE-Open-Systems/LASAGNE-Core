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
#ifndef TAFDDS_DDSPUBSUB_H
#define TAFDDS_DDSPUBSUB_H

#include "DDSDefs.h"
#include "DDSQos.h"
#include "DDSListener.h"

#include "DDSManagedType_T.h"

TAF_BEGIN_DDS_NAMESPACE_DECL

namespace TAFDDS
{
    /******************* Define the Meta Types *******************************/
    typedef class ::TAF::DDS_Domain_holder<DDS::DomainId_t, DDS::DomainParticipant_ref>     DDS_DomainParticipant_FactoryBase;
    typedef class ::TAF::DDS_Domain_holder<u_long, DDS::TypeSupport_ref>                    DDS_Type_FactoryBase;
    typedef class ::TAF::DDS_Domain_holder<u_long, DDS::Topic_ref>                          DDS_Topic_FactoryBase;

    /******************* DDS_ParticipantFactory ******************************/

    class DDSPubSub_Export DDS_DomainParticipant_factory : public DDS_DomainParticipant_FactoryBase
    {
        DDS::DomainParticipantFactory_ref   dpf_;

    public:
        /* Define Meta Types */
        typedef class DDS_DomainParticipant_holder      _holder_type;
        typedef class DAF::ObjectRef<_holder_type>      _handle_type;

        DDS_DomainParticipant_factory(void);
        ~DDS_DomainParticipant_factory(void);

        _handle_type    create_participant(const DDS::DomainId_t&);
    };

    typedef DDS_DomainParticipant_factory::_handle_type DDS_DomainParticipant_handle;

    /******************* DDS_TypeFactory *************************************/
    struct DDSPubSub_Export DDS_Type_factory : DDS_Type_FactoryBase
    {
        /* Define Meta Types */
        typedef class DDS_Type_holder                   _holder_type;
        typedef class DAF::ObjectRef<_holder_type>      _handle_type;

        ~DDS_Type_factory(void);

        _handle_type    register_type(const DDS_DomainParticipant_handle &participant, const DDS::TypeSupport_ref&);
    };

    typedef DDS_Type_factory::_handle_type  DDS_Type_handle;

    /******************* DDS_TopicFactory ************************************/

    struct DDSPubSub_Export DDS_Topic_factory : DDS_Topic_FactoryBase
    {
        /* Define Meta Types */
        typedef class DDS_Topic_holder                  _holder_type;
        typedef class DAF::ObjectRef<_holder_type>      _handle_type;

        ~DDS_Topic_factory(void);

        _handle_type    create_topic(const DDS_DomainParticipant_handle&, const DDS_Type_handle&, DDS::String_ptr topic_name);
    };

    typedef DDS_Topic_factory::_handle_type     DDS_Topic_handle;

    /******************* DDS_Participant_holder ******************************/

    class DDSPubSub_Export DDS_DomainParticipant_holder : public DDS_DomainParticipant_factory::DDS_holder
    {
        DDS_Topic_factory   topicFactory_;
        DDS_Type_factory    typeFactory_;

    public:

        DAF_DEFINE_REFCOUNTABLE(DDS_DomainParticipant_holder);

        DDS_DomainParticipant_holder(DDS_DomainParticipant_factory&, const DDS::DomainParticipant_ref&);
        ~DDS_DomainParticipant_holder(void);

        DDS_Topic_handle    create_topic(DDS::String_ptr topic_name, const DDS::TypeSupport_ref &type_ref);

        const DDS_Topic_factory&    getTopicFactory(void) const { return this->topicFactory_; }
        const DDS_Type_factory&     getTypeFactory(void) const  { return this->typeFactory_;  }
    };

    /******************* DDS_Type_holder ******************************/

    class DDSPubSub_Export DDS_Type_holder : public DDS_Type_FactoryBase::DDS_holder
    {
        DDS_DomainParticipant_handle    participant_;

    public:

        DAF_DEFINE_REFCOUNTABLE(DDS_Type_holder);

        DDS_Type_holder(DDS_Type_factory&, const DDS_DomainParticipant_handle&, const DDS::TypeSupport_ref&);
        ~DDS_Type_holder(void);
    };

    /******************* DDS_Topic_holder *****************************/

    class DDSPubSub_Export DDS_Topic_holder : public DDS_Topic_FactoryBase::DDS_holder
    {
        DDS_DomainParticipant_handle    participant_;
        DDS_Type_handle                 type_;

    public:

        DAF_DEFINE_REFCOUNTABLE(DDS_Topic_holder);

        DDS_Topic_holder(DDS_Topic_factory&, const DDS_DomainParticipant_handle&, const DDS::Topic_ref&, const DDS_Type_handle&);
        ~DDS_Topic_holder(void);
    };

    /******************* DDS_Subscriber_holder *******************************/

    class DDSPubSub_Export DDS_Subscriber_holder : public DAF::RefCountHandler_T<DDS::Subscriber_ref>
    {
        DDS_DomainParticipant_handle    participant_;

    public:

        DAF_DEFINE_REFCOUNTABLE(DDS_Subscriber_holder);

        DDS_Subscriber_holder(const DDS_DomainParticipant_handle&, const DDS::Subscriber_ref&);
        virtual ~DDS_Subscriber_holder(void) {
            this->_finalize(this->handle_inout());
        }

    protected:

        virtual void _finalize(_handle_inout_type);
    };

    typedef DDS_Subscriber_holder::_ref_type    DDS_Subscriber_handle;

    /******************* DDS_Publisher_holder ********************************/

    class DDSPubSub_Export DDS_Publisher_holder : public DAF::RefCountHandler_T<DDS::Publisher_ref>
    {
        DDS_DomainParticipant_handle    participant_;

    public:

        DAF_DEFINE_REFCOUNTABLE(DDS_Publisher_holder);

        DDS_Publisher_holder(const DDS_DomainParticipant_handle&, const DDS::Publisher_ref&);
        virtual ~DDS_Publisher_holder(void) {
            this->_finalize(this->handle_inout());
        }

    protected:

        virtual void _finalize(_handle_inout_type);

    };

    typedef DDS_Publisher_holder::_ref_type     DDS_Publisher_handle;

    /******************* DDS_DomainParticipant  ******************************************/

    class DDSPubSub_Export DDS_DomainParticipant : public DDS_DomainParticipant_handle
    {
    public:
        virtual ~DDS_DomainParticipant(void) {}
        virtual DDS::ReturnCode_t   init(const DDS::DomainId_t &domain_id = TAFDDS::DEFAULT_DOMAIN);
    };

    /******************* DDS_Topic  *************************************************/

    template < typename T_SUPPORT >
    class DDS_Topic : public DDS_Topic_handle
    {
        std::string topic_name_;

    public:

        virtual ~DDS_Topic(void) {}

        /* Define Meta Types */

        typedef T_SUPPORT                                           _support_type;
        typedef typename TAFDDS::DDS_Topic<_support_type>           _topic_type;

        typedef typename _support_type::_data_type                  _data_type;
        typedef typename _support_type::_data_seq_type              _data_seq_type;
        typedef typename _support_type::_data_holder_type           _data_holder_type;
        typedef typename _support_type::_data_reader_stub_type      _data_reader_stub_type;
        typedef typename _support_type::_data_reader_stub_type_ptr  _data_reader_stub_type_ptr;
        typedef typename _support_type::_data_reader_stub_type_ref  _data_reader_stub_type_ref;
        typedef typename _support_type::_data_writer_stub_type      _data_writer_stub_type;
        typedef typename _support_type::_data_writer_stub_type_ptr  _data_writer_stub_type_ptr;
        typedef typename _support_type::_data_writer_stub_type_ref  _data_writer_stub_type_ref;

        DDS::String_ptr getTopicName(void) const;

        virtual DDS::ReturnCode_t   init(const DDS_DomainParticipant_handle&, DDS::String_ptr topic_name, DDS::String_ptr topic_type = 0);
    };

    /******************* DDS_Subscriber *******************************************/

    template < typename T_LISTENER = TAFDDS::SubscriberListener >
    class DDS_Subscriber : public virtual T_LISTENER, public DDS_Subscriber_handle
    {
        TAFDDS::SubscriberQos qos_;

    public:

        virtual ~DDS_Subscriber(void) {}

        /* Define Meta Types */
        typedef T_LISTENER                                      _listener_type;
        typedef typename TAFDDS::DDS_Subscriber<_listener_type> _subscriber_type;

        virtual DDS::ReturnCode_t   init(const DDS_DomainParticipant_handle&);

        virtual DDS::ReturnCode_t   getQos(DDS::SubscriberQos &qos) const;
        virtual DDS::StatusMask     getStatusMask(void) const;
    };

    /******************* DDS_Publisher ***********************************************/

    template < typename T_LISTENER = TAFDDS::PublisherListener >
    class DDS_Publisher : public virtual T_LISTENER, public DDS_Publisher_handle
    {
        TAFDDS::PublisherQos  qos_;

    public:

        virtual ~DDS_Publisher(void) {}

        /* Define Meta Types */
        typedef T_LISTENER                                      _listener_type;
        typedef typename TAFDDS::DDS_Publisher<_listener_type>  _publisher_type;

        virtual DDS::ReturnCode_t   init(const DDS_DomainParticipant_handle&);

        virtual DDS::ReturnCode_t   getQos(DDS::PublisherQos &qos) const;
        virtual DDS::StatusMask     getStatusMask(void) const;
    };

    /******************* DDS_Writer ***********************************************/

    template < typename T_TOPIC, typename T_LISTENER = TAFDDS::DataWriterListener >
    class DDS_Writer : public virtual T_LISTENER
    {
        DDS_Topic_handle        topic_;
        DDS_Publisher_handle    publisher_;

        TAFDDS::DataWriterQos   qos_;

    public:

        /* Define Meta Types */
        typedef T_TOPIC                                             _topic_type;
        typedef T_LISTENER                                          _listener_type;

        typedef typename T_TOPIC::_support_type                     _support_type;
        typedef typename _topic_type::_data_type                    _data_type;
        typedef typename _topic_type::_data_seq_type                _data_seq_type;
        typedef typename _topic_type::_data_holder_type             _data_holder_type;
        typedef typename _topic_type::_data_writer_stub_type        _data_writer_stub_type;
        typedef typename _topic_type::_data_writer_stub_type_ptr    _data_writer_stub_type_ptr;
        typedef typename _topic_type::_data_writer_stub_type_ref    _data_writer_stub_type_ref;

        typedef typename TAFDDS::DDS_Writer<_topic_type, _listener_type>  _writer_type;

        DDS_Writer(void) : writer_(0) {}

        virtual ~DDS_Writer(void);

        const _data_writer_stub_type_ref & operator -> () const // Get To Underlying DDS Writer Methods
        {
            return this->writer_;
        }

        virtual DDS::ReturnCode_t   publish(const _data_type&);

        virtual DDS::ReturnCode_t   init(const DDS_Publisher_handle&, const DDS_Topic_handle&);

        virtual DDS::ReturnCode_t   getQos(DDS::DataWriterQos &qos) const;
        virtual DDS::StatusMask     getStatusMask(void) const;

        /* Implementation needs to be here because of dependant return type */
        friend _writer_type& operator << (_writer_type &lhs, const _data_type &rhs)
        {
            lhs.publish(rhs); return lhs;
        }

    protected:

        mutable ACE_SYNCH_MUTEX writer_lock_;

    private:

        _data_writer_stub_type_ref  writer_;
    };

    /******************* DDS_Reader **********************************************/

    template < typename T_TOPIC, typename T_LISTENER = TAFDDS::DataReaderListener >
    class DDS_Reader : public virtual T_LISTENER
    {
        DDS_Topic_handle        topic_;
        DDS_Subscriber_handle   subscriber_;

        TAFDDS::DataReaderQos   qos_;

    public:

        /* Define Meta Types */

        typedef T_TOPIC    _topic_type;
        typedef T_LISTENER _listener_type;

        typedef typename _topic_type::_support_type                 _support_type;
        typedef typename _topic_type::_data_type                    _data_type;
        typedef typename _topic_type::_data_seq_type                _data_seq_type;
        typedef typename _topic_type::_data_holder_type             _data_holder_type;
        typedef typename _topic_type::_data_reader_stub_type        _data_reader_stub_type;
        typedef typename _topic_type::_data_reader_stub_type_ptr    _data_reader_stub_type_ptr;
        typedef typename _topic_type::_data_reader_stub_type_ref    _data_reader_stub_type_ref;

        typedef typename TAFDDS::DDS_Reader<_topic_type, _listener_type>  _reader_type;

        DDS_Reader(void) : reader_(0) {}

        virtual ~DDS_Reader(void);

        const _data_reader_stub_type_ref & operator -> () const // Get To Underlying DDS Reader Methods
        {
            return this->reader_;
        }

        virtual DDS::ReturnCode_t   init(const DDS_Subscriber_handle&, const DDS_Topic_handle&);

        virtual DDS::ReturnCode_t   getQos(DDS::DataReaderQos &qos) const;
        virtual DDS::StatusMask     getStatusMask(void) const;

    protected:

        mutable ACE_SYNCH_MUTEX reader_lock_;

        virtual void on_data_available(DDS::DataReader_ptr);

    private:

        _data_reader_stub_type_ref  reader_;
    };

    /******************* TOPICReaderListener *****************************************/

    template < typename T_SUPPORT >
    class TOPICReaderListener : public virtual TAFDDS::DataReaderListener
    {
    public:

        virtual ~TOPICReaderListener(void) {}

        enum {
            TOPIC_SAMPLES_MAX = 32
        };

        /* Define Meta Types */
        typedef T_SUPPORT _support_type;

        typedef typename _support_type::_data_type                  _data_type;
        typedef typename _support_type::_data_seq_type              _data_seq_type;
        typedef typename _support_type::_data_holder_type           _data_holder_type;
        typedef typename _support_type::_data_reader_stub_type      _data_reader_stub_type;
        typedef typename _support_type::_data_reader_stub_type_ptr  _data_reader_stub_type_ptr;
        typedef typename _support_type::_data_reader_stub_type_ref  _data_reader_stub_type_ref;

    protected:

        virtual DDS::ReturnCode_t on_data_available(const _data_type&);

        virtual void on_data_available(DDS::DataReader_ptr);
    };

/***********************************************************************************/
} //  namespace TAFDDS

TAF_END_DDS_NAMESPACE_DECL

inline bool is_valid_dcps_domain(int dcps_domain)
{
    return ace_range(0, 230, dcps_domain) == dcps_domain;
}

#if defined (__ACE_INLINE__)
# include "DDSPubSub_T.inl"
#endif /* __ACE_INLINE__ */

#if defined (ACE_TEMPLATES_REQUIRE_SOURCE)
# include "DDSPubSub_T.cpp"
#endif /* ACE_TEMPLATES_REQUIRE_SOURCE */

#if defined (ACE_TEMPLATES_REQUIRE_PRAGMA)
# pragma implementation ("DDSPubSub_T.cpp")
#endif /* ACE_TEMPLATES_REQUIRE_PRAGMA */

#endif // TAFDDS_DDSPUBSUB_H
