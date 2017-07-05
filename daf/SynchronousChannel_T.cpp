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

#include "SynchronousChannel_T.h"

#include <ace/OS_Errno.h>

namespace DAF
{
    template <typename T> inline
    SynchronousChannel<T>::SynchronousChannel(size_t capacity) : Channel<T>()
    {
        ACE_UNUSED_ARG(capacity);  // Maybe used in later version to limit parallism
    }

    template <typename T> inline
    SynchronousChannel<T>::~SynchronousChannel(void)
    {
        this->interrupt();
    }

    template <typename T>
    int
    SynchronousChannel<T>::put(const T & t, const ACE_Time_Value * abstime)
    {
        // Entirely symmetric to take()

        const ACE_thread_t thr_id(DAF_OS::thr_self()); ACE_UNUSED_ARG(thr_id); // Usefull in debugging

        for (;;) {

            if (this->interrupted()) {
                DAF_THROW_EXCEPTION(InterruptedException);
            }

            // Exactly one of item or slot will be nonnull at end of
            // synchronized block, depending on whether a put or a take
            // arrived first. 

            typename SYNCHNode::_ref_type slot,item;

            {
                // Try to match up with a waiting taker; fill and signal it below
                ACE_GUARD_REACTION(_mutex_type, guard, *this, DAF_THROW_EXCEPTION(LockFailureException));
                if (this->waitingTakes.deque(slot.out())) {
                    this->waitingPuts.enque(item = new SYNCHNode(t)); // Put item in waiting for a taker
                }
            }

            // There is a waiting taker - Fill in the slot created by the taker and signal taker to continue.

            if (slot) {

                ACE_GUARD_REACTION(_mutex_type, slot_guard, *slot, DAF_THROW_EXCEPTION(LockFailureException));

                try { // Safe guard against user code error

                    if (this->interrupted()) {
                        DAF_THROW_EXCEPTION(InterruptedException);
                    } else if (slot->put_item(t) ? false : slot->isFULL()) {
                        return 0;
                    }

                } catch (...) {
                    ACE_Errno_Guard g(errno); ACE_UNUSED_ARG(g);
                    slot->state(CANCELLED);
                    throw;
                }

            }
            else if (item) { // Wait for a taker to arrive and take the item.

                ACE_GUARD_REACTION(_mutex_type, item_guard, *item, DAF_THROW_EXCEPTION(LockFailureException));

                while (!item->isCANCELLED()) try {

                    if (this->interrupted()) {
                        DAF_THROW_EXCEPTION(InterruptedException);
                    }
                    else if (item->isFULL() && item->wait(abstime)) {
                        int last_error = DAF_OS::last_error();
                        if (item->isFULL()) {
                            switch (this->interrupted() ? DAF_OS::last_error(EINTR) : last_error) {
                            case EINTR: DAF_THROW_EXCEPTION(InterruptedException);
                            default:
                                {
                                    ACE_Errno_Guard g(errno); ACE_UNUSED_ARG(g);
                                    item->state(CANCELLED);
                                    return -1;
                                }
                            }
                        }
                    }

                    if (item->isEMPTY()) {
                        return 0;
                    }
                }
                catch (...) {
                    ACE_Errno_Guard g(errno); ACE_UNUSED_ARG(g);
                    item->state(CANCELLED);
                    throw;
                }
            }
        }

        return -1; // Should never get here but keeps compiler happy
    }

