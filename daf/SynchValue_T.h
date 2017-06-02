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

/**
* @file     SynchValue_T.h
* @author   Derek Dominish
* @author   $LastChangedBy$
* @date     1st September 2011
* @version  $Revision$
* @ingroup
* @namespace DAF
*/

#include "Monitor.h"
#include "Semaphore.h"

namespace DAF
{
     /** @class SynchWaiterSemaphore
     * @brief Extension of DAF::Semaphore to allow a direct acquisition of a number
     * of permits.
     *
     * Details \todo{Detailed description}
     */
    class SynchWaiterSemaphore : public DAF::Semaphore
    {
    public:
        SynchWaiterSemaphore(int permits = 0) : DAF::Semaphore(permits)
        {}

        /// Acquire n times.
        virtual int acquire(int permits)
        {
            while(permits-- >= 0 ) {
                Semaphore::acquire();
            }
            return 0;
        }
    };

     /** @class SynchValue
     * @brief Synchronize on a particular value.
     *
     * A waiting thread will request sync.waitValue(value) with a desired value.
     * A second setting thread will set the internal value sync.setValue().
     *
     * When a the requested value is found. the Waiting Thread will
     * exit and return.
     */
    template <typename T = bool>
    class SynchValue : protected DAF::Monitor
    {

        /// current value held.
        volatile T value_;

        /// flag to indicate Object is destroying.
        volatile bool shutdown_;

        /// flag for number of waiters
        volatile size_t   waiters_;

        /// Setter semaphore
        SynchWaiterSemaphore    itemTaken_;
        DAF::Semaphore          setterGate_;

    public:

        /** \todo{Fill this in} */
        typedef T _value_type;


        /**
         *  \todo{Fill this in}
         */
        SynchValue(const T &value = T());

        /// Upon destruction this value will notifyAll waiters and will cause them to throw IllegalThreadStateException
        virtual ~SynchValue();

        /**
         * getValue
         *
         * Get the current value of the SynchValue.
         * @return current value of the SynchValue.
         */
        virtual T   getValue(void) const;

        /**
         * setValue
         *
         * Assign a value to the SyncValue. Upon  setting the value
         * notifyAll is used on all waiters. It will not exit until all
         * waiters have checked the current value.
         *
         * @return 0 upon success. -1 upon failure to acquire guard.
         */
        virtual int setValue(const T &value);

        /**
         * waitValue
         *
         * Wait upon the SyncValue for a particluar value to be set. This
         * will block the thread until the current value of the SynchValue
         * is equal to the input value. @see setValue().
         *
         * @arg value - The value to wait on.
         *
         * @return 0 upon completion when value has been set.
         *         -1 if the guard could not be acquired.
         *
         * @throw IllegalThreadStateException upon the SynchValue being destroyed while waiting
         */
        virtual int waitValue(const T &value);

        virtual int waitValue(const T &value, time_t msec);

    };
}  // namespace DAF

#if defined (ACE_TEMPLATES_REQUIRE_SOURCE)
# include "SynchValue_T.cpp"
#endif /* ACE_TEMPLATES_REQUIRE_SOURCE */

#if defined (ACE_TEMPLATES_REQUIRE_PRAGMA)
# pragma implementation ("SynchValue_T.cpp")
#endif /* ACE_TEMPLATES_REQUIRE_PRAGMA */

#endif
