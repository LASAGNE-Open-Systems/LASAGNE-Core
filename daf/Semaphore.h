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
#ifndef DAF_SEMAPHORE_H
#define DAF_SEMAPHORE_H

/**
* ATTRIBUTION: Doug Lee Based On 'Concurrency Patterns in Java'
*/

#include "Monitor.h"

namespace DAF
{
    /** @class Semaphore (WaiterPreferenceSemaphore)
    * @brief An implementation of counting Semaphores that
    * enforces enough fairness for applications that
    * need to avoid indefinite overtaking without
    * necessarily requiring FIFO ordered access.
    *
    * Empirically, very little is paid for this property
    * unless there is a lot of contention among threads
    * or very unfair OS scheduling.
    * The acquire method waits even if there are permits
    * available but have not yet been claimed by threads that have
    * been notified but not yet resumed. This makes the semaphore
    * almost as fair as the underlying primitives allow.
    * So, if synch lock entry and notify are both fair
    * so is the semaphore -- almost:  Rewaits stemming
    * from timeouts in attempt, along with potentials for
    * interrupted threads to be notified can compromise fairness,
    * possibly allowing later-arriving threads to pass before
    * later arriving ones. However, in no case can a newly
    * entering thread obtain a permit if there are still others waiting.
    * Also, signalling order need not coincide with
    * resumption order. Later-arriving threads might get permits
    * and continue before other resumable threads are actually resumed.
    * However, all of these potential fairness breaches are
    * very rare in practice....
    * --> Doug Lee
    */

    class DAF_Export Semaphore : protected Monitor
    {
        using Monitor::wait; // Hide wait.

    public:

        typedef typename Monitor::_mutex_type _mutex_type;

        /** Construct with initial permit count */
        Semaphore(int permits = 1); // Set default == 1 (Unlocked)

        /** Access to current permit count */
        int permits(void) const;

        /**
        * If @a abstime == 0 the call acquire() directly.  Otherwise, Block the
        * thread until we acquire a permit or until @a abstime times out, in
        * which case -1 is returned with @c errno == @c ETIME.  Note that
        * @a abstime is assumed to be in "absolute" rather than "relative" time.
        */
        int acquire(const ACE_Time_Value * abstime = 0);
        int acquire(const ACE_Time_Value & abstime);

        /** Same as acquire, however a conversion of @a msecs to absolute time.
        * A @a msecs <= 0 will result in the current absolute time and if 
        * a permit is not imedaitely available, a -1 is returned with
        * @c errno == @c ETIME
        * @deprecated
        */
        int attempt(time_t msecs);

        /** Release a single permit. NOTE: permits can legally go negative! */
        int release(void);

        /** Release multiple @a n permits. NOTE: permits can legally go negative! */
        int release(int n);

        using Monitor::waiters;
        using Monitor::interrupt;
        using Monitor::interrupted;

    private:

        int permits_;
    };


    /*
     * Documented within Doug Lee's book on Concurrency in Java
     * @deprecated
     */

    typedef class DAF::Semaphore    WaiterPreferenceSemaphore;


    inline
    Semaphore::Semaphore(int permits) : permits_(permits)
    {
    }

    inline
    int
    Semaphore::acquire(const ACE_Time_Value & tv)
    {
        return this->acquire(&tv);
    }

    inline
    int
    Semaphore::attempt(time_t msecs)
    {
        return this->acquire(DAF_OS::gettimeofday(ace_max(msecs, time_t(0))));
    }

} // namespace DAF

#endif // DAF_SEMAPHORE_H
