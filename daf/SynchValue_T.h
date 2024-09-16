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
#ifndef DAF_SYNCHVALUE_T_H
#define DAF_SYNCHVALUE_T_H

#include "Monitor.h"
#include "Semaphore.h"

#include <algorithm>

namespace DAF
{
     /** @class SynchValue
     * @brief Synchronize on a particular value.
     *
     * A waiting thread will request sync.waitValue(value) with a desired value.
     * A second setting thread will set the internal value sync.setValue().
     *
     * When a the requested value is found. the Waiting Thread will
     * exit and return.
     */
    template < typename T, typename F = std::equal_to<const T &> >
    class SynchValue_T : protected Monitor
    {
        using Monitor::wait;    // Hide Monitor::wait

    public:

        typedef T   _value_type;
        typedef F   _comparator_type;

        typedef typename Monitor::_mutex_type   _mutex_type;

        SynchValue_T(const T & value = T());

        virtual ~SynchValue_T(void);

        using Monitor::waiters;
        using Monitor::interrupt;
        using Monitor::interrupted;

        /// Return the current value
        T   getValue(void) const;

        /**
         * setValue
         *
         * Assign a value to the SyncValue. Upon  setting the value
         * notifyAll is used on all waiters. It will not exit until all
         * waiters have checked the current value.
         *
         * @return 0 upon success.
         *         -1 if other error (Use DAF_OS::last_error() to examine error).
         */
        int setValue(const T & value);

        /**
         * waitValue
         *
         * Wait upon the SyncValue for a particluar value to be set. This
         * will block the thread until the current value of the SynchValue
         * is equal to the input value. @see setValue().
         *
         * @arg value - The value to wait on.
         *
         * @return 0  upon completion when value has been set.
         * throws DAF::TimeoutException on timeout
         * otherwise -1 if other error (Use DAF_OS::last_error() to examine error).
         */
        int waitValue(const T & value, const ACE_Time_Value * abstime = 0) const;

        /** Wait on a Latched Value until abs time value tv */
        int waitValue(const T & value, const ACE_Time_Value & abstime) const;

        /** Wait on a Latched Value for upto msec */
        int waitValue(const T & value, time_t msec) const;

    protected:

        T   value_;

    private:

        mutable _mutex_type valueLock_;
        mutable class ValueSemaphore : public Semaphore
        {
        public:

            ValueSemaphore(int permits = 1);
            ~ValueSemaphore(void);

            int releaseWaiter(void);
            int acquireWaiter(void);

            int valueWaiters(void) const;

        private:

            class ValueWaiters : Monitor
            {
                int valueWaiters_;

            public:

                ValueWaiters(int valueWaiters = 0);
                ~ValueWaiters(void);

                int operator ++ (); // Prefix
                int operator -- (); // Prefix

                int valueWaiters(void) const;

            } valueWaiters_;

        } valueSemaphore_;

        class ValueSemaphoreWaiterGuard
        {
            ValueSemaphore & value_semaphore_;

        public:

            ValueSemaphoreWaiterGuard(ValueSemaphore & value_semaphore) : value_semaphore_(value_semaphore) {
                this->value_semaphore_.acquireWaiter();
            }
            ~ValueSemaphoreWaiterGuard(void) {
                this->value_semaphore_.releaseWaiter();
            }
        };
    };

}  // namespace DAF

#if defined (ACE_TEMPLATES_REQUIRE_SOURCE)
# include "SynchValue_T.cpp"
#endif /* ACE_TEMPLATES_REQUIRE_SOURCE */

#if defined (ACE_TEMPLATES_REQUIRE_PRAGMA)
# pragma implementation ("SynchValue_T.cpp")
#endif /* ACE_TEMPLATES_REQUIRE_PRAGMA */

#endif
