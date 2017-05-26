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
#ifndef DAF_REFCOUNT_H
#define DAF_REFCOUNT_H

#include "DAF.h"
#include "ObjectRef_T.h"

#include <ace/Intrusive_Auto_Ptr.h>

#if !defined(DAF_DEFINE_REFCOUNTABLE)
# define DAF_DEFINE_REFCOUNTABLE( TYP ) \
    friend class DAF::ObjectRefTraits< TYP >; \
    static inline TYP *  _nil(void) \
        { return DAF::ObjectRefTraits< TYP >::nil(); } \
    static inline TYP *  _duplicate(const TYP *p) \
        { return DAF::ObjectRefTraits< TYP >::duplicate(p); } \
    template <typename A> \
    static inline TYP *  _narrow(const A * p) \
        { return DAF::ObjectRefTraits< TYP >::narrow(p); } \
    static inline void intrusive_add_ref(TYP *p) \
        { DAF::ObjectRefTraits< TYP >::duplicate(p); } \
    static inline void intrusive_remove_ref(TYP *p) \
        { DAF::ObjectRefTraits< TYP >::release(p); } \
    typedef TYP *                       _ptr_type;  \
    typedef DAF::ObjectRef< TYP >       _ref_type;  \
    typedef DAF::ObjectRefOut< TYP >    _out_type   /* No closing semi-colon to force the user to do it */
#endif

#if !defined(DAF_DECLARE_REFCOUNTABLE)
# define DAF_DECLARE_REFCOUNTABLE( TYP )                    \
  typedef TYP *                             TYP ## _ptr;    \
  typedef class DAF::ObjectRef< TYP >       TYP ## _ref;    \
  typedef class DAF::ObjectRefOut< TYP >    TYP ## _out     /* No closing semi-colon to force the user to do it */
#endif

namespace DAF
{
    /**
    * \class RefCount
    * \brief Intrusive Reference Counting base object.
    *
    * This provides a similar concept to CORBA::Object in being the base object
    * that provides intrinsic memory management via reference counting.
    *
    * To make good use of this in code the developer should make use of the macros
    *
    * DAF_DEFINE_REFCOUNTABLE is used to define a number of metatypes that are
    * used in templated infrastructure.
    *
    * DAF_DECLARE_REFCOUNTABLE is used to define helper types for ease of use.
    * It must be instantiated outside the class definition.
    *
    * For example:
    *
    *        struct ExampleStruct : RefCount
    *        {
    *           // Define Metatypes
    *           DAF_DEFINE_REFCOUNTABLE(ExampleStruct);
    *        };
    *        DAF_DECLARE_REFCOUNTABLE(ExampleStruct);

    *        // At this point ExampleStruct_ref, ExampleStruct_ptr, ExampleStruct_out
    *        // are defined. Developer now works with refs.
    *        {
    *          ExampleStruct_ref myEx = new ExampleStruct;
    *        } // myEx doesn't leak memory.
    *
    * ATTRIBUTION: Based on the CORBA::Object specification and TAO implementation of proxy's
    */
    class DAF_Export RefCount : ACE_Copy_Disabled
    {
        volatile size_t refCount_;

    public:


        DAF_DEFINE_REFCOUNTABLE( RefCount );


        size_t  refCount(void) const
        {
            return this->refCount_;
        }

        /// Increment the reference count.
        size_t  _add_ref(void);

        /// Decrement the reference count.
        size_t  _remove_ref(void);

    protected:

        /// DEFAULT: Same as CORBA refcounting - start at 1
        RefCount(size_t refCount = 1) : refCount_(refCount)
        {
        }

        virtual ~RefCount (void);

        /// Called by the final release to remove this item.
        virtual void  __remove (void)
        {
            delete this;
        }

    protected:

        ACE_SYNCH_MUTEX refLock_;  // Allow Protected Access
    };

    DAF_DECLARE_REFCOUNTABLE( RefCount );
} // namespace DAF

#endif  // DAF_REFCOUNT_H
