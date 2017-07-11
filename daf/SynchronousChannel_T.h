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
#ifndef DAF_SYNCHRONOUSCHANNEL_T_H
#define DAF_SYNCHRONOUSCHANNEL_T_H

#include "Channel_T.h"

#include "RefCount.h"
#include "Semaphore.h" // Not Used Yet, but dependants may require it

#include <list>

namespace DAF
{
    /** @class SynchronousChannel
    *@brief A rendezvous channel, similar to those used in CSP and Ada.
    *
    * Each put() must wait for a take() and vice versa.
    *
    * Synchronous channels are well suited for hand-off designs, in which an
    * object running in one thread must synch-up with an object running in
    * another thread in order to hand it some information, event, or task. This
    * implementation uses a WaiterPreferenceSemaphore to help avoid infinite
    * overtaking among multiple threads trying to perform put() or take().
    *
    * If you only need threads to synch up without exchanging information,
    * consider using a Barrier. If you need bi-directional exchanges, consider
    * using a Rendezvous.
    */

    template <typename T>
    class SynchronousChannel : public Channel<T>
    {
    public:

        typedef typename Monitor::_mutex_type   _mutex_type;

        using Channel<T>::interrupt;
        using Channel<T>::interrupted;

        enum {
            DEFAULT_CAPACITY_LIMIT  = size_t(100) // Currently Not Honoured - Maybe Later
        };

        SynchronousChannel(size_t capacity = DEFAULT_CAPACITY_LIMIT);  // Set Up initial State

        virtual ~SynchronousChannel(void);

        /**
        * Basic protocol is:
        * -# Wait until a taker arrives (via unclaimedTakers_ semaphore)
        * -# Assign item value using overloaded item assignment operator.
        */
        virtual int     put(const T &t, const ACE_Time_Value * abstime = 0);

        /**
        * Basic protocol is:
        * -# Announce that a taker has arrived (via unclaimedTakers_ semaphore),
        *    so that a put can proceed.
        * -# Wait until the item has a value.
        * -# Get the item value through item cast operator overload.
        */
        virtual T       take(const ACE_Time_Value * abstime = 0);

        /**
        * Return the number of items currently in the channel.
        * This value may change immediately upon return, and therefore
        * is only an instantanous value.
        */
        virtual size_t  capacity(void) const;

        /**
        * Return the number of items currently in the channel.
        * This value may change immediately upon return, and therefore
        * is only an instantanous value.
        */
        virtual size_t  size(void) const;

    private:

        enum {
            CANCELLED = -1, EMPTY = 0, FULL = 1
        };

        class SYNCHNode : virtual public Monitor, virtual public RefCount
        {
            T item_;

        public:

            DAF_DEFINE_REFCOUNTABLE( SYNCHNode );

            SYNCHNode(void);
            SYNCHNode(const T &t);

            int put_item(const T &t);
            int get_item(T &t);

            int state(void) const;

            int isFULL(void)        const { return this->state() == FULL; }
            int isEMPTY(void)       const { return this->state() == EMPTY; }
            int isCANCELLED(void)   const { return this->state() == CANCELLED; }

            int state(int state);

        private:

            int state_;
        };

        typedef std::list<typename SYNCHNode::_ref_type>    _waiter_list_type;

        struct WaiterQueue : _waiter_list_type
        {
            int deque(typename SYNCHNode::_out_type node);
            int enque(const typename SYNCHNode::_ref_type & node);

        } waitingProducers, waitingConsumers;
    };

}  // namespace DAF

#if defined (ACE_TEMPLATES_REQUIRE_SOURCE)
# include "SynchronousChannel_T.cpp"
#endif /* ACE_TEMPLATES_REQUIRE_SOURCE */

#if defined (ACE_TEMPLATES_REQUIRE_PRAGMA)
# pragma implementation ("SynchronousChannel_T.cpp")
#endif /* ACE_TEMPLATES_REQUIRE_PRAGMA */

#endif // DAF_SYNCHRONOUSCHANNEL_T_H
