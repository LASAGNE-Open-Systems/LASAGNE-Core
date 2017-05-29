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
#ifndef DAF_OBJECTREF_T_CPP
#define DAF_OBJECTREF_T_CPP

#include "ObjectRef_T.h"

namespace DAF
{
    template <typename T> inline
    void
    ObjectRefTraits<T>::release(T *&p)
    {
        if (p) { p->_remove_ref(); p = ObjectRefTraits<T>::nil(); }
    }

    template <typename T> inline
    T *
    ObjectRefTraits<T>::duplicate(const T *p)
    {
        if (p) { const_cast<T*>(p)->_add_ref(); } return const_cast<T*>(p);
    }

    template <typename T> template <typename A> inline
    T *
    ObjectRefTraits<T>::narrow(const A *p)
    {
        try {
            for(const T *tp = dynamic_cast<const T*>(p); tp;) {
                return ObjectRefTraits<T>::duplicate(tp);
            }
        } DAF_CATCH_ALL {}

        return ObjectRefTraits<T>::nil();
    }

    /**************************************************************************/

    template <typename T> inline
    ObjectRef<T>::ObjectRef(T *p)
        : ptr_(p)
    {
    }

    template <typename T> inline
    ObjectRef<T>::ObjectRef(const T &p)
        : ptr_(ObjectRefTraits<T>::duplicate(&p))
    {
    }

    template <typename T> inline
    ObjectRef<T>::ObjectRef(const ObjectRef<T> &p)
        : ptr_(ObjectRefTraits<T>::duplicate(p.ptr_))
    {
    }

    template <typename T> inline
    ObjectRef<T>::~ObjectRef(void)
    {
        ObjectRefTraits<T>::release(this->ptr_);
    }

    /// Cast operators.
    template <typename T> inline
    ObjectRef<T>::operator T * const & () const
    {
        return this->ptr_;
    }

    template <typename T> inline
    ObjectRef<T>::operator T *& ()
    {
        return this->ptr_;
    }

    /// Function operators.
    template <typename T> inline
    typename ObjectRef<T>::_in_type
    ObjectRef<T>::operator -> (void) const
    {
        if (this->ptr_) {
            return this->ptr_;
        }
        throw DAF::INV_OBJREF();
    }

    template <typename T> inline
    typename ObjectRef<T>::_ref_type
    ObjectRef<T>::operator * (void) const
    {
        if (this->ptr_) {
            return *this->ptr_;
        }
        throw DAF::INV_OBJREF();
    }

    template <typename T> inline
    typename ObjectRef<T>::_in_type
    ObjectRef<T>::in(void) const
    {
        return this->ptr_;
    }

    template <typename T> inline
    typename ObjectRef<T>::_inout_type
    ObjectRef<T>::inout(void)
    {
        return this->ptr_;
    }

    template <typename T> inline
    typename ObjectRef<T>::_out_type
    ObjectRef<T>::out(void)
    {
        ObjectRefTraits<T>::release(this->ptr_); return this->ptr_ = ObjectRefTraits<T>::nil();
    }

    template <typename T> inline
    typename ObjectRef<T>::_retn_type
    ObjectRef<T>::_retn(void)
    {
        T *val = this->ptr_; this->ptr_ = ObjectRefTraits<T>::nil(); return val;
    }

        /// TAO extension.
    template <typename T> inline
    typename ObjectRef<T>::_in_type
    ObjectRef<T>::ptr(void) const // Not Part of the CORBA Standard
    {
        return this->ptr_;
    }

    template <typename T> inline
    ObjectRef<T> &
    ObjectRef<T>::operator = (T *p)
    {
        ObjectRefTraits<T>::release(this->ptr_); this->ptr_ = p; return *this;
    }

    template <typename T> inline
    ObjectRef<T> &
    ObjectRef<T>::operator = (const T &p)
    {
        if (this->ptr_ != &p)  {
            ObjectRefTraits<T>::release(this->ptr_); this->ptr_ = ObjectRefTraits<T>::duplicate(&p);
        }
        return *this;
    }

    template <typename T> inline
    ObjectRef<T> &
    ObjectRef<T>::operator = (const ObjectRef<T> &p)
    {
        if (this != &p)  {
            ObjectRefTraits<T>::release(this->ptr_); this->ptr_ = ObjectRefTraits<T>::duplicate(p.ptr_);
        }
        return *this;
    }

    template <typename T> inline
    void
    ObjectRef<T>::free(void)
    {
        ObjectRefTraits<T>::release(this->ptr_);
    }

    template <typename T> inline
    void
    ObjectRef<T>::reset(T *p)
    {
        ObjectRefTraits<T>::release(this->ptr_); this->ptr_ = p;
    }

    template <typename T> template <typename A>  // Support widening (C++11 Standard - Section 6.6.3)
    ObjectRef<T>::operator ObjectRef<A> () const
    {
        for (A *ap = ObjectRefTraits<A>::narrow(this->ptr_);ap;) {
            return ObjectRef<A>(ap);
        }
        throw DAF::INV_OBJREF();
    }

    /**************************************************************/

    template <typename T> inline
    ObjectRefOut<T>::ObjectRefOut(T *&p)
        : ptr_(p)
    {
        this->ptr_ = ObjectRefTraits<T>::nil();
    }

    template <typename T> inline
    ObjectRefOut<T>::ObjectRefOut(ObjectRef<T> &p)
        : ptr_(p.out())
    {
        release(this->ptr_); this->ptr_ = ObjectRefTraits<T>::nil();
    }

    template <typename T> inline
    ObjectRefOut<T>::ObjectRefOut(const ObjectRefOut<T> &p)
        : ptr_(p.ptr_)
    {
    }

    template <typename T> inline
    ObjectRefOut<T>::operator T *& ()
    {
        return this->ptr_;
    }

    template <typename T> inline
    T *&
    ObjectRefOut<T>::ptr(void)
    {
        return this->ptr_;
    }

    template <typename T> inline
    ObjectRefOut<T> &
    ObjectRefOut<T>::operator = (T *p)
    {
        this->ptr_ = p; return *this;
    }

    template <typename T> inline
    ObjectRefOut<T> &
    ObjectRefOut<T>::operator = (const ObjectRefOut<T> &p)
    {
        this->ptr_ = p.ptr_; return *this;
    }

    template <typename T> inline
    ObjectRefOut<T> &
    ObjectRefOut<T>::operator = (const ObjectRef<T> &p)
    {
        this->ptr_ = ObjectRefTraits<T>::duplicate(p.in()); return *this;
    }

    template <typename T> inline
    T *
    ObjectRefOut<T>::operator -> (void) const
    {
        if (this->ptr_) {
            return this->ptr_;
        }
        throw DAF::INV_OBJREF();
    }
} // namespace DAF

#endif  // DAF_OBJECTREF_T_CPP
