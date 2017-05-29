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
    template <typename T> void
    RendezvousRotator<T>::operator () (std::vector<T> &val)
    {
        if (!val.empty()) try {
            val.push_back(val.front()); val.erase(val.begin());
        } DAF_CATCH_ALL { /* Ignore Error */ }
    }

    /********************************************************************/

    template <typename T, typename F>
    Rendezvous<T,F>::Rendezvous(size_t parties,  F &fn)
        : entryGate_    (int(parties))
        , parties_      (parties)
        , synch_        (0)
        , count_        (0)
        , resets_       (0)
        , broken_       (false)
        , shutdown_     (false)
        , triggered_    (false)
        , fn_           (fn)
    {
        if ( parties == 0 ) throw InitializationException("DAF::Rendezvous Initialization Error parties == 0");

        this->slots_.reserve(parties + 2);  // Reserve enough space for manipulation (+2)
    }

    template <typename T, typename F>
    Rendezvous<T,F>::~Rendezvous(void)
    {
        ACE_Guard<ACE_SYNCH_MUTEX> ace_mon( *this );
        this->shutdown_ = true;  // signal we are shutting down

        // clear out the entryGate_.waiters(). They should hit
        // the shutdown flag and exit.
        this->entryGate_.release(this->entryGate_.waiters() + 1);

        while (this->slots_.size() && this->count_ != 0 ) {
            // Having set the shutdown_ flag. All waiters should be notified.
            // But we need to release the lock on this->wait(). One thing to consider
            // is whether this->wait() will be reacquired prior to all threads have exit?
            // The entrant threads need the object's state to complete.
            // If destruction is found to be problematic consider refactoring
            // the rendezvous entrant logic to push state on the stack prior to
            // subsequent calls to notifyAll().
            this->notifyAll();

            this->wait(100);
        }
    }

    template <typename T, typename F> bool
    Rendezvous<T,F>::waitReset(time_t msecs)
    {
        ACE_Guard<ACE_SYNCH_MUTEX> ace_mon( *this );

        for (size_t synch = this->synch_; this->slots_.size();) try {
            if (this->shutdown_) {
                throw IllegalThreadStateException();
            } else if (this->synch_ == synch) {
                if (this->triggered_ || this->broken_) {
                    this->notifyAll();
                }
                if (this->wait(DAF_OS::gettimeofday(msecs))) {
                    this->broken_ = true;
                    this->notifyAll();
                    return false;
                }
            } else break;
        } catch(...) { // JB: Deliberate catch(...) - DON'T replace with DAF_CATCH_ALL
            this->broken_ = true;
            this->notifyAll();
            throw;
        }

        return true;
    }

    template <typename T, typename F> T
    Rendezvous<T,F>::rendezvous(const T &t)
    {
        this->entryGate_.acquire();

        if ( this->shutdown_ ) {
            throw IllegalThreadStateException();
        }

        ACE_Guard<ACE_SYNCH_MUTEX> ace_mon( *this );

        size_t index = this->count_;

        T t_rtn(t);

        this->slots_.push_back(t);

        this->resets_ = ++this->count_;

        while (!(this->broken_ || this->triggered_)) try {
            if (this->shutdown_) {
                this->broken_ = true;
            } else if (this->count_ != this->parties_) {
                this->wait();
            } else  {
                this->fn_(this->slots_);
                this->triggered_ = true;
            }
        } catch(...) { // JB: Deliberate catch(...) - DON'T replace with DAF_CATCH_ALL
            --this->resets_;
            --this->count_;
            this->broken_ = true;
            this->notifyAll();
            throw;
        }

        if (index >= this->slots_.size()) {
            this->broken_ = true;
        } else {
            t_rtn = this->slots_[index];
        }

        bool broken = this->broken_, shutdown = this->shutdown_; // Get state onto the stack

        this->notifyAll();

        if (--this->resets_ == 0) {
            this->slots_.clear();
            this->entryGate_.release(int(this->count_));
            this->broken_ = this->triggered_ = false; ++this->synch_;
            this->count_ = 0;
        }

        if (shutdown) {
            throw IllegalThreadStateException();
        } else if (broken) {
            throw BrokenBarrierException();
        }

        return t_rtn;
    }

    template <typename T, typename F> T
    Rendezvous<T,F>::rendezvous(const T &t, time_t msec)
    {
        if (this->entryGate_.attempt(msec)) {
            throw TimeoutException();
        }

        if ( this->shutdown_ ) {
            throw IllegalThreadStateException();
        }

        ACE_Guard<ACE_SYNCH_MUTEX> ace_mon( *this );

        const ACE_Time_Value end_time(DAF_OS::gettimeofday(msec));

        bool timeout    = false;
        size_t index    = this->count_;

        T t_rtn(t);

        this->slots_.push_back(t);
        this->resets_ = ++this->count_;

        while (!(this->broken_ || this->triggered_)) try {
            if (this->shutdown_) {
                this->broken_ = true;
            } else if (this->count_ != this->parties_) {
                if (end_time > DAF_OS::gettimeofday()) {
                    this->wait(end_time);
                } else timeout = this->broken_ = true;
            } else {
                this->fn_(this->slots_);
                this->triggered_ = true;
            }
        } catch(...) { // JB: Deliberate catch(...) - DON'T replace with DAF_CATCH_ALL
            --this->resets_;
            --this->count_;
            this->broken_ = true;
            this->notifyAll();
            throw;
        }

        if (index >= this->slots_.size()) {
            this->broken_ = true;
        } else {
            t_rtn = this->slots_[index];
        }

        bool broken = this->broken_, shutdown = this->shutdown_; // Get state onto the stack

        this->notifyAll();

        if (--this->resets_ == 0) {
            this->slots_.clear();
            this->entryGate_.release(int(this->count_));
            this->broken_ = this->triggered_ = false; ++this->synch_;
            this->count_ = 0;
        }

        if (timeout) {
            throw TimeoutException();
        } else if (shutdown) {
            throw IllegalThreadStateException();
        } else if (broken) {
            throw BrokenBarrierException();
        }

        return t_rtn;
    }
} // namespace DAF

#endif  // DAF_RENDEZVOUS_T_CPP
