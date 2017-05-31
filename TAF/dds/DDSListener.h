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
#ifndef TAFDDS_DDSLISTENER_H
#define TAFDDS_DDSLISTENER_H

#include "DDSDefs.h"

#include <iostream>

TAF_BEGIN_DDS_NAMESPACE_DECL

namespace TAFDDS {
    struct Listener : virtual DDS::Listener, ACE_Copy_Disabled {
        virtual ~Listener(void) {}  // Ensure We destruct OK
    };

    struct TopicListener : virtual TAFDDS::Listener, virtual DDS::TopicListener
    {
        virtual void on_inconsistent_topic(DDS::Topic_ptr, const DDS::InconsistentTopicStatus&)
        {
            std::cout << "DDS::TopicListener::on_inconsistent_topic." << std::endl;
        }
    };

    struct DataWriterListener : virtual TAFDDS::Listener, virtual DDS::DataWriterListener
    {
        virtual void on_offered_deadline_missed(DDS::DataWriter_ptr, const DDS::OfferedDeadlineMissedStatus&)
        {
            std::cout << "DDS::DataWriterListener::on_offered_deadline_missed." << std::endl;
        }

        virtual void on_offered_incompatible_qos(DDS::DataWriter_ptr, const DDS::OfferedIncompatibleQosStatus&)
        {
            std::cout << "DDS::DataWriterListener::on_offered_incompatible_qos." << std::endl;
        }

        virtual void on_liveliness_lost(DDS::DataWriter_ptr, const DDS::LivelinessLostStatus&)
        {
            std::cout << "DDS::DataWriterListener::on_liveliness_lost." << std::endl;
        }

        virtual void on_publication_matched(DDS::DataWriter_ptr, const DDS::PublicationMatchedStatus&)
        {
            std::cout << "DDS::DataWriterListener::on_publication_matched." << std::endl;
        }

#if defined(TAF_USES_NDDS)

        virtual void on_reliable_writer_cache_changed(DDS::DataWriter_ptr, const DDS::ReliableWriterCacheChangedStatus&)
        {
            std::cout << "DDS::DataWriterListener::on_reliable_writer_cache_changed." << std::endl;
        }

        virtual void on_reliable_reader_activity_changed(DDS::DataWriter_ptr, const DDS_ReliableReaderActivityChangedStatus&)
        {
            std::cout << "DDS::DataWriterListener::on_reliable_reader_activity_changed." << std::endl;
        }

#endif
    };

    struct DataReaderListener : virtual TAFDDS::Listener, virtual DDS::DataReaderListener
    {
        virtual void on_data_available(DDS::DataReader_ptr)
        {
            std::cout << "DDS::DataReaderListener::on_data_available." << std::endl;
        }

        virtual void on_requested_deadline_missed(DDS::DataReader_ptr, const DDS::RequestedDeadlineMissedStatus&)
        {
            std::cout << "DDS::DataReaderListener::on_requested_deadline_missed." << std::endl;
        }

        virtual void on_requested_incompatible_qos(DDS::DataReader_ptr, const DDS::RequestedIncompatibleQosStatus&)
        {
            std::cout << "DDS::DataReaderListener::on_requested_incompatible_qos." << std::endl;
        }

        virtual void on_sample_rejected(DDS::DataReader_ptr, const DDS::SampleRejectedStatus&)
        {
            std::cout << "DDS::DataReaderListener::on_sample_rejected." << std::endl;
        }

        virtual void on_liveliness_changed(DDS::DataReader_ptr, const DDS::LivelinessChangedStatus&)
        {
            std::cout << "DDS::DataReaderListener::on_liveliness_changed." << std::endl;
        }

        virtual void on_sample_lost(DDS::DataReader_ptr, const DDS::SampleLostStatus&)
        {
            std::cout << "DDS::DataReaderListener::on_sample_lost." << std::endl;
        }

        virtual void on_subscription_matched(DDS::DataReader_ptr, const DDS::SubscriptionMatchedStatus&)
        {
            std::cout << "DDS::DataReaderListener::on_subscription_matched." << std::endl;
        }
    };

    struct PublisherListener    : virtual DDS::PublisherListener, virtual TAFDDS::DataWriterListener
    {
        virtual void on_publication_matched(DDS::DataWriter_ptr, const DDS::PublicationMatchedStatus&)
        {
            std::cout << "DDS::PublisherListener::on_publication_matched." << std::endl;
        }
    };

    struct SubscriberListener   : virtual DDS::SubscriberListener, virtual TAFDDS::DataReaderListener
    {
        virtual void on_subscription_matched(DDS::DataReader_ptr, const DDS::SubscriptionMatchedStatus&)
        {
            std::cout << "DDS::SubscriberListener::on_subscription_matched." << std::endl;
        }

        virtual void on_data_on_readers(DDS::Subscriber_ptr)
        {
            std::cout << "DDS::SubscriberListener::on_data_on_readers." << std::endl;
        }
    };

    struct DomainParticipantListener :  virtual DDS::DomainParticipantListener,
                                        virtual TAFDDS::TopicListener,
                                        virtual TAFDDS::PublisherListener,
                                        virtual TAFDDS::SubscriberListener
    {
        virtual void on_publication_matched(DDS::DataWriter_ptr, const DDS::PublicationMatchedStatus&)
        {
            std::cout << "DDS::DomainParticipantListener::on_publication_matched." << std::endl;
        }

        virtual void on_subscription_matched(DDS::DataReader_ptr, const DDS::SubscriptionMatchedStatus&)
        {
            std::cout << "DDS::DomainParticipantListener::on_subscription_matched." << std::endl;
        }
    };
} // namespace TAFDDS

TAF_END_DDS_NAMESPACE_DECL

#endif /* TAFDDS_DDSLISTENER_H_ */
