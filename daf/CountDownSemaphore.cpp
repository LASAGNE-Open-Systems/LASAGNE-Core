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

#include "Exception.h"

namespace DAF
{
    CountDownSemaphore::CountDownSemaphore(int count) : Monitor()
        , count_(ace_max(0,count))
    {
    }

    CountDownSemaphore::~CountDownSemaphore(void)
    {
        this->interrupt();
    }

    int
    CountDownSemaphore::count(void) const
    {
        return this->count_;
    }

    int
    CountDownSemaphore::acquire(const ACE_Time_Value * abstime)
    {
        ACE_GUARD_REACTION(_mutex_type, guard, *this, DAF_THROW_EXCEPTION(LockFailureException));

        while (this->interrupted() || this->count() > 0) {
            if (this->interrupted() ? DAF_OS::last_error(EINTR) : this->wait(abstime)) {
                switch (DAF_OS::last_error()) {
                case EINTR: DAF_THROW_EXCEPTION(InterruptedException);
                case ETIME:
                    if (0 >= this->count()) {
                        continue; // Ensure we check interrupted before exit
                    }

                    // Fall through to report errno

                default: return -1; // Return with errno.
                }
            }
        }

        return 0;
    }

    int
    CountDownSemaphore::release(int count)
    {
        ACE_GUARD_REACTION(_mutex_type, guard, *this, DAF_THROW_EXCEPTION(LockFailureException));

        if (this->count() > 0) {
            while (count-- > 0) {
                if (--this->count_ == 0) {
                    break;
                }
            }
        }

        return this->broadcast();
    }

}  // namespace DAF
