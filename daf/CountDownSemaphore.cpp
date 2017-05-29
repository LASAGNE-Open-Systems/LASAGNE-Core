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
#define DAF_COUNTDOWNSEMAPHORE_CPP

/******************  CountDownSemaphore ************************
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
* @file     CountDownSemaphore.cpp
* @author   Derek Dominish
* @author   $LastChangedBy$
* @date     1st September 2011
* @version  $Revision$
* @ingroup
*
* A CountDownSemaphore can serve as a simple one-shot
* barrier that is initialized with a given count value.
*
* Each release decrements the count.
* All acquires block until the count reaches zero. Upon reaching
* zero all current acquires are unblocked and all
* subsequent acquires pass without blocking. This is a one-shot
* phenomenon -- the count cannot be reset.
* If you need a version that resets the count, consider
* using a CyclicBarrier.
***********************************************************/

#include "CountDownSemaphore.h"

namespace DAF
{
    CountDownSemaphore::CountDownSemaphore(int count)
        : count_(count)
    {
    }

    int CountDownSemaphore::acquire(void)
    {
        ACE_GUARD_RETURN( ACE_SYNCH_MUTEX, ace_mon, *this, -1 );
        while (this->count_ > 0) {
            this->wait();
        }
        return 0;
    }

    int CountDownSemaphore::release(void)
    {
        ACE_GUARD_RETURN( ACE_SYNCH_MUTEX, ace_mon, *this, -1 );
        if (this->count_) {
            if (--this->count_ == 0) {
                return this->notifyAll(); return 0;
            }
        }
        return -1;
    }

    int CountDownSemaphore::release(int n)
    {
        ACE_GUARD_RETURN( ACE_SYNCH_MUTEX, ace_mon, *this, -1 );
        if (this->count_) while (n-- > 0) {
            if (--this->count_ == 0) {
                this->notifyAll(); return (n ? -1 : 0);
            }
        }
        return -1;
    }

    int CountDownSemaphore::attempt(time_t msecs)
    {
        ACE_GUARD_RETURN( ACE_SYNCH_MUTEX, ace_mon, *this, -1 );

        if (this->count_ == 0) {
            return 0;
        } else if (msecs > 0) {

            const ACE_Time_Value end_time(DAF_OS::gettimeofday(msecs));

            do {
                this->wait(end_time);
                if (this->count_ == 0) {
                    return 0;
                }
            } while (end_time >= DAF_OS::gettimeofday());
        }
        return -1;
    }
}  // namespace DAF
