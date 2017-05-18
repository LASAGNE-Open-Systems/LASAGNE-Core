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
#ifndef TAFDDS_MANAGEDTYPE_T_H
#define TAFDDS_MANAGEDTYPE_T_H

#include "daf/RefCountHandler_T.h"

#include <map>

/*
 * NOTE: namespace is TAF (NOT TAFDDS). There is some reasoning to this decision.
 * Firstly, the types below are templates. Therefore, symbols generated should be
 * unique due to the template argument. Secondly, they are all private symbols which
 * are not exported, therefore there is no reason to include the TAF_BEGIN_DDS_NAMESPACE etc macros. Thirdly, It aids re-use in other parts of TAF, namely the gsoap support.
 */
namespace TAF {
    template <typename K, typename T>
    class DDS_Domain_holder : protected std::map< K, typename DAF::RefCountHandler_T<T>::_ptr_type >
        , ACE_Copy_Disabled
    {
        mutable ACE_SYNCH_MUTEX lock_;

    public:

        typedef typename std::map< K, typename DAF::RefCountHandler_T<T>::_ptr_type >   map_type;
        typedef typename map_type::value_type                                           value_type;
        typedef typename map_type::key_type                                             key_type;
        typedef typename map_type::mapped_type                                          mapped_type;

        class DDS_holder : public DAF::RefCountHandler_T<T>
        {
            DDS_Domain_holder & holder_home_;

        public:

            DDS_holder(DDS_Domain_holder &holder_home, const T &ref)
                : DAF::RefCountHandler_T<T> (ref)
                , holder_home_              (holder_home)
            {}

        protected:

            virtual void __remove(void)
            {
                this->holder_home_.remove_holder(this); delete this;
            }
        };

        typedef typename DDS_holder::_ptr_type  _holder_ptr_type;
        typedef typename DDS_holder::_ref_type  _holder_ref_type;
        typedef typename DDS_holder::_out_type  _holder_out_type;

        _holder_ref_type   locate_holder(const key_type &key) const;

    protected:

        operator ACE_SYNCH_MUTEX & () const
        {
            return this->lock_;
        }

        /* Implementation needs to be here because of dependant return type */
        _holder_ref_type    create_holder(const value_type &val);
        size_t              remove_holder(const mapped_type &); // Returns map::size()
    };

    template < typename K, typename T >
    typename DDS_Domain_holder<K, T>::_holder_ref_type
    DDS_Domain_holder<K, T>::locate_holder(const key_type &key) const
    {   // Locked on entry
        for (typename map_type::const_iterator it = this->find(key); it != this->end();) {
            return DDS_holder::_duplicate(it->second);
        }
        DAF_THROW_EXCEPTION(DAF::NotFoundException);
    }

    template < typename K, typename T >
    typename DDS_Domain_holder<K, T>::_holder_ref_type
    DDS_Domain_holder<K, T>::create_holder(const value_type &val)
    {   // Locked on entry
        if (val.second) {
            for (typename std::pair<typename map_type::iterator, bool> ib = this->insert(val); ib.second;) {
                return (ib.first)->second; // NOTE: No Duplicate! - 1st client now owns the map contained reference
            }
        }
        DAF_THROW_EXCEPTION(DAF::ObjectNotExistException);
    }

    template < typename K, typename T >
    size_t
    DDS_Domain_holder<K, T>::remove_holder(const mapped_type &val)
    {
        ACE_Guard <ACE_SYNCH_MUTEX> mon(*this);
        if (mon.locked()) for (typename map_type::iterator it = this->begin(); it != this->end(); it++) {
            if (val == it->second) {
                this->erase(it); break;
            }
        }
        return this->size();
    }

} // namespace TAF

#endif  // TAFDDS_MANAGEDTYPE_T_H
