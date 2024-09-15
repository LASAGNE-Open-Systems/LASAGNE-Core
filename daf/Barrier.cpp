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
#define DAF_BARRIER_CPP

/************************ Barrier ******************************
*
* A Barrier for a fixed number of threads with an all-or-none breakage model.
*
* A Barrier is a reasonable choice for a barrier in
* contexts involving a fixed sized group of threads that
* must occasionally wait for each other.
* (A Rendezvous better handles applications in which
* any number of threads meet, n-at-a-time.)
*
* Barriers use an all-or-none breakage model
* for failed synchronization attempts: If threads
* leave a barrier point prematurely because of timeout
* or interruption, others will also leave abnormally
* (via BrokenBarrierException), until
* the barrier is <code>restart</code>ed. This is usually
* the simplest and best strategy for sharing knowledge
* about failures among cooperating threads in the most
* common usages contexts of Barriers.
* This implementation  has the property that interruptions
* among newly arriving threads can cause as-yet-unresumed
* threads from a previous barrier cycle to return out
* as broken. This transmits breakage
* as early as possible, but with the possible byproduct that
* only some threads returning out of a barrier will realize
* that it is newly broken. (Others will not realize this until a
* future cycle.) (The Rendezvous class has a more uniform, but
* sometimes less desirable policy.)
*
* Barriers support an optional Runnable command
* that is run once per barrier point.
****************************************************************/

#include "Barrier.h"

#include "DirectExecutor.h"

namespace DAF
{
    Barrier::Barrier(int parties, const Runnable_ref & command) : Monitor()
        , barrierSemaphore_ (ace_max(0, parties))
        , barrierCommand_   (command)
        , parties_          (0)
        , resets_           (0)
        , count_            (0)
        , broken_           (false)
        , triggered_        (false)
    {
        if ((this->parties_ = this->barrierSemaphore_.permits()) == 0) {
            DAF_THROW_EXCEPTION(InitializationException);
        }
    }

    Barrier::~Barrier(void)
    {
        this->interrupt(); ACE_GUARD(_mutex_type, mon, *this);
        for (const ACE_Time_Value tv(DAF_OS::gettimeofday(DAF_MSECS_ONE_SECOND)); this->count_ > 0;) {
            if (this->wait(tv) && DAF_OS::last_error() == ETIME) { // Wait for threads to exit
                break;
            }
        }
    }

    int
    Barrier::interrupt(void)
    {
        return Monitor::interrupt() + this->barrierSemaphore_.interrupt() ? -1 : 0;
    }

    bool
    Barrier::broken(void) const
    {
        return this->broken_;
    }

    int
    Barrier::parties(void) const
    {
        return this->parties_;
    }

    void
    Barrier::setBarrierCommand(const Runnable_ref & command)
    {
        ACE_GUARD(_mutex_type, mon, *this); this->barrierCommand_ = command;
    }

    int
    Barrier::barrier(const ACE_Time_Value * abstime)
    {
        int index = 0, resets = 0;

        { // Process entry gate

            ACE_GUARD_REACTION(_mutex_type, mon, *this, DAF_THROW_EXCEPTION(LockFailureException));

            if (this->interrupted()) {
                DAF_THROW_EXCEPTION(InterruptedException);
            }
            else if (this->triggered_ || this->broken_ || 0 >= this->barrierSemaphore_.permits()) {
                DAF_THROW_EXCEPTION(IllegalStateException);
            }
            else if (this->barrierSemaphore_.acquire(abstime)) {
                switch (this->interrupted() ? DAF_OS::last_error(EINTR) : DAF_OS::last_error()) {
                case EINTR: DAF_THROW_EXCEPTION(InterruptedException);
                case ETIME: DAF_THROW_EXCEPTION(TimeoutException);
                default:    DAF_THROW_EXCEPTION(IllegalStateException);
                }
            }

            resets  = this->resets_;
            index   = this->count_++;
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

                if (--this->count_ > 0 ? (this->signal(), false) : true) {
                    this->resetBarrier();
                }

                return index;
            }
            else if (this->barrierSemaphore_.permits() > 0) {
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
                DirectExecutor().execute(this->barrierCommand_); this->broadcast();
            }

        } catch (...) {
            ACE_Errno_Guard g(errno); ACE_UNUSED_ARG(g);
            if (--this->count_ > 0) {
                this->broken_ = true; this->broadcast();
            } else {
                this->resetBarrier();
            }
            throw;
        }
    }

    int
    Barrier::waitReset(const ACE_Time_Value * abstime)
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
                    this->resetBarrier();
                }
            }
        }

        return last_error ? (DAF_OS::last_error(last_error), -1) : 0;
    }

    int // called with Monitor locks held
    Barrier::resetBarrier(void)
    {
        // Reset our state
        this->broken_       = false;
        this->triggered_    = false;
        this->count_        = 0;

        // Reset permits on the entry gate
        for (int permits = this->parties() - this->barrierSemaphore_.permits(); permits > 0;) {
            this->barrierSemaphore_.release(permits); break; // Set our permits back
        }

        ++this->resets_; return this->signal();
    }

} // namespace DAF
