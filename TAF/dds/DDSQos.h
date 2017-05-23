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

#ifndef TAFDDS_DDSQOS_H
#define TAFDDS_DDSQOS_H

#include "DDSDefs.h"

#if defined(TAF_USES_NDDS)
# define DEFINE_DDS_QOSSUPPORT(CLS,TYP)                                     \
class _##CLS##_##TYP##QosSupport : public CLS::TYP, ACE_Copy_Disabled {     \
public:                                                                     \
    _##CLS##_##TYP##QosSupport(void)  { CLS##_##TYP##_initialize(this); }   \
    ~_##CLS##_##TYP##QosSupport(void) { CLS##_##TYP##_finalize(this); }     \
} /* Note!!! No closing ';' to force user to close macro usage */
#else
# define DEFINE_DDS_QOSSUPPORT(CLS,TYP)                                     \
class _##CLS##_##TYP##QosSupport : public CLS::TYP, ACE_Copy_Disabled {     \
public:                                                                     \
    _##CLS##_##TYP##QosSupport(void)  {}                                    \
    ~_##CLS##_##TYP##QosSupport(void) {}                                    \
} /* Note!!! No closing ';' to force user to close macro usage */
#endif

TAF_BEGIN_DDS_NAMESPACE_DECL

namespace TAFDDS
{
    typedef DEFINE_DDS_QOSSUPPORT(DDS,DomainParticipantQos)     DomainParticipantQos;
    typedef DEFINE_DDS_QOSSUPPORT(DDS,TopicQos)                 TopicQos;
    typedef DEFINE_DDS_QOSSUPPORT(DDS,PublisherQos)             PublisherQos;
    typedef DEFINE_DDS_QOSSUPPORT(DDS,SubscriberQos)            SubscriberQos;
    typedef DEFINE_DDS_QOSSUPPORT(DDS,DataWriterQos)            DataWriterQos;
    typedef DEFINE_DDS_QOSSUPPORT(DDS,DataReaderQos)            DataReaderQos;

    /************************************************************************************************/
    template <typename T> struct QOSPolicyTraits
    {
        static void _init(T&);
        static void _copy(T&, const T&);
        static void _fini(T&);
    };
    /************************************************************************************************/

#if defined(TAF_USES_OPENDDS)  // These Copy and Initialize fine

    template <typename T> inline void
    QOSPolicyTraits<T>::_init(T &t) {
        t = T();  // Uses CORBA So assignment works OK
    }

    template <typename T> inline void
    QOSPolicyTraits<T>::_fini(T &t) {
        t = T();  // Uses CORBA So assignment works OK
    }

    template <typename T> inline void
    QOSPolicyTraits<T>::_copy(T &t, const T &from) {
        t = from;  // Uses CORBA So assignment works OK
    }

#elif defined(TAF_USES_NDDS)

    template <typename T> inline void
    QOSPolicyTraits<T>::_init(T &t) {
        t = T();
    }

    template <typename T> inline void
    QOSPolicyTraits<T>::_fini(T &t) {
        t = T();
    }

    template <typename T> inline void
    QOSPolicyTraits<T>::_copy(T &t, const T &from) {
        QOSPolicyTraits<T>::_fini(t); t = from;  // ALL Types HAVE Assignement Operators
    }

#elif defined(TAF_USES_COREDX)

    template <typename T> inline void
    QOSPolicyTraits<T>::_init(T &t) {
        t.clear();
    }

    template <typename T> inline void
    QOSPolicyTraits<T>::_fini(T &t) {
        t.clear();
    }

    template <typename T> inline void
    QOSPolicyTraits<T>::_copy(T &t, const T &from) {
        t.copy(&from);  // ALL Types Copy Methods
    }

    template <typename T> inline T& operator << (T &qos, const DDS::ThreadModelQosPolicy &policy)
        { qos.thread_model = policy; return qos; }
    template <typename T> inline T& operator << (T &qos, const DDS::LoggingQosPolicy &policy)
        { qos.logging = policy; return qos; }
    template <typename T> inline T& operator << (T &qos, const DDS::PeerParticipantQosPolicy &policy)
        { qos.peer_participants = policy; return qos; }
    template <typename T> inline T& operator << (T &qos, const DDS::RTPSWriterQosPolicy &policy)
        { qos.rtps_writer = policy; return qos; }
    template <typename T> inline T& operator << (T &qos, const DDS::RTPSReaderQosPolicy &policy)
        { qos.rtps_reader = policy; return qos; }

#endif

