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
#ifndef TAFDDS_DDSPUBSUB_T_CPP
#define TAFDDS_DDSPUBSUB_T_CPP

#include "DDSPubSub.h"

#if !defined (__ACE_INLINE__)
# include "DDSPubSub_T.inl"
#endif /* __ACE_INLINE__ */

#include <iostream>

TAF_BEGIN_DDS_NAMESPACE_DECL

namespace TAFDDS
{
    /******************* DDS_Topic  *************************************************/

    template < typename T_SUPPORT >
    DDS::ReturnCode_t
        DDS_Topic<T_SUPPORT>::init(const DDS_DomainParticipant_handle &participant, DDS::String_ptr topic_name, DDS::String_ptr type_name)
    {
        if (participant == 0 || topic_name == 0) {
            return DDS::RETCODE_ERROR;
        }
        else if ((this->out() = participant->create_topic(topic_name, new T_SUPPORT(type_name))._retn()) == 0) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: DDS_Topic - Unable to create Topic [%C]\n"), topic_name), DDS::RETCODE_ERROR);
        }

        this->topic_name_.assign(DDS::String_ref((**this)->get_name()));

        return DDS::RETCODE_OK;
    }

    /******************* DDS_Publisher ********************************************/

    template <typename T_LISTENER>
    DDS::ReturnCode_t
    DDS_Publisher<T_LISTENER>::init(const DDS_DomainParticipant_handle &participant)
    {
        if (participant == 0) {
            return DDS::RETCODE_BAD_PARAMETER;
        }

        DDS::ReturnCode_t   r_code;

#if defined(TAF_USES_COREDX)
        if ((r_code = (*participant)->get_default_publisher_qos(&this->qos_)) == DDS::RETCODE_OK) try {
#else
        if ((r_code = (*participant)->get_default_publisher_qos(this->qos_)) == DDS::RETCODE_OK) try {
#endif
            if ((r_code = this->getQos(this->qos_)) == DDS::RETCODE_OK) {

                DDS::Publisher_ref  publisher((*participant)->create_publisher(this->qos_, this, this->getStatusMask()));

                if (publisher && (this->out() = new DDS_Publisher_holder(participant, publisher)) != 0) {
                    return DDS::RETCODE_OK;
                }

                r_code = DDS::RETCODE_ERROR;
            }
        } DAF_CATCH_ALL {  /* Safe Guard UserCode from Exceptions */ }

        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: DDS_Publisher - Unable to create publisher.\n")), r_code);
    }

    /******************* DDS_Writer ***********************************************/

    template <typename T_TOPIC, typename T_LISTENER>
    DDS_Writer<T_TOPIC,T_LISTENER>::~DDS_Writer(void)
    {
        if (this->writer_) {
            ACE_Guard<ACE_SYNCH_MUTEX> write_guard(this->writer_lock_); if (this->writer_) {
                this->writer_->set_listener(0, DDS::STATUS_MASK_ALL);
                (*this->publisher_)->delete_datawriter(this->writer_); this->writer_ = 0;
            }
        }
    }

    template <typename T_TOPIC, typename T_LISTENER>
    DDS::ReturnCode_t
    DDS_Writer<T_TOPIC,T_LISTENER>::init(const DDS_Publisher_handle &publisher, const DDS_Topic_handle &topic)
    {
        if (publisher == 0 || topic == 0) {
            return DDS::RETCODE_BAD_PARAMETER;
        }

        DDS::ReturnCode_t   r_code;

#if defined(TAF_USES_COREDX)
        if ((r_code = (*publisher)->get_default_datawriter_qos(&this->qos_)) == DDS::RETCODE_OK) try {
#else
        if ((r_code = (*publisher)->get_default_datawriter_qos(this->qos_)) == DDS::RETCODE_OK) try {
#endif
            if ((r_code = this->getQos(this->qos_)) == DDS::RETCODE_OK) {
                this->writer_ = _support_type::narrow(DDS::DataWriter_ref((*publisher)->create_datawriter(topic->handle_in(), this->qos_, this, this->getStatusMask())));

                if (this->writer_) {
                    this->publisher_ = publisher; this->topic_ = topic; return DDS::RETCODE_OK;
                }

                r_code = DDS::RETCODE_ERROR;
            }
        } DAF_CATCH_ALL {  /* Safe Guard UserCode from Exceptions */ }

        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: DDS_Writer - Unable to create DataWriter.\n")), r_code);
    }

    template <typename T_SUPPORT, typename T_LISTENER>
    DDS::ReturnCode_t
    DDS_Writer<T_SUPPORT,T_LISTENER>::publish(const _data_type &data)
    {
        if (this->writer_) try {

            ACE_GUARD_RETURN (ACE_SYNCH_MUTEX, write_guard, this->writer_lock_, DDS::RETCODE_OUT_OF_RESOURCES);

            if (this->writer_) {
#if defined(TAF_USES_COREDX)
                DDS::ReturnCode_t wRC(this->writer_->write(&data, this->writer_->register_instance(&data)));
#else
                DDS::ReturnCode_t wRC(this->writer_->write(data, this->writer_->register_instance(data)));
#endif
                if (wRC == DDS::RETCODE_OK) {
                    return DDS::RETCODE_OK;
                } else {
                    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: Writing data (RC=%d).\n"), int(wRC)), wRC);
                }
            }
        } DAF_CATCH_ALL {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: DataWriter exception Encountered.\n")), DDS::RETCODE_ERROR);
        }

        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: DataWriter not initialized.\n")), DDS::RETCODE_PRECONDITION_NOT_MET);
    }

    /******************* DDS_Subscriber *******************************************/

    template <typename T_LISTENER>
    DDS::ReturnCode_t
    DDS_Subscriber<T_LISTENER>::init(const DDS_DomainParticipant_handle &participant)
    {
        if (participant == 0) {
            return DDS::RETCODE_BAD_PARAMETER;
        }

        DDS::ReturnCode_t   r_code;

#if defined(TAF_USES_COREDX)
        if ((r_code = (*participant)->get_default_subscriber_qos(&this->qos_)) == DDS::RETCODE_OK) try {
#else
        if ((r_code = (*participant)->get_default_subscriber_qos(this->qos_)) == DDS::RETCODE_OK) try {
#endif
            if ((r_code = this->getQos(this->qos_)) == DDS::RETCODE_OK) {
                DDS::Subscriber_ref subscriber((*participant)->create_subscriber(this->qos_, this, this->getStatusMask()));

                if (subscriber && (this->out() = new DDS_Subscriber_holder(participant, subscriber)) != 0) {
                    return DDS::RETCODE_OK;
                }

                r_code = DDS::RETCODE_ERROR;
            }
        } DAF_CATCH_ALL {  /* Safe Guard UserCode from Exceptions */ }

        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: DDS_Subscriber - Unable to create subscriber.\n")), r_code);
    }

    /******************* DDS_Reader **********************************************/

    template <typename T_TOPIC, typename T_LISTENER> DDS::ReturnCode_t
    DDS_Reader<T_TOPIC,T_LISTENER>::init(const DDS_Subscriber_handle &subscriber, const DDS_Topic_handle &topic)
    {
        if (subscriber == 0 || topic == 0) {
            return DDS::RETCODE_BAD_PARAMETER;
        }

        DDS::ReturnCode_t   r_code;

#if defined(TAF_USES_COREDX)
        if ((r_code = (*subscriber)->get_default_datareader_qos(&this->qos_)) == DDS::RETCODE_OK) try {
#else
        if ((r_code = (*subscriber)->get_default_datareader_qos(this->qos_)) == DDS::RETCODE_OK) try {
#endif
            if ((r_code = this->getQos(this->qos_)) == DDS::RETCODE_OK) {
                this->reader_ = _support_type::narrow(DDS::DataReader_ref((*subscriber)->create_datareader(topic->handle_in(), this->qos_, this, this->getStatusMask())));

                if (this->reader_) {
                    this->subscriber_ = subscriber; this->topic_ = topic; return DDS::RETCODE_OK;
                }

                r_code = DDS::RETCODE_ERROR;
            }
        } DAF_CATCH_ALL {  /* Safe Guard UserCode from Exceptions */ }

        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: DDS_Reader - Unable to create DataReader.\n")), r_code);
    }

    template <typename T_TOPIC, typename T_LISTENER>
    DDS_Reader<T_TOPIC,T_LISTENER>::~DDS_Reader(void)
    {
        if (this->reader_) {
            ACE_Guard<ACE_SYNCH_MUTEX> read_guard(this->reader_lock_); if (this->reader_) {
                this->reader_->set_listener(0, DDS::STATUS_MASK_ALL);
                (*this->subscriber_)->delete_datareader(this->reader_); this->reader_ = 0;
            }
        }
    }

    template <typename T_TOPIC, typename T_LISTENER> void
    DDS_Reader<T_TOPIC,T_LISTENER>::on_data_available(DDS::DataReader_ptr rdr)
    {
        ACE_Guard<ACE_SYNCH_MUTEX> read_guard(this->reader_lock_, false);
        if (read_guard.locked()) {
            _listener_type::on_data_available(rdr);
        }
    }

    /******************* TOPICReaderListener *****************************************/

    template <typename T_SUPPORT> DDS::ReturnCode_t
    TOPICReaderListener<T_SUPPORT>::on_data_available(const _data_type&)
    {
        return DDS::RETCODE_OK;
    }

    template <typename T_SUPPORT> void
    TOPICReaderListener<T_SUPPORT>::on_data_available(DDS::DataReader_ptr rdr)
    {
        _data_reader_stub_type_ref d_rdr(rdr ? _support_type::narrow(rdr) : 0);

        if (d_rdr) try {
#if defined(TAF_USES_OPENDDS) && (OPENDDS_USES_TAKE_NEXT_SAMPLE == 1)

            _data_type data_t; DDS::SampleInfo si;

            if (d_rdr->take_next_sample(data_t, si) == DDS::RETCODE_OK)
            {
                if (si.valid_data) try {
                    this->on_data_available(data_t);
                } DAF_CATCH_ALL {
                    /* Ignore Application Error */
                }
            }

#elif defined(TAF_USES_COREDX)

            _data_seq_type data_t; DDS::SampleInfoSeq si;

            if (d_rdr->take(&data_t, &si, TOPIC_SAMPLES_MAX,
                            DDS::ANY_SAMPLE_STATE,
                            DDS::ANY_VIEW_STATE,
                            DDS::ANY_INSTANCE_STATE) == DDS::RETCODE_OK)
            {
                unsigned len = ace_min(si.size(), data_t.size());

                for (unsigned i = 0; i < len; i++) {
                    if (si[i] && si[i]->valid_data) try {
                        if (data_t[i]) { this->on_data_available(*data_t[i]); }
                    } DAF_CATCH_ALL {
                        /* Ignore Application Error */
                    }
                }

                d_rdr->return_loan(&data_t, &si);
            }
#else
            _data_seq_type data_t(TOPIC_SAMPLES_MAX); DDS::SampleInfoSeq si(TOPIC_SAMPLES_MAX);

            if (d_rdr->take( data_t, si, TOPIC_SAMPLES_MAX,
                             DDS::ANY_SAMPLE_STATE,
                             DDS::ANY_VIEW_STATE,
                             DDS::ANY_INSTANCE_STATE) == DDS::RETCODE_OK)
            {
                unsigned len = ace_min(si.length(), data_t.length());

                for (unsigned i = 0; i < len; i++) {
                    if (si[i].valid_data) try {
                        this->on_data_available(data_t[i]);
                    } DAF_CATCH_ALL {
                        /* Ignore Application Error */
                    }
                }

                d_rdr->return_loan(data_t, si);
            }
#endif
        } DAF_CATCH_ALL { /* Ignore DDS Read Error */ }
    }
} // namespace TAFDDS

TAF_END_DDS_NAMESPACE_DECL

#endif  // TAFDDS_DDSPUBSUB_T_CPP
