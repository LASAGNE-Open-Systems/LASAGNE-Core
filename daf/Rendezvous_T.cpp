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
#ifndef DAF_RENDEZVOUS_T_CPP
#define DAF_RENDEZVOUS_T_CPP

#include "Rendezvous_T.h"

namespace DAF
{
    template <typename T> typename RendezvousCommand<T>::_result_type
    RendezvousCommand<T>::operator () (typename RendezvousCommand<T>::_argument_type & val)
    {
        if (val.empty() ? false : true) {
            val.push_back(val.front()); val.erase(val.begin());
        }
    }

    /********************************************************************/

    template <typename T, typename F>
    Rendezvous<T,F>::Rendezvous(int parties, typename Rendezvous<T,F>::_function_type & function) : Monitor()
        , rendezvousSemaphore_  (ace_max(0, parties))
        , rendezvousFunction_   (function)
        , parties_              (0)
        , resets_               (0)
        , count_                (0)
        , broken_               (false)
        , triggered_            (false)
    {
        if ((this->parties_ = this->rendezvousSemaphore_.permits()) == 0) {
            DAF_THROW_EXCEPTION(InitializationException);
        }
        this->rendezvousSlots_.reserve(this->parties_ + 2); // Reserve enough space for manipulation (+2)
    }

    template <typename T, typename F>
    Rendezvous<T, F>::~Rendezvous(void)
    {
        this->interrupt(); ACE_GUARD(_mutex_type, mon,*this);
        for (const ACE_Time_Value tv(DAF_OS::gettimeofday(DAF_MSECS_ONE_SECOND)); this->count_ > 0;) {
            if (this->wait(tv) && DAF_OS::last_error() == ETIME) { // Wait for threads to exit
                break;
            }
        }
    }

    template <typename T, typename F>
    int
    Rendezvous<T, F>::interrupt(void)
    {
        return Monitor::interrupt() + this->rendezvousSemaphore_.interrupt() ? -1 : 0;
    }

    template <typename T, typename F>
    bool
    Rendezvous<T, F>::broken(void) const
    {
        return this->broken_;
    }

    template <typename T, typename F>
    int
    Rendezvous<T, F>::parties(void) const
    {
        return this->parties_;
    }

    template <typename T, typename F>
    T
    Rendezvous<T, F>::rendezvous(const T & t, const ACE_Time_Value * abstime)
    {
        int index = 0, resets = 0;

        { // Process entry gate

            ACE_GUARD_REACTION(_mutex_type, mon, *this, DAF_THROW_EXCEPTION(LockFailureException));

            if (this->interrupted()) {
                DAF_THROW_EXCEPTION(InterruptedException);
            }
            else if (this->triggered_ || this->broken_ || 0 >= this->rendezvousSemaphore_.permits()) {
                DAF_THROW_EXCEPTION(IllegalStateException);
            }
            else if (this->rendezvousSemaphore_.acquire(abstime)) {
                switch (this->interrupted() ? DAF_OS::last_error(EINTR) : DAF_OS::last_error()) {
                case EINTR: DAF_THROW_EXCEPTION(InterruptedException);
                case ETIME: DAF_THROW_EXCEPTION(TimeoutException);
                default:    DAF_THROW_EXCEPTION(IllegalStateException);
                }
            }

            resets  = this->resets_;
            index   = this->count_++;

            this->rendezvousSlots_.push_back(t);
        }

        for (;;) try {

            ACE_GUARD_REACTION(_mutex_type, mon, *this, DAF_THROW_EXCEPTION(LockFailureException));

            if (this->interrupted()) {
                DAF_THROW_EXCEPTION(InterruptedException);
            }
            else if (resets != this->resets_) { // Same Transaction
                DAF_THROW_EXCEPTION(IllegalStateException);
            }
            else if (this->broken_) {
                DAF_THROW_EXCEPTION(BrokenBarrierException);
            }
            else if (this->triggered_) {

                T t_rtn = this->rendezvousSlots_[index];

                if (--this->count_ > 0 ? (this->signal(), false) : true) {
                    this->resetRendezvous();
                }

                return t_rtn;
            }
            else if (this->rendezvousSemaphore_.permits() > 0) {
                if (this->interrupted() || this->wait(abstime)) {
                    switch (this->interrupted() ? DAF_OS::last_error(EINTR) : DAF_OS::last_error()) {
                    case EINTR: DAF_THROW_EXCEPTION(InterruptedException);
                    case ETIME:

                        if (this->triggered_ || this->broken_) {
                            break;
                        }

                        DAF_THROW_EXCEPTION(TimeoutException);

                    default:  this->broken_ = true; break;
                    }
                }
            }
            else if ((this->triggered_ || this->broken_) ? false : (this->triggered_ = true)) {
                this->rendezvousFunction_(this->rendezvousSlots_); this->broadcast();
            }
        }
        catch (...) {
            ACE_Errno_Guard g(errno); ACE_UNUSED_ARG(g);
            if (--this->count_ > 0) {
                this->broken_ = true; this->broadcast();
            }
            else {
                this->resetRendezvous();
            }
            throw;
        }
    }

    template <typename T, typename F>
    int
    Rendezvous<T, F>::waitReset(const ACE_Time_Value * abstime)
    {
        if (this->interrupted()) {
            DAF_THROW_EXCEPTION(InterruptedException);
        }

        int last_error = 0;

        { // Scope Lock

            ACE_GUARD_REACTION(_mutex_type, mon, *this, DAF_THROW_EXCEPTION(LockFailureException));

            int resets = this->resets_;

            for (ACE_Time_Value * abstimer = const_cast<ACE_Time_Value *>(abstime); resets == this->resets_;) {
                if (this->count_ > 0) {
                    if (this->interrupted() || this->wait(abstimer)) {
                        switch (this->interrupted() ? EINTR : (last_error = DAF_OS::last_error())) {
                        case EINTR: DAF_THROW_EXCEPTION(InterruptedException);
                        default: this->broken_ = true; this->broadcast(); abstimer = 0; // Set infinate wait
                        }
                    }
                } else {
                    this->resetRendezvous();
                }
            }
        }

        return last_error ? (DAF_OS::last_error(last_error), -1) : 0;
    }

    template <typename T, typename F>
    int // called with Monitor locks held
    Rendezvous<T, F>::resetRendezvous(void)
    {
        // Reset our state
        this->broken_       = false;
        this->triggered_    = false;
        this->count_        = 0;

        this->rendezvousSlots_.clear();

        // Reset permits on the entry gate
        for (int permits = this->parties() - this->rendezvousSemaphore_.permits(); permits > 0;) {
            this->rendezvousSemaphore_.release(permits); break; // Set our permits back
        }

        ++this->resets_; return this->signal();
    }


} // namespace DAF

#endif  // DAF_RENDEZVOUS_T_CPP
