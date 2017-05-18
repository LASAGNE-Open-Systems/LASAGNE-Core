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
#ifndef DAF_REFCOUNTHANDLER_T_H
#define DAF_REFCOUNTHANDLER_T_H

#include "RefCount.h"


namespace DAF
{
    template <typename T>

    /** @class RefCountHandler_T
    *@brief Brief \todo{Fill this in}
    *
    * Details \todo{Detailed description}
    */
    class RefCountHandler_T : public DAF::RefCount
    {
    public:

        DAF_DEFINE_REFCOUNTABLE( RefCountHandler_T<T> );

        /** \todo{Fill this in} */
        typedef T           _handle_type;
        /** \todo{Fill this in} */
        typedef T const &   _handle_in_type;
        /** \todo{Fill this in} */
        typedef T &         _handle_inout_type;

        /** \todo{Fill this in} */
        RefCountHandler_T(const T &t = T());
        /** \todo{Fill this in} */
        virtual ~RefCountHandler_T(void);

        /** \todo{Fill this in} */
        _handle_in_type operator -> () const;

        /** \todo{Fill this in} */
        operator _handle_in_type () const;

        /** \todo{Fill this in} */
        _handle_type        handle(void) const;

        /** \todo{Fill this in} */
        _handle_in_type     handle_in(void) const;
        /** \todo{Fill this in} */
        _handle_inout_type  handle_inout(void);
        /** \todo{Fill this in} */
        _handle_inout_type  handle_out(void);
        /** \todo{Fill this in} */
        _handle_type        handle_retn(void);

        /** \todo{Fill this in} */
        _handle_in_type     handle_ref(void) const;

    protected:

        /** \todo{Fill this in} */
        virtual void _finalize(_handle_inout_type);

    private:

        T t_;
    };

    /** \todo{Fill this in} */
    template <typename T>
    inline
    RefCountHandler_T<T>::RefCountHandler_T(const T &t) : t_(t)
    {
    }

    /** \todo{Fill this in} */
    template <typename T>
    inline
    RefCountHandler_T<T>::~RefCountHandler_T(void)
    {
        this->_finalize(this->handle_inout());
    }

    /** \todo{Fill this in} */
    template <typename T>
    inline void
    RefCountHandler_T<T>::_finalize(T &t)
    {
        t = T();
    }

    /** \todo{Fill this in} */
    template <typename T>
    inline T const &
    RefCountHandler_T<T>::operator -> () const
    {
        return this->t_;
    }

    /** \todo{Fill this in} */
    template <typename T>
    inline
    RefCountHandler_T<T>::operator T const & () const
    {
        return this->t_;
    }

    /** \todo{Fill this in} */
    template <typename T>
    inline T
    RefCountHandler_T<T>::handle(void) const
    {
        return this->t_;
    }

    /** \todo{Fill this in} */
    template <typename T>
    inline T const &
    RefCountHandler_T<T>::handle_in(void) const
    {
        return this->t_;
    }

    /** \todo{Fill this in} */
    template <typename T>
    inline T &
    RefCountHandler_T<T>::handle_inout(void)
    {
        return this->t_;
    }

    /** \todo{Fill this in} */
    template <typename T>
    inline T &
    RefCountHandler_T<T>::handle_out(void)
    {
        this->_finalize(this->handle_inout()); return this->t_ = T();
    }

    /** \todo{Fill this in} */
    template <typename T>
    inline T
    RefCountHandler_T<T>::handle_retn(void)
    {
        T t = this->t_; this->t_ = T(); return t;
    }

    /** \todo{Fill this in} */
    template <typename T>
    inline T const &
    RefCountHandler_T<T>::handle_ref(void) const
    {
        return this->t_;
    }

} // namespace DAF

#endif // !DAF_REFCOUNTHANDLER_T_H
