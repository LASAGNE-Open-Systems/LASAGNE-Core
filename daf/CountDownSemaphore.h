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
#ifndef DAF_COUNTDOWNSEMAPHORE_H
#define DAF_COUNTDOWNSEMAPHORE_H

/**
* ATTRIBUTION: Doug Lee Based On 'Concurrency Patterns in Java'
*/

#include "Monitor.h"

#include <ace/Min_Max.h>

namespace DAF
{
    /** @class CountDownSemaphore
     *
     * @brief A CountDownSemaphore can serve as a simple one-shot
     * barrier that is initialized with a given count value.
     *
     * Each release decrements the count.
     * All acquires block until the count reaches zero. Upon reaching
     * zero all current acquires are unblocked and all
     * subsequent acquires pass without blocking. This is a one-shot
     * phenomenon -- the count cannot be reset.
     * If you need a version that resets the count, consider
     * using a Barrier type.
     */
    class DAF_Export CountDownSemaphore : protected Monitor
    {
        using Monitor::wait; // Hide Wait

    public:

        /** Constructor @a count >= 0 */
        CountDownSemaphore(int count);

        /** Accessor to the current count value */
        int count(void) const;

        /**
        * If @a abstime == 0 then call acquire() directly for infinate time.
        * Otherwise, block the acquiring thread until the count is 0 or
        * until @a abstime times out, in which case -1 is returned
        * with @c errno == @c ETIME.  Note that @a abstime is assumed to
        * be in "absolute" rather than "relative" time.
        */
        int acquire(const ACE_Time_Value * abstime = 0);
        int acquire(const ACE_Time_Value & abstime);
        int acquire(time_t msecs);

        int attempt(time_t msecs); // Backwards Compatability

        /**
        * Release (decrement) the current count by @a count and broadcast 
        * to waiting acquirer's if 0. NOTE: underlying count will not be
        * allowed to go negative.
        */
        int release(int count = 1);

        /**
        * Allow public access to underlying Monitor capabilities
        */
        using Monitor::waiters;
        using Monitor::interrupt;
        using Monitor::interrupted;

    private:

        int count_; // The current counter >= 0 always
    };

    inline
    CountDownSemaphore::CountDownSemaphore(int count) : Monitor()
        , count_(ace_max(0, count))
    {
    }

    inline int
    CountDownSemaphore::acquire(const ACE_Time_Value & tv)
    {
        return this->acquire(&tv);
    }

    inline int
    CountDownSemaphore::acquire(time_t msecs)
    {
        return this->acquire(DAF_OS::gettimeofday(ace_max(msecs, time_t(0))));
    }

    inline int // Backwards Compatability
    CountDownSemaphore::attempt(time_t msecs)
    {
        return this->acquire(msecs);
    }

}  // namespace DAF

#endif // DAF_COUNTDOWNSEMAPHORE_H
