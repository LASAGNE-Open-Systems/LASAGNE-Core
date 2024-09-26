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
#ifndef DAF_RENDEZVOUS_T_H
#define DAF_RENDEZVOUS_T_H

#include "Monitor.h"
#include "Semaphore.h"

#include <ace/Min_Max.h>

#include <vector>
#include <algorithm>

namespace DAF
{
  /*
   * Barriers serve as synchronous points for groups of threads that
   * must occasionally wait for each other.  Barriers may support
   * any of several methods that accomplish this synchronisation.
   */

  /** @class RendezvousCommand
  * @brief A common rotator rendezvous function.
  *
  * Rotates the array so that each thread returns an item presented by
  * some other thread (or itself, if parties is 1).
  */
    template <typename T>
    struct RendezvousCommand : std::unary_function<std::vector<T>,void> {
        typedef T               _value_type;
        typedef void            _result_type;
        typedef std::vector<T>  _argument_type;
        virtual ~RendezvousCommand(void) {}
        virtual typename RendezvousCommand<T>::_result_type operator () (typename RendezvousCommand<T>::_argument_type &);
    };

    /** @class Rendezvous
    *@brief Specialised Barrier class
    *
    * A rendezvous is a barrier that:
    * <ul>
    *   <li> Unlike a CyclicBarrier, is not restricted to use
    *     with fixed-sized groups of threads.
    *     Any number of threads can attempt to enter a rendezvous,
    *     but only the predetermined number of parties enter
    *     and later become released from the rendezvous at any give time.
    *   <li> Enables each participating thread to exchange information
    *     with others at the rendezvous point. Each entering thread
    *     presents some object on entry to the rendezvous, and
    *     returns some object on release. The object returned is
    *     the result of a RendezvousFunction that is run once per
    *     rendezvous, (it is run by the last-entering thread). By
    *     default, the function applied is a rotation, so each
    *     thread returns the object given by the next (modulo parties)
    *     entering thread. This default function faciliates simple
    *     application of a common use of rendezvous, as exchangers.
    * </ul>
    * <p>
    * Rendezvous use an all-or-none breakage model
    * for failed synchronization attempts: If threads
    * leave a rendezvous point prematurely because of timeout
    * or interruption, others will also leave abnormally
    * (via BrokenBarrierException), until
    * the rendezvous is <code>restart</code>ed. This is usually
    * the simplest and best strategy for sharing knowledge
    * about failures among cooperating threads in the most
    * common usages contexts of Rendezvous.
    * <p>
    * While any positive number (including 1) of parties can
    * be handled, the most common case is to have two parties.
    * --> Doug Lee
    */
    template < typename T, typename F = RendezvousCommand<T> >
    class Rendezvous : protected Monitor
    {
        using Monitor::wait; // Hide Wait

    public:

        typedef T   _value_type;
        typedef F   _function_type;

        typedef typename _function_type::_argument_type _slots_type;
        typedef typename Monitor::_mutex_type           _mutex_type;

        /**
        * Create a Barrier for the indicated number of parties,
        */
        Rendezvous(int parties, typename Rendezvous<T,F>::_function_type & function = F());

        /** \todo{Fill this in} */
        virtual ~Rendezvous(void);

        /** \todo{Fill this in} */
        bool    broken(void) const;

        /** \todo{Fill this in} */
        int     parties(void) const;

        /**
        * Wait msecs to complete a rendezvous.
        * @param t the item to present at rendezvous point.
        * By default, this item is exchanged with another.
        * @param msec The maximum time to wait.
        * @return an item given by some thread, and/or processed
        * by the rendezvousFunction.
        * @exception BrokenBarrierException
        * if any other thread
        * in any previous or current barrier
        * since either creation or the last <code>restart</code>
        * operation left the barrier
        * prematurely due to interruption or time-out. (If so,
        * the <code>broken</code> status is also set.)
        * Also returns as
        * broken if the RendezvousFunction encountered a run-time exception.
        * @exception TimeoutException if this thread timed out waiting for
        * the exchange. If the timeout occured while already in the
        * exchange, <code>broken</code> status is also set.
        */
        T   rendezvous(const T & t, const ACE_Time_Value * abstime = 0);
        T   rendezvous(const T & t, const ACE_Time_Value & abstime);
        T   rendezvous(const T & t, time_t msecs);

        /**
        * Wait For the Rendezvous to complete its transaction after an exchange
        * and all the treads have exited.
        * @return indicates <code>true</code> if the rendezvous was reset
        * ie all threads exited, <code>false</code> if an error occured.
        * Upon a Timeout occuring this will set the state to broken
        * and notify all existing waiters.
        */
        int waitReset(const ACE_Time_Value * abstime = 0);
        int waitReset(const ACE_Time_Value & abstime);
        int waitReset(time_t msecs);

        /** Interrupt the monitor and the entry semaphore */
        int     interrupt(void);

        using Monitor::waiters;
        using Monitor::interrupted;

    private:

        int resetRendezvous(void); // Internal method called with monitor locked

    private:

        Semaphore           rendezvousSemaphore_;

        _slots_type         rendezvousSlots_;
        _function_type &    rendezvousFunction_;

    private:

        int     parties_;
        int     resets_;
        int     count_;
        bool    broken_;
        bool    triggered_;
    };

    template <typename T, typename F>
    inline T
    Rendezvous<T,F>::rendezvous(const T & t, const ACE_Time_Value & abstime)
    {
        return this->rendezvous(t, &abstime);
    }

    template <typename T, typename F>
    inline T
    Rendezvous<T,F>::rendezvous(const T & t, time_t msecs)
    {
        return this->rendezvous(t, DAF_OS::gettimeofday(ace_max(msecs, time_t(0))));
    }

    template <typename T, typename F>
    inline int
    Rendezvous<T, F>::waitReset(const ACE_Time_Value & abstime)
    {
        return this->waitReset(&abstime);
    }

    template <typename T, typename F>
    inline int
    Rendezvous<T, F>::waitReset(time_t msecs)
    {
        return this->waitReset(DAF_OS::gettimeofday(ace_max(msecs, time_t(0))));
    }

} // namespace DAF

#if defined (ACE_TEMPLATES_REQUIRE_SOURCE)
# include "Rendezvous_T.cpp"
#endif /* ACE_TEMPLATES_REQUIRE_SOURCE */

#if defined (ACE_TEMPLATES_REQUIRE_PRAGMA)
# pragma implementation ("Rendezvous_T.cpp")
#endif /* ACE_TEMPLATES_REQUIRE_PRAGMA */

#endif  // DAF_RENDEZVOUS_T_H