    template <typename T> inline T& operator << (T &qos, const DDS::EntityFactoryQosPolicy &policy)
        { QOSPolicyTraits<DDS::EntityFactoryQosPolicy>::_copy(qos.entity_factory,policy); return qos; }
    template <typename T> inline T& operator << (T &qos, const DDS::UserDataQosPolicy &policy)
        { QOSPolicyTraits<DDS::UserDataQosPolicy>::_copy(qos.user_data,policy); return qos; }
    template <typename T> inline T& operator << (T &qos, const DDS::TopicDataQosPolicy &policy)
        { QOSPolicyTraits<DDS::TopicDataQosPolicy>::_copy(qos.topic_data,policy); return qos; }
    template <typename T> inline T& operator << (T &qos, const DDS::DurabilityQosPolicy &policy)
        { QOSPolicyTraits<DDS::DurabilityQosPolicy>::_copy(qos.durability,policy); return qos; }
    template <typename T> inline T& operator << (T &qos, const DDS::DurabilityServiceQosPolicy &policy)
        { QOSPolicyTraits<DDS::DurabilityServiceQosPolicy>::_copy(qos.durability_service,policy); return qos; }
    template <typename T> inline T& operator << (T &qos, const DDS::DeadlineQosPolicy &policy)
        { QOSPolicyTraits<DDS::DeadlineQosPolicy>::_copy(qos.deadline,policy); return qos; }
    template <typename T> inline T& operator << (T &qos, const DDS::LatencyBudgetQosPolicy &policy)
        { QOSPolicyTraits<DDS::LatencyBudgetQosPolicy>::_copy(qos.latency_budget,policy); return qos; }
    template <typename T> inline T& operator << (T &qos, const DDS::LivelinessQosPolicy &policy)
        { QOSPolicyTraits<DDS::LivelinessQosPolicy>::_copy(qos.liveliness,policy); return qos; }
    template <typename T> inline T& operator << (T &qos, const DDS::ReliabilityQosPolicy &policy)
        { QOSPolicyTraits<DDS::ReliabilityQosPolicy>::_copy(qos.reliability,policy); return qos; }
    template <typename T> inline T& operator << (T &qos, const DDS::DestinationOrderQosPolicy &policy)
        { QOSPolicyTraits<DDS::DestinationOrderQosPolicy>::_copy(qos.destination_order,policy); return qos; }
    template <typename T> inline T& operator << (T &qos, const DDS::HistoryQosPolicy &policy)
        { QOSPolicyTraits<DDS::HistoryQosPolicy>::_copy(qos.history,policy); return qos; }
    template <typename T> inline T& operator << (T &qos, const DDS::ResourceLimitsQosPolicy &policy)
        { QOSPolicyTraits<DDS::ResourceLimitsQosPolicy>::_copy(qos.resource_limits,policy); return qos; }
    template <typename T> inline T& operator << (T &qos, const DDS::TransportPriorityQosPolicy &policy)
        { QOSPolicyTraits<DDS::TransportPriorityQosPolicy>::_copy(qos.transport_priority,policy); return qos; }
    template <typename T> inline T& operator << (T &qos, const DDS::LifespanQosPolicy &policy)
        { QOSPolicyTraits<DDS::LifespanQosPolicy>::_copy(qos.lifespan,policy); return qos; }
    template <typename T> inline T& operator << (T &qos, const DDS::OwnershipQosPolicy &policy)
        { QOSPolicyTraits<DDS::OwnershipQosPolicy>::_copy(qos.ownership,policy); return qos; }
    template <typename T> inline T& operator << (T &qos, const DDS::OwnershipStrengthQosPolicy &policy)
        { QOSPolicyTraits<DDS::OwnershipStrengthQosPolicy>::_copy(qos.ownership_strength,policy); return qos; }
    template <typename T> inline T& operator << (T &qos, const DDS::PresentationQosPolicy &policy)
        { QOSPolicyTraits<DDS::PresentationQosPolicy>::_copy(qos.presentation,policy); return qos; }
    template <typename T> inline T& operator << (T &qos, const DDS::PartitionQosPolicy &policy)
        { QOSPolicyTraits<DDS::PartitionQosPolicy>::_copy(qos.partition,policy); return qos; }
    template <typename T> inline T& operator << (T &qos, const DDS::GroupDataQosPolicy &policy)
        { QOSPolicyTraits<DDS::GroupDataQosPolicy>::_copy(qos.group_data,policy); return qos; }
    template <typename T> inline T& operator << (T &qos, const DDS::WriterDataLifecycleQosPolicy &policy)
        { QOSPolicyTraits<DDS::WriterDataLifecycleQosPolicy>::_copy(qos.writer_data_lifecycle,policy); return qos; }
    template <typename T> inline T& operator << (T &qos, const DDS::ReaderDataLifecycleQosPolicy &policy)
        { QOSPolicyTraits<DDS::ReaderDataLifecycleQosPolicy>::_copy(qos.reader_data_lifecycle,policy); return qos; }

#if defined(TAF_USES_COREDX) || defined(TAF_USES_NDDS)
    template <typename T> inline T& operator << (T &qos, const DDS::EntityNameQosPolicy &policy)
        { qos.entity_name = policy; return qos; }
    template <typename T> inline T& operator << (T &qos, const DDS::DiscoveryQosPolicy &policy)
        { qos.discovery = policy; return qos; }
#endif

