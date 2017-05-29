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
#ifndef TAF_STRINGMANAGER_T_H
#define TAF_STRINGMANAGER_T_H

/************************ StringSequence_T *************************
*
* AUTHOR:   Derek Dominish <derek.dominish@dsto.defence.gov.au>
* DATE:     30th March 2016
*
*****************************************************************/

#include "VectorSequence_T.h"

#include <tao/CORBA_String.h>
#include <tao/String_Manager_T.h>
#include <tao/StringSeqC.h>

#include <string>

namespace TAF
{
    template <typename T = CORBA::Char>
    struct StringManager_T : std::basic_string< T, std::char_traits<T>, std::allocator<T> >
    {
        typedef std::basic_string< T, std::char_traits<T>, std::allocator<T> >  _string_type;

        typedef TAO::String_Manager_T<T>    _mgr_type;
        typedef TAO::String_var<T>          _var_type;
        typedef TAO::String_out<T>          _out_type;

        StringManager_T(const _var_type &s = 0) : _string_type()
        {
            const T *p(s); if (p) this->assign(p);
        }

        StringManager_T(const StringManager_T<T> &s) : _string_type(s)
        {
        }

        StringManager_T<T> & operator = (const StringManager_T<T> &s)
        {
            this->assign(s); return *this;
        }

        StringManager_T<T> & operator = (const _string_type &s)
        {
            this->assign(s); return *this;
        }

        operator _var_type () const
        {
            return this->c_str();
        }

        T const * in(void) const
        {
            return this->c_str();
        }

        T * _retn(void) const  // Returns unowned T*
        {
            return _var_type(*this)._retn();
        }
    };

    template <typename T = CORBA::StringSeq>
    struct StringSequence_T : TAF::VectorSequence_T< T, StringManager_T<typename T::character_type> >
    {
        typedef TAF::VectorSequence_T< T, StringManager_T<typename T::character_type> > _vector_sequence;
        typedef typename _vector_sequence::_vector_type                                 _vector_type;

        StringSequence_T(const typename T::_var_type &s = 0) : _vector_sequence(s) {}
        StringSequence_T(const _vector_sequence &s) : _vector_sequence(s) {}
        StringSequence_T(const _vector_type &s) : _vector_sequence(s) {}
        StringSequence_T(const T &s) : _vector_sequence(s) {}
    };

} // namespace TAF

template <typename T>
inline bool operator <  (const TAF::StringManager_T<T> &lhs, const TAF::StringManager_T<T> &rhs)
{
    return DAF_OS::strcmp(lhs.in(), rhs.in()) < 0;
}

#endif // TAF_STRINGMANAGER_T_H
