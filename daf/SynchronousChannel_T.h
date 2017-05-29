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

/**
* @file     SynchronousChannel_T.h
* @author   Derek Dominish
* @author   $LastChangedBy$
* @date     1st September 2011
* @version  $Revision$
* @ingroup
* @namespace DAF
*/

#include "Channel_T.h"
#include "Semaphore.h"

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
    class SynchronousChannel : public DAF::Channel<T>
    {
        DAF_SYNCH_MUTEX putLock_;
        DAF::Semaphore  itemAvailable_, unclaimedTakers_, itemTaken_;
        T               item_;
        volatile bool   itemError_;

        // Assignment protocol is:
        // 1. Wait until no other puts are active (via putLock_)
        // 2. Insert the item value, and signal item that it is ready
        //       (via underlying WaiterPreferenceSemaphore).
        // 3. Wait for item to be taken (via itemTaken_ semaphore).
        // 4. Allow another put to insert item (by releasing putLock_).
        virtual int insert(const T &t);

        // Item value overload protocol is:
        // 1. This item has previously been acquired by caller
        // 2. Insert the item value, and signal item that it is ready
        // 3. Return item value.
        virtual T   extract(void);

    public:

        SynchronousChannel(void);  // Set Up initial State

        /**
        * Basic protocol is:
        * -# Wait until a taker arrives (via unclaimedTakers_ semaphore)
        * -# Assign item value using overloaded item assignment operator.
        */
        virtual int put(const T &t);

        /**
        * Protocol is the same as put() except:
        * Backouts due to timeouts are allowed only during the wait
        * for takers. Upon claiming a taker, puts are forced to proceed.
        */
        virtual int offer(const T &t, time_t msecs = 0);

        /**
        * Basic protocol is:
        * -# Announce that a taker has arrived (via unclaimedTakers_ semaphore),
        *    so that a put can proceed.
        * -# Wait until the item has a value.
        * -# Get the item value through item cast operator overload.
        */
        virtual T   take(void);

        /**
        * Protocol is the same as take() except:
        *   Backouts due to timeouts are allowed
        *   only during the wait for the item to be available. However,
        *   even here, if the put of an item we should get has already
        *   begun, we ignore the timeout and proceed.
        */
        virtual T   poll(time_t msecs = 0);

        /**
        * Return the number of items currently in the channel.
        * This value may change immediately upon return, and therefore
        * is only an instantanous value.
        */
        virtual size_t  capacity(void) const
        {
            return size_t(1);
        }

        /**
        * Return the number of items currently in the channel.
        * This value may change immediately upon return, and therefore
        * is only an instantanous value.
        */
        virtual size_t  size(void) const
        {
            return size_t(this->itemAvailable_.permits());
        }

        /** \todo{Fill this in} */
        virtual int     interrupt(void);
  };
}  // namespace DAF

#if defined (ACE_TEMPLATES_REQUIRE_SOURCE)
# include "SynchronousChannel_T.cpp"
#endif /* ACE_TEMPLATES_REQUIRE_SOURCE */

#if defined (ACE_TEMPLATES_REQUIRE_PRAGMA)
# pragma implementation ("SynchronousChannel_T.cpp")
#endif /* ACE_TEMPLATES_REQUIRE_PRAGMA */

#endif // DAF_SYNCHRONOUSCHANNEL_T_H
