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
*(c)Copyright 2011,
*   Defence Science and Technology Organisation,
*   Department of Defence,
*   Australia.
*
* All rights reserved.
*
* This is unpublished proprietary source code of DSTO.
* The copyright notice above does not evidence any actual or
* intended publication of such source code.
*
* The contents of this file must not be disclosed to third
* parties, copied or duplicated in any form, in whole or in
* part, without the express prior written permission of DSTO.
*
*
* @file     Barrier.h
* @author   Derek Dominish
* @author   $LastChangedBy$
* @date     1st September 2011
* @version  $Revision$
* @ingroup
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

namespace DAF
{
    Barrier::Barrier(size_t parties, const Runnable_ref &command)
        : entryGate_        (int(parties))
        , parties_          (parties)
        , synch_            (0)
        , count_            (0)
        , resets_           (0)
        , broken_           (false)
        , shutdown_         (false)
        , triggered_        (false)
        , barrierCommand_   (command)
    {
        if ( parties == 0 ) throw InitializationException("DAF::Barrier Initialization Error parties == 0");
    }

    Barrier::~Barrier(void)
    {
        ACE_GUARD( ACE_SYNCH_MUTEX, ace_mon, *this );
        this->shutdown_ = true;
        while (this->count_ > 0) {
            this->notifyAll();
            if (this->wait(100) && errno == ETIME) {
                if (this->broken_) {
                    this->entryGate_.release(int(this->count_)); break;
                } else this->broken_ = true;
            }
        }
    }

    int     Barrier::barrier(void)
    {
        this->entryGate_.acquire();

        ACE_Guard<ACE_SYNCH_MUTEX> ace_mon( *this );

        size_t index = this->count_;

        this->resets_ = ++this->count_;

        while (!(this->broken_ || this->triggered_)) try {
            if (this->shutdown_) {
                this->broken_ = true;
            } else if (this->count_ != this->parties_) {
                this->wait();
            } else if (DAF::is_nil(this->barrierCommand_)) {
                this->triggered_ = true;
            } else {
                this->barrierCommand_->run(); this->triggered_ = true;
            }
        } catch(...) {
            this->broken_ = true;
            --this->resets_;
            this->notifyAll();
            throw;
        }

        bool broken = this->broken_, shutdown = this->shutdown_; // Get state onto the stack

        this->notifyAll();

        if (--this->resets_ == 0) {
            this->entryGate_.release(int(this->count_));
            this->broken_ = this->triggered_ = false; ++this->synch_;
            this->count_ = 0;
        }

        if (shutdown) {
            throw IllegalThreadStateException();
        } else if (broken) {
            throw BrokenBarrierException();
        }

        return int(index);
    }

    int   Barrier::barrier(time_t msecs)
    {
        if (this->entryGate_.attempt(msecs)) {
            throw TimeoutException();
        }

        ACE_Guard<ACE_SYNCH_MUTEX> ace_mon( *this );

        ACE_Time_Value end_time(DAF_OS::gettimeofday(msecs));

        bool timeout    = false;
        size_t index    = this->count_;

        this->resets_   = ++this->count_;

        while (!(this->broken_ || this->triggered_)) try {
            if (this->shutdown_) {
                this->broken_ = true;
            } else if (this->count_ != this->parties_) {
                if (end_time > DAF_OS::gettimeofday()) {
                    this->wait(end_time);
                } else timeout = this->broken_ = true;
            } else if (DAF::is_nil(this->barrierCommand_)) {
                this->triggered_ = true;
            } else  {
                this->barrierCommand_->run(); this->triggered_ = true;
            }
        } catch(...) {
            this->broken_ = true;
            --this->resets_;
            this->notifyAll();
            throw;
        }

        this->notifyAll();

        bool broken = this->broken_, shutdown = this->shutdown_; // Get state onto the stack

        if (--this->resets_ == 0) {
            this->entryGate_.release(int(this->count_));
            this->broken_ = this->triggered_ = false; ++this->synch_;
            this->count_ = 0;
        }

        if (timeout) {
            throw DAF::TimeoutException();
        } else if (shutdown) {
            throw DAF::IllegalThreadStateException();
        } else if (broken) {
            throw DAF::BrokenBarrierException();
        }

        return int(index);
    }

    bool    Barrier::waitReset(time_t msecs)
    {
        ACE_Guard<ACE_SYNCH_MUTEX> ace_mon( *this );

        for (size_t synch = this->synch_; this->count_ > 0;) {
            if (this->shutdown_) {
                throw DAF::IllegalThreadStateException();
            } else if (this->synch_ == synch) {
                if (this->triggered_ || this->broken_) {
                    this->notifyAll();
                }
                if (this->wait(DAF_OS::gettimeofday(msecs))) {
                    this->broken_ = true; this->notifyAll(); return false;
                }
            } else break;
        }

        return true;
    }
} // namespace DAF
