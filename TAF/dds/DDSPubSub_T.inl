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

TAF_BEGIN_DDS_NAMESPACE_DECL

namespace TAFDDS
{
    /******************* DDS_Topic  *************************************************/
    template < typename T_SUPPORT > ACE_INLINE
    DDS::String_ptr
    DDS_Topic<T_SUPPORT>::getTopicName(void) const
    {
        return this->topic_name_.c_str();
    }

    /******************* DDS_Publisher ***********************************************/
    template < typename T_LISTENER > ACE_INLINE
    DDS::ReturnCode_t
    DDS_Publisher<T_LISTENER>::getQos(DDS::PublisherQos &) const
    {
        return DDS::RETCODE_OK;
    }

    template < typename T_LISTENER > ACE_INLINE
    DDS::StatusMask
    DDS_Publisher<T_LISTENER>::getStatusMask(void) const
    {
        return DDS::STATUS_MASK_NONE;
    }

    /******************* DDS_Subscriber *******************************************/
    template < typename T_LISTENER > ACE_INLINE
    DDS::ReturnCode_t
    DDS_Subscriber<T_LISTENER>::getQos(DDS::SubscriberQos &) const
    {
        return DDS::RETCODE_OK;
    }

    template < typename T_LISTENER > ACE_INLINE
    DDS::StatusMask
    DDS_Subscriber<T_LISTENER>::getStatusMask(void) const
    {
        return DDS::STATUS_MASK_NONE;
    }

    /******************* DDS_Writer ***********************************************/
    template < typename T_TOPIC, typename T_LISTENER > ACE_INLINE
    DDS::ReturnCode_t
    DDS_Writer<T_TOPIC,T_LISTENER>::getQos(DDS::DataWriterQos &) const
    {
        return DDS::RETCODE_OK;
    }

    template < typename T_TOPIC, typename T_LISTENER > ACE_INLINE
    DDS::StatusMask
    DDS_Writer<T_TOPIC,T_LISTENER>::getStatusMask(void) const
    {
        return DDS::STATUS_MASK_NONE;
    }

    /******************* DDS_Reader **********************************************/
    template < typename T_TOPIC, typename T_LISTENER > ACE_INLINE
    DDS::ReturnCode_t
    DDS_Reader<T_TOPIC,T_LISTENER>::getQos(DDS::DataReaderQos &) const
    {
        return DDS::RETCODE_OK;
    }

    template < typename T_TOPIC, typename T_LISTENER > ACE_INLINE
    DDS::StatusMask
    DDS_Reader<T_TOPIC,T_LISTENER>::getStatusMask(void) const
    {
        return DDS::STATUS_MASK_ALL;
    }
} // namespace TAFDDS

TAF_END_DDS_NAMESPACE_DECL