    /************************************************************************************************/

    const struct EntityFactoryQosPolicy : DDS::EntityFactoryQosPolicy {
        EntityFactoryQosPolicy(DDS::Boolean autoenable_created_entities) {
            this->autoenable_created_entities = autoenable_created_entities;
        }
    } ENTITYFACTORY_QOS_DEFAULT(true); DAF_UNUSED_STATIC(ENTITYFACTORY_QOS_DEFAULT)

    const struct PresentationQosPolicy : DDS::PresentationQosPolicy {
        PresentationQosPolicy(const DDS::PresentationQosPolicyAccessScopeKind &access_scope, DDS::Boolean coherent_access = false, DDS::Boolean ordered_access = false) {
            this->access_scope = access_scope; this->coherent_access = coherent_access; this->ordered_access = ordered_access;
        }
    } PRESENTATION_QOS_DEFAULT(DDS::INSTANCE_PRESENTATION_QOS, false, false); DAF_UNUSED_STATIC(PRESENTATION_QOS_DEFAULT)

    const struct HistoryQosPolicy : DDS::HistoryQosPolicy {
        HistoryQosPolicy(const DDS::HistoryQosPolicyKind &kind, DDS::Long depth = 1) {
            this->kind = kind; this->depth = depth;
#if defined(TAF_USES_NDDS)
            this->refilter = DDS::NONE_REFILTER_QOS;
#endif
        }
    } HISTORY_QOS_DEFAULT(DDS::KEEP_LAST_HISTORY_QOS, 1L); DAF_UNUSED_STATIC(HISTORY_QOS_DEFAULT)

    const struct ReliabilityQosPolicy : DDS::ReliabilityQosPolicy {
        ReliabilityQosPolicy(const DDS::ReliabilityQosPolicyKind &kind, const DDS::Duration_t &max_blocking_time = ::TAFDDS::Duration_ZERO) {
            this->kind = kind; this->max_blocking_time = max_blocking_time;
        }
    } RELIABILITY_QOS_DEFAULT(DDS::BEST_EFFORT_RELIABILITY_QOS, TAFDDS::Duration_t(0L, 100000000UL)); DAF_UNUSED_STATIC(RELIABILITY_QOS_DEFAULT)

    const struct LivelinessQosPolicy : DDS::LivelinessQosPolicy {
        LivelinessQosPolicy(const DDS::LivelinessQosPolicyKind &kind, const DDS::Duration_t &lease_duration = ::TAFDDS::Duration_INFINITE) {
            this->kind = kind; this->lease_duration = lease_duration;
        }
    } LIVELINESS_QOS_DEFAULT(DDS::AUTOMATIC_LIVELINESS_QOS, Duration_INFINITE); DAF_UNUSED_STATIC(LIVELINESS_QOS_DEFAULT)

    const struct DurabilityQosPolicy : DDS::DurabilityQosPolicy {
        DurabilityQosPolicy(const DDS::DurabilityQosPolicyKind &kind, DDS::Boolean direct_communication = true) {
            this->kind = kind;
#if defined(TAF_USES_NDDS)
            this->direct_communication = direct_communication;
#else
            ACE_UNUSED_ARG(direct_communication);
#endif
        }
    } DURABILITY_QOS_DEFAULT(DDS::VOLATILE_DURABILITY_QOS); DAF_UNUSED_STATIC(DURABILITY_QOS_DEFAULT)

    const struct DeadlineQosPolicy : DDS::DeadlineQosPolicy {
        DeadlineQosPolicy(const DDS::Duration_t &period) {
            this->period = period;
        }
    } DEADLINE_QOS_DEFAULT(TAFDDS::Duration_INFINITE); DAF_UNUSED_STATIC(DEADLINE_QOS_DEFAULT)

    const struct LatencyBudgetQosPolicy : DDS::LatencyBudgetQosPolicy {
        LatencyBudgetQosPolicy(const DDS::Duration_t &duration) {
            this->duration = duration;
        }
    } LATENCY_BUDGET_QOS_DEFAULT(TAFDDS::Duration_ZERO); DAF_UNUSED_STATIC(LATENCY_BUDGET_QOS_DEFAULT)

    const DDS::OwnershipQosPolicyKind     OWNERSHIP_QOS_DEFAULT(DDS::SHARED_OWNERSHIP_QOS); DAF_UNUSED_STATIC(OWNERSHIP_QOS_DEFAULT)
}//namespace TAFDDS

TAF_END_DDS_NAMESPACE_DECL

#endif
