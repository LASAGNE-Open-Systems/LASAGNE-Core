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
#ifndef DAF_OBJECTREF_T_H
#define DAF_OBJECTREF_T_H

/**
* ATTRIBUTION: Based on the CORBA var specification and TAO implementation of proxy's
*
* @file     ObjectRef_T.h
* @author   Derek Dominish
* @author   $LastChangedBy$
* @date     21st December 2011
* @version  $Revision$
* @ingroup
* @namespace DAF
*/

#include "DAF.h"

namespace DAF
{
    /** @struct INV_OBJREF
    *@brief Brief \todo{Fill this in}
    *
    * Details \todo{Detailed description}
    */
    struct INV_OBJREF : ObjectNotExistException {}; // SAME AS C++11 Standard

    template <typename T> class ObjectRef;      // forward declaration
    template <typename T> class ObjectRefOut;   // forward declaration

    /** @class ObjectRefTraits
    *@brief Brief \todo{Fill this in}
    *
    * Details \todo{Detailed description}
    */
    template <typename T> class ObjectRefTraits
    {
    public:

        /** \todo{Fill this in} */
        typedef T   _obj_type;
        /** \todo{Fill this in} */
        typedef T * _obj_ptr_type;

        /** \todo{Fill this in} */
        static T *  nil(void)
        {
            return static_cast<T*>(0);
        }

        /** \todo{Fill this in} */
        static void release(T *&);
        /** \todo{Fill this in} */
        static T *  duplicate(const T *);
        /** \todo{Fill this in} */
        template <typename A>
        static T *  narrow(const A *);
    };

    /** @class ObjectRef
    *@brief Brief \todo{Fill this in}
    *
    * Details \todo{Detailed description}
    */
    template <typename T> class ObjectRef
    {
        T * ptr_;

    public:
        /** \todo{Fill this in} */
        typedef T                       _obj_type;
        /** \todo{Fill this in} */
        typedef T *                     _obj_ptr_type;

        /** \todo{Fill this in} */
        typedef ObjectRef<T>            _obj_ref_type;
        /** \todo{Fill this in} */
        typedef ObjectRefOut<T>         _obj_out_type;
        /** \todo{Fill this in} */
        typedef ObjectRefTraits<T>      _obj_traits_type;

        /** \todo{Fill this in} */
        typedef T *     _in_type;
        /** \todo{Fill this in} */
        typedef T &     _ref_type;
        /** \todo{Fill this in} */
        typedef T *&    _inout_type;
        /** \todo{Fill this in} */
        typedef T *&    _out_type;
        /** \todo{Fill this in} */
        typedef T *     _retn_type;

        /** \todo{Fill this in} */
        ObjectRef(T * = 0);
        /** \todo{Fill this in} */
        ObjectRef(const T &);
        /** \todo{Fill this in} */
        ObjectRef(const ObjectRef<T> &);

        /** \todo{Fill this in} */
        ~ObjectRef(void);

        /** \todo{Fill this in} */
        ObjectRef<T> &operator = (T *);
        /** \todo{Fill this in} */
        ObjectRef<T> &operator = (const T &);
        /** \todo{Fill this in} */
        ObjectRef<T> &operator = (const ObjectRef<T> &);

        /** \todo{Fill this in} */
        _in_type    operator -> (void) const;
        /** \todo{Fill this in} */
        _ref_type   operator *  (void) const; // Not Part of the CORBA Standard

        /** \todo{Fill this in} */
        template <typename A>  // Support widening (C++11 Standard - Section 6.6.3)
            operator ObjectRef<A> () const;

        /// Cast operators.
        /** \todo{Fill this in} */
        operator T * const & () const;
        /** \todo{Fill this in} */
        operator T *& ();

        /** \todo{Fill this in} */
        _in_type    in(void) const;
        /** \todo{Fill this in} */
        _inout_type inout(void);
        /** \todo{Fill this in} */
        _out_type   out(void);
        /** \todo{Fill this in} */
        _retn_type  _retn(void);

        /// TAO extension.
        /** \todo{Fill this in} */
        _in_type    ptr(void) const; // Not Part of the CORBA Standard

    protected:

        /// TAO extensions.
        /** \todo{Fill this in} */
        void free(void);
        /** \todo{Fill this in} */
        void reset(T * = 0);

    private:

        // Prohibit Heap allocation of proxy
        static void * operator new (size_t n)       { return ::operator new (n);    }
        static void * operator new [] (size_t n)    { return ::operator new [] (n); }
    };

    /**
     * @class ObjectRefOut
     *
     * @brief Parametrized implementation of _out class for implementation references.
     *
     * Details \todo{Detailed description}
     */
    template <typename T> class ObjectRefOut
    {
        T *& ptr_;

    public:

        /** \todo{Fill this in} */
        typedef T                       _obj_type;
        /** \todo{Fill this in} */
        typedef T *                     _obj_ptr_type;

        /** \todo{Fill this in} */
        typedef ObjectRef<T>            _obj_ref_type;
        /** \todo{Fill this in} */
        typedef ObjectRefOut<T>         _obj_out_type;
        /** \todo{Fill this in} */
        typedef ObjectRefTraits<T>      _obj_traits_type;

        /** \todo{Fill this in} */
        ObjectRefOut(T *&p);
        /** \todo{Fill this in} */
        ObjectRefOut(ObjectRef<T> &p);
        /** \todo{Fill this in} */
        ObjectRefOut(const ObjectRefOut<T> &p);

        /** \todo{Fill this in} */
        ObjectRefOut<T> &operator = (T *p);
        /** \todo{Fill this in} */
        ObjectRefOut<T> &operator = (const ObjectRefOut<T> &p);
        /** \todo{Fill this in} */
        ObjectRefOut<T> &operator = (const ObjectRef<T> &p);

        /** \todo{Fill this in} */
        operator T *& ();

        /** \todo{Fill this in} */
        T *& ptr(void);

        /** \todo{Fill this in} */
        T *  operator -> (void) const;

    private:

        // Prohibit Heap allocation of proxy
        static void * operator new (size_t n)       { return ::operator new (n);    }
        static void * operator new [] (size_t n)    { return ::operator new [] (n); }
    };
} // namespace DAF

#if defined (ACE_TEMPLATES_REQUIRE_SOURCE)
# include "ObjectRef_T.cpp"
#endif /* ACE_TEMPLATES_REQUIRE_SOURCE */

#if defined (ACE_TEMPLATES_REQUIRE_PRAGMA)
# pragma implementation ("ObjectRef_T.cpp")
#endif /* ACE_TEMPLATES_REQUIRE_PRAGMA */

#endif /* DAF_OBJECTREF_T_H */
