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
#ifndef DAF_SYNCHRONOUSCHANNEL_T_CPP
#define DAF_SYNCHRONOUSCHANNEL_T_CPP

#include "daf/SynchronousChannel_T.h"

#include <ace/OS_Errno.h>

namespace DAF
{
    template <typename T>
    SynchronousChannel<T>::SynchronousChannel(void) : Channel<T>()
        , itemAvailable_(0), unclaimedTakers_(0), itemTaken_(0), itemError_(false)
    {
         // Initially all synchronisation resources Locked
    }

    template <typename T> int
    SynchronousChannel<T>::insert(const T &t)
    {
        // putLock guard is ensuring embrace taker has cleared the item.
        ACE_GUARD_REACTION(ACE_SYNCH_MUTEX, put_mon, this->putLock_, DAF_THROW_EXCEPTION(ResourceExhaustionException));
        {
            ACE_GUARD_REACTION(ACE_SYNCH_MUTEX, item_mon, *this, DAF_THROW_EXCEPTION(ResourceExhaustionException));

            // Entering User code - try and catch
            // The putter needs to transfer error state to the taker.
            // Possible causes are thread termination, poor assignment operators etc
            // There is a race/risk here, that although itemError is used to transfer
            // error state, the taker could possibly be overtaken by a second putter
            // causing another error and loss of data. ATM, not coding for this case, but it can
            // possibly be reworked by using DCL on itemError around the putLock and itemLock
            // - See JAB for further information
            try {
                this->item_ = t; this->itemAvailable_.release();
            } catch(...) { // JB: Deliberate catch(...) - DON'T replace with DAF_CATCH_ALL
                this->itemError_ = true; throw; // set error condition
            }
        }
        this->itemTaken_.acquire(); return 0;  // Wait for taker to leave
    }

    template <typename T> T
    SynchronousChannel<T>::extract(void)
    {
        ACE_GUARD_REACTION(ACE_SYNCH_MUTEX, item_mon, *this, DAF_THROW_EXCEPTION(ResourceExhaustionException));
        T t(item_); this->item_ = T(); this->itemTaken_.release(); // Announce we have taken item
        if (this->itemError_) {
            this->itemError_ = false; DAF_THROW_EXCEPTION(DAF::InternalException);
        }
        return t;
    }

    template <typename T> T
    SynchronousChannel<T>::take(void)
    {
        this->unclaimedTakers_.release(); DAF_OS::thr_yield(); // Announce a taker

        try {

            if (this->itemAvailable_.acquire()) {
                DAF_THROW_EXCEPTION(DAF::InternalException);
            }

            return this->extract();

        } catch(...) { // JB: Deliberate catch(...) - DON'T replace with DAF_CATCH_ALL
            // Error condition - Thread Kill or similar - Attempt to reclaim the resource
            ACE_Errno_Guard g(errno); ACE_UNUSED_ARG(g);
            this->unclaimedTakers_.attempt(0);
            throw;
        }
    }

    template <typename T> int
    SynchronousChannel<T>::put(const T &t)
    {
        this->unclaimedTakers_.acquire(); return this->insert(t);
    }

    template <typename T> T
    SynchronousChannel<T>::poll(time_t msecs)
    {
        this->unclaimedTakers_.release(); DAF_OS::thr_yield();

        try {

            if (this->itemAvailable_.attempt(msecs)) switch (DAF_OS::last_error()) {
            case ETIME: DAF_THROW_EXCEPTION(DAF::TimeoutException); // Reverse the fact that we have been here and exit with error
            default:    DAF_THROW_EXCEPTION(DAF::InternalException);
            }

            return this->extract();

        } catch(...) { // JB: Deliberate catch(...) - DON'T replace with DAF_CATCH_ALL
            // Using an Errno guard to ensure the ETIME error
            // from itemAvailable_.attempt is reported and not
            // unclaimedTakers_.attempt.
            ACE_Errno_Guard g(errno); ACE_UNUSED_ARG(g);
            this->unclaimedTakers_.attempt(0);
            throw;
        }
    }

    template <typename T> int
    SynchronousChannel<T>::offer(const T &t, time_t msecs)
    {
        if (this->unclaimedTakers_.attempt(msecs) == 0) {
            return this->insert(t);
        }
        return -1;
    }

    template <typename T> int
    SynchronousChannel<T>::interrupt(void)
    {
        return this->itemAvailable_.interrupt() + this->unclaimedTakers_.interrupt() + this->itemTaken_.interrupt() ? -1 : 0;
    }

} // namespace DAF

#endif  // DAF_SYNCHRONOUSCHANNEL_T_CPP
