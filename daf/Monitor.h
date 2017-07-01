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
#ifndef DAF_MONITOR_H
#define DAF_MONITOR_H

/**
* ATTRIBUTION: Doug Lee Based On 'Concurrency Patterns in Java'
*/

#include "SYNCHCondition_T.h"

namespace DAF
{
    /** @class Monitor
    *@brief Brief \todo{Fill this in}
    *
    * Details \todo{Detailed description}
    */
    class DAF_Export Monitor
    {
        mutable DAF_SYNCH_MUTEX     mutex_;
        mutable DAF_SYNCH_CONDITION cond_mutex_;

    public:

        typedef DAF_SYNCH_CONDITION::_mutex_type        _mutex_type;
        typedef DAF_SYNCH_CONDITION::_condition_type    _condition_type;

        /** \todo{Fill this in}   */
        Monitor(void) : cond_mutex_(mutex_) {}

        /** \todo{Fill this in}   */
        virtual ~Monitor(void)
        {
            this->interrupt();
        }

        /** \todo{Fill this in}   */
        operator _mutex_type & () const
        {
            return this->mutex_;
        }

        /// Get the number of waiters.
        int waiters(void) const
        {
            return this->cond_mutex_.waiters();
        }

        /// Wait indefinately until condition is signalled.
        int wait(const ACE_Time_Value * tv = 0) const
        {
            return this->cond_mutex_.wait(tv);
        }

        /// Wait for upto ACE_Time_Value as an absolute TOD deadline or until condition is signalled.
        int wait(const ACE_Time_Value & tv) const
        {
            return this->wait(&tv);
        }

        /// Wait for upto msec or until condition is signalled.
        int wait(time_t msec) const
        {
            return this->wait(DAF_OS::gettimeofday(msec));
        }

        /// Notify single waiter on condition queue to awake. This queue is OS dependant but *almost* always FIFO ordered
        int signal(void) const
        {
            return this->cond_mutex_.signal();
        }

        /// Notify all waiters on condition queue to awake. This queue is OS dependant but *almost* always these waiters will awake in a FIFO order
        int broadcast(void) const
        {
            return this->cond_mutex_.broadcast();
        }

        /// Support JAVA interface to notify single waiter on condition queue to awake.
        int notify(void) const
        {
            return this->signal();
        }

        /// Support JAVA interface to notify all waiters on condition queue to awake.
        int notifyAll(void) const
        {
            return this->broadcast();
        }

        /// Get the interrupted state.
        int interrupted(void) const
        {
            return this->cond_mutex_.interrupted();
        }

        /** Notify all waiters on condition queue to awake.
        * The underlying condition will also be marked as removed
        * and its OS backed handles will be closed.
        */
        int interrupt(void)
        {
            return this->cond_mutex_.interrupt();
        }

    private:

        ACE_UNIMPLEMENTED_FUNC(void operator = (const Monitor &))
        ACE_UNIMPLEMENTED_FUNC(Monitor(const Monitor &))
    };

}  // namespace DAF

#endif
