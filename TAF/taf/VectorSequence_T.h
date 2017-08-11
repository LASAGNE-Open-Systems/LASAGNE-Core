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
#ifndef TAF_VECTORSEQUENCE_T_H
#define TAF_VECTORSEQUENCE_T_H


#include <tao/Basic_Types.h>

#include <vector>

namespace TAF
{
    template <typename T, typename V = typename T::value_type>
    struct VectorSequence_T : std::vector < V >
    {
        typedef T                       _seq_type;
        typedef V                       _value_type;
        typedef std::vector < V >       _vector_type;
        typedef VectorSequence_T<T, V>  _vector_sequence;

        VectorSequence_T(const typename T::_var_type &s = 0);
        VectorSequence_T(const _vector_sequence &s);
        VectorSequence_T(const _vector_type &s);
        VectorSequence_T(const T &s);

        operator typename T::_var_type () const;

        VectorSequence_T<T, V> & operator += (const T &t);
        VectorSequence_T<T, V> & operator += (const _vector_type &s);
        VectorSequence_T<T, V> & operator += (typename T::const_value_type const &t);

        T const in(void) const;
        T *  _retn(void) const; // Returns unowned T*
    };

    template <typename T, typename V> inline
    VectorSequence_T<T, V>::VectorSequence_T(const typename T::_var_type &s) : _vector_type()
    {
        typename T::const_value_type const *p(s.ptr() ? s->get_buffer() : 0);
        if (p) {
            this->assign(p, p + s->length());
        }
    }

    template <typename T, typename V> inline
    VectorSequence_T<T, V>::VectorSequence_T(const _vector_sequence &s) : _vector_type(s)
    {
    }

    template <typename T, typename V> inline
    VectorSequence_T<T, V>::VectorSequence_T(const _vector_type &s) : _vector_type(s)
    {
    }

    template <typename T, typename V> inline
    VectorSequence_T<T, V>::VectorSequence_T(const T &t) : _vector_type()
    {
        typename T::const_value_type const *p(t.get_buffer());
        if (p) {
            this->assign(p, p + t.length());
        }
    }

    template <typename T, typename V>
    VectorSequence_T<T, V>::operator typename T::_var_type () const
    {
        typename T::_var_type t(new T(CORBA::ULong(this->size())));
        // This needs to be done by hand as _value_type may not be
        // a T::value_type but would have implicit conversions
        for (CORBA::ULong i = 0; i < t->maximum(); i++) {
            t->length(i + 1); t[i] = (*this)[size_t(i)];
        }
        return t._retn();
    }

    template <typename T, typename V> inline VectorSequence_T<T, V> &
    VectorSequence_T<T, V>::operator += (const T &t)
    {
        typename T::const_value_type const *p(t.get_buffer());
        if (p) {
            this->insert(this->end(), p, p + t.length());
        }
        return *this;
    }

    template <typename T, typename V> inline VectorSequence_T<T, V> &
    VectorSequence_T<T, V>::operator += (const _vector_type &t)
    {
        this->insert(this->end(), t.begin(), t.end());
        return *this;
    }

    template <typename T, typename V> inline VectorSequence_T<T, V> &
    VectorSequence_T<T, V>::operator += (typename T::const_value_type const &t)
    {
        this->insert(this->end(), _value_type(t));
        return *this;
    }

    template <typename T, typename V> inline T const
    VectorSequence_T<T, V>::in(void) const
    {
        return typename T::_var_type(*this).in();
    }

    template <typename T, typename V> inline T *
    VectorSequence_T<T, V>::_retn(void) const // Returns unowned T*
    {
        return typename T::_var_type(*this)._retn();
    }

    template <typename T, typename V> inline
    VectorSequence_T<T, V> operator + (const VectorSequence_T<T, V> &s, const T &t)
    {
        return VectorSequence_T<T, V>(s) += t;
    }

    template <typename T, typename V> inline
    VectorSequence_T<T, V> operator + (const VectorSequence_T<T, V> &s, typename T::const_value_type const &t)
    {
        return VectorSequence_T<T, V>(s) += t;
    }

    template <typename T, typename V> inline
    VectorSequence_T<T, V> operator + (const VectorSequence_T<T, V> &s, const typename VectorSequence_T<T,V>::_vector_type &t)
    {
        return VectorSequence_T<T, V>(s) += t;
    }

} // namespace TAF

template <typename T, typename V> inline
CORBA::Boolean operator << (TAO_OutputCDR &strm, const TAF::VectorSequence_T<T, V> &_taf_sequence)
{
    return strm << _taf_sequence.in();
}

template <typename T, typename V> inline
CORBA::Boolean operator >> (TAO_InputCDR &strm, TAF::VectorSequence_T<T, V> &_taf_sequence)
{
    T t; if (strm >> t) {
        _taf_sequence = t; return true;
    }
    return false;
}

#endif // TAF_VECTORSEQUENCE_T_H
