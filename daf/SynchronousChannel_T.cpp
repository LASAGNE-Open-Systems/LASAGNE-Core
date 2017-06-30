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

        const ACE_thread_t thr_id(DAF_OS::thr_self()); ACE_UNUSED_ARG(thr_id);

        for (;;) {

            if (this->interrupted()) {
                DAF_THROW_EXCEPTION(InterruptedException);
            }

            // Exactly one of item or slot will be nonnull at end of
            // synchronized block, depending on whether a put or a take
            // arrived first. 

            SYNCHNode_ref taker(0), puter(0);

            {
                // Try to match up with a waiting taker; fill and signal it below
                ACE_GUARD_REACTION(_mutex_type, guard, *this, DAF_THROW_EXCEPTION(LockFailureException));
                if (this->waitingTakes.deque(taker.out())) {
                    this->waitingPuts.enque(puter = new SYNCHNode(t)); // Put one in waiting for a taker
                }
            }

            // There is a waiting taker - Fill in the slot created by the taker and signal taker to continue.

            if (taker) {

                ACE_GUARD_REACTION(_mutex_type, taker_guard, *taker, continue);
                if (taker->put_item(t) ? false : taker->isFULL()) {
                    return 0;
                }
            }
            else if (puter) { // Wait for a taker to arrive and take the item.

                ACE_GUARD_REACTION(_mutex_type, puter_guard, *puter, continue);

                while (!puter->isCANCELLED()) try {

                    if (puter->isFULL()) {
                        if (puter->wait(abstime) && DAF_OS::last_error() == ETIME) {
                            ACE_Errno_Guard g(errno); ACE_UNUSED_ARG(g);
                            switch (puter->state()) {
                            case EMPTY: return 0;
                            default:    puter->state(CANCELLED); return -1;
                            }
                        }
                    }

                    if (puter->isEMPTY()) {
                        return 0;
                    }
                }
                catch (...) {
                    ACE_Errno_Guard g(errno); ACE_UNUSED_ARG(g);
                    puter->state(CANCELLED);
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

        const ACE_thread_t thr_id(DAF_OS::thr_self()); ACE_UNUSED_ARG(thr_id);

        for (;;) {

            if (this->interrupted()) {
                DAF_THROW_EXCEPTION(InterruptedException);
            }

            // Exactly one of item or slot will be nonnull at end of
            // synchronized block, depending on whether a put or a take
            // arrived first. 

            SYNCHNode_ref puter(0), taker(0);

            {
                ACE_GUARD_REACTION(_mutex_type, guard, *this, DAF_THROW_EXCEPTION(LockFailureException));
                if (this->waitingPuts.deque(puter.out())) {
                    this->waitingTakes.enque(taker = new SYNCHNode()); // Create an empty one to recieve item
                }
            }

            T t = T(); // Temporary Holder for retrieved item

            if (puter) {  // Puter has already put item so get it and release putter

                ACE_GUARD_REACTION(_mutex_type, puter_guard, *puter, continue);
                if (puter->get_item(t) ? false : puter->isEMPTY()) {
                    return t;
                }

            }
            else if (taker) { // Wait for a putter to arrive and set the item.

                ACE_GUARD_REACTION(_mutex_type, taker_guard, *taker, continue);

                while (!taker->isCANCELLED()) {

                    if (taker->get_item(t)) {
                        if (taker->wait(abstime) == 0) {
                            continue;
                        }
                        else if (taker->get_item(t)) {
                            switch (DAF_OS::last_error()) {
                            case ETIME: DAF_THROW_EXCEPTION(TimeoutException);
                            default:    DAF_THROW_EXCEPTION(InternalException);
                            }
                        }
                    }

                    if (taker->isEMPTY()) {
                        return t;
                    }
                }
            }

            // Retry Outer Loop

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
        switch (this->interrupted() ? CANCELLED : state) {
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
        if (this->isCANCELLED() ? false : this->isEMPTY()) {
            this->item_ = t; this->state(FULL); return 0;
        }
        return -1;
    }

    template <typename T> int // Called with lock held
    SynchronousChannel<T>::SYNCHNode::get_item(T & t)
    {
        if (this->isCANCELLED() ? false : this->isFULL()) {
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
            if (item->isCANCELLED() ? false : (node = item._retn())) {
                return 0;
            }
        }
        node = 0; return -1;
    }

    template <typename T> inline int // Called with lock held
    SynchronousChannel<T>::WaiterQueue::enque(const SYNCHNode_ref & node)
    {
        this->push_back(node); return 0;
    }

    /**********************************************************************************************/

} // namespace DAF

#endif  // DAF_SYNCHRONOUSCHANNEL_T_CPP
