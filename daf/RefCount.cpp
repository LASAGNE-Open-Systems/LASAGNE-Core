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
#define DAF_REFCOUNT_CPP

/**********************  RefCount *****************************
 *
 *(c)Copyright 2011,
 *   Defence Science and Technology Organisation,
 *   Department of Defence,
 *   Australia.
 *
 * All rights reserved.
 *
 * This is unpublished proprietary source code of DSTO.
 * The copyright notice above does not evidence any actual or
 * intended publication of such source code.
 *
 * The contents of this file must not be disclosed to third
 * parties, copied or duplicated in any form, in whole or in
 * part, without the express prior written permission of DSTO.
 *
 *
 * @file     RefCount.cpp
 * @author   Derek Dominish
 * @author   $LastChangedBy$
 * @date     1st September 2011
 * @version  $Revision$
 * @ingroup
 */

#include "RefCount.h"

namespace DAF
{
    RefCount::~RefCount(void)
    {
        switch (this->refCount()) {
        case 0  : break;  // Heap Allocated
        case 1  :
            if (DAF::debug()) {
                ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P | %t) WARNING: RefCount(0x%p)::~RefCount() [Refcount = %u]\n"),this,this->refCount()));
            } break;  // Stack Allocated -> Maybe Bad, but we don't know if we are on the stack?

        default : // Bad Bad Bad -> someone still has a reference to us, and *MAY* still use it...
            {
                ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P | %t) ERROR: RefCount(0x%p)::~RefCount() [Refcount = %u]\n"),this,this->refCount()));
            }
        }
    }

    /// Increment the reference count.
    size_t  RefCount::_add_ref(void)
    {
        if (this->refCount_ > 0) try {  // DCL
            ACE_Guard<ACE_SYNCH_MUTEX> guard(this->refLock_); ACE_UNUSED_ARG(guard);
            if (this->refCount_ > 0) {
                return ++this->refCount_;
            }
        } DAF_CATCH_ALL {}

        throw DAF::INV_OBJREF();
    }

    /// Decrement the reference count.
    size_t  RefCount::_remove_ref(void)
    {
        if (this->refCount_ > 0) try {  // DCL
            do  {
                {
                    ACE_Guard<ACE_SYNCH_MUTEX> guard(this->refLock_); ACE_UNUSED_ARG(guard);

                    if (this->refCount_ > 0) {
                        if (--this->refCount_ > 0) {
                            return this->refCount_;
                        }
                    } else break;
                }

                this->__remove(); return 0;
            } while (false);
        } DAF_CATCH_ALL {}

        throw DAF::INV_OBJREF();
    }
}  // namespace DAF