    template <typename T>
    T
    SynchronousChannel<T>::take(const ACE_Time_Value * abstime)
    {
        // Entirely symmetric to put()

        const ACE_thread_t thr_id(DAF_OS::thr_self()); ACE_UNUSED_ARG(thr_id); // Usefull in debugging

        for (;;) {

            if (this->interrupted()) {
                DAF_THROW_EXCEPTION(InterruptedException);
            }

            // Exactly one of item or slot will be nonnull at end of
            // synchronized block, depending on whether a put or a take
            // arrived first. 

            typename SYNCHNode::_ref_type item,slot;

            {
                ACE_GUARD_REACTION(_mutex_type, guard, *this, DAF_THROW_EXCEPTION(LockFailureException));
                if (this->waitingPuts.deque(item.out())) {
                    this->waitingTakes.enque(slot = new SYNCHNode()); // Create an empty slot to recieve item
                }
            }

            T t = T(); // Temporary Holder for retrieved item

            if (item) {  // Puter has already put item so get it and release putter

                ACE_GUARD_REACTION(_mutex_type, item_guard, *item, DAF_THROW_EXCEPTION(LockFailureException));

                try { // Safe guard against user code error

                    if (this->interrupted()) {
                        DAF_THROW_EXCEPTION(InterruptedException);
                    } else if (item->get_item(t) ? false : item->isEMPTY()) {
                        return t;
                    }

                } catch (...) {
                    ACE_Errno_Guard g(errno); ACE_UNUSED_ARG(g);
                    item->state(CANCELLED);
                    throw;
                }
            }
            else if (slot) { // Wait for a putter to arrive and fill in the slot.

                ACE_GUARD_REACTION(_mutex_type, slot_guard, *slot, DAF_THROW_EXCEPTION(LockFailureException));

                while (!slot->isCANCELLED()) try {

                    if (this->interrupted()) {
                        DAF_THROW_EXCEPTION(InterruptedException);
                    } else if (slot->get_item(t)) {
                        if (slot->isCANCELLED()) {
                            break;
                        } else if (slot->wait(abstime)) {
                            int last_error = DAF_OS::last_error();
                            if (slot->get_item(t)) {
                                switch (this->interrupted() ? DAF_OS::last_error(EINTR) : last_error) {
                                case EINTR: DAF_THROW_EXCEPTION(InterruptedException);
                                case ETIME: DAF_THROW_EXCEPTION(TimeoutException);
                                default:    DAF_THROW_EXCEPTION(InternalException);
                                }
                            }
                        } else {
                            continue; // Go retry to get item if not cancelled
                        }
                    }

                    return t; // return the item
                }
                catch (...) {
                    ACE_Errno_Guard g(errno); ACE_UNUSED_ARG(g);
                    slot->state(CANCELLED);
                    throw;
                }

                // Retry Outer Loop

            }
        }

        DAF_THROW_EXCEPTION(InternalException);  // Should never get here but keeps compiler happy
    }

    template <typename T>
    inline size_t
    SynchronousChannel<T>::size(void) const
    {
        return size_t(this->waitingPuts.size());
    }

    template <typename T>
    inline size_t
    SynchronousChannel<T>::capacity(void) const
    {
        return size_t(this->size() + 1); // Say we can take another one
    }

    /******************************************************************************************/

    template <typename T>
    inline
    SynchronousChannel<T>::SYNCHNode::SYNCHNode(void) : item_()
        , state_(EMPTY)
    {
    }

    template <typename T>
    inline
    SynchronousChannel<T>::SYNCHNode::SYNCHNode(const T & t) : item_(t)
        , state_(FULL)
    {
    }

    template <typename T> int
    SynchronousChannel<T>::SYNCHNode::state(void) const
    {
        return this->state_;
    }

    template <typename T> int // Called with lock held
    SynchronousChannel<T>::SYNCHNode::state(int state)
    {
        switch (this->interrupted() ? (state = CANCELLED) : state) {
        case CANCELLED:
        case EMPTY: this->item_ = T();
        case FULL:  break;
        default:    return -1;
        }

        this->state_ = state; return this->signal();
    }

    template <typename T> int // Called with lock held
    SynchronousChannel<T>::SYNCHNode::put_item(const T & t)
    {
        if (this->isEMPTY()) {
            this->item_ = t; this->state(FULL); return 0;
        }
        return -1;
    }

    template <typename T> int // Called with lock held
    SynchronousChannel<T>::SYNCHNode::get_item(T & t)
    {
        if (this->isFULL()) {
            t = this->item_; this->state(EMPTY); return 0;
        }
        return -1;
    }

    /**********************************************************************************************/

    template <typename T> int // Called with lock held
    SynchronousChannel<T>::WaiterQueue::deque(typename SYNCHNode::_out_type node)
    {
        while (!this->empty()) {
            typename _waiter_list_type::value_type item(this->front()._retn()); this->pop_front();
            if (item) {
                if (item->isCANCELLED()) {
                    continue;
                }
                node = item._retn(); return 0;
            }
        }

        return -1;
    }

    template <typename T> inline int // Called with lock held
    SynchronousChannel<T>::WaiterQueue::enque(const typename SYNCHNode::_ref_type & node)
    {
        this->push_back(node); return 0;
    }

    /**********************************************************************************************/

} // namespace DAF

#endif  // DAF_SYNCHRONOUSCHANNEL_T_CPP
