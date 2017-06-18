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
*
* @file     Semaphore.h
* @author   Derek Dominish
* @author   $LastChangedBy$
* @date     1st September 2011
* @version  $Revision$
* @ingroup
* @namespace DAF
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

    typedef class DAF_Export Semaphore : protected DAF::Monitor
    {
        volatile int permits_;

    public:

        /** \todo{Fill this in} */
        Semaphore(int permits = 1); // Set default as unlocked

        using DAF::Monitor::waiters;
        using DAF::Monitor::interrupt;
        using DAF::Monitor::interrupted;

        /** \todo{Fill this in} */
        int permits(void) const
        {
            return this->permits_;
        }

        /** \todo{Fill this in} */
        virtual int acquire(void);
        /** \todo{Fill this in} */
        virtual int attempt(time_t msecs);
        /** \todo{Fill this in} */
        virtual int release(void);
        /** \todo{Fill this in} */
        virtual int release(int n);

    } WaiterPreferenceSemaphore;

} // namespace DAF

#endif // DAF_SEMAPHORE_H
