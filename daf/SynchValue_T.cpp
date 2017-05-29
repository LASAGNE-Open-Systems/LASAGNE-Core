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
#ifndef DAF_SYNCHVALUE_T_CPP
#define DAF_SYNCHVALUE_T_CPP

#include "SynchValue_T.h"

namespace DAF
{
    template <typename T>
    SynchValue<T>::SynchValue(const T &value) : DAF::Monitor()
        , value_(value)
        , shutdown_(false)
        , waiters_(0)
        , itemTaken_(0)
        , setterGate_(1)
    {
    }

    template <typename T>
    SynchValue<T>::~SynchValue()
    {
        // Got a couple of options on shutdown..
        // we can either poll the waiters_ value and wait for others to exit
        // or we can use the itemTaken_ semaphore.
        ACE_GUARD(ACE_SYNCH_MUTEX, ace_mon, *this);

        this->shutdown_ = true;

        // flush our setters - they should hit the shutdown flag
        int setter_waiters = this->setterGate_.waiters();
        this->setterGate_.release(setter_waiters);

        // flush a setter in mid-cycle
        this->itemTaken_.release(int(this->waiters_));


        // flush our waiters.
        while( this->waiters_ > 0 ) {
            this->notifyAll();

            if (this->wait(100) && errno == ETIME ) {
                // CRUDE: using wait with timeout allow others to clear
                // out before destroying. We don't expect a
                // corresponding notify.
                break;
            }
        }
    }

    template <typename T> T
    SynchValue<T>::getValue(void) const
    {
        ACE_GUARD_RETURN( ACE_SYNCH_MUTEX, ace_mon, *this, this->value_ ); return this->value_;
    }

    template <typename T> int
    SynchValue<T>::setValue(const T &value)
    {
        if ( this->shutdown_ ) return -1;

        this->setterGate_.acquire();

        if (this->value_ == value) { // DCL
            return 0;
        } else if ( !this->shutdown_ )   {
            int waiters = 0;

            { // scope guard.
                ACE_GUARD_RETURN( ACE_SYNCH_MUTEX, ace_mon, *this, -1 );
                if ( this->shutdown_ ) return -1;

                // store the number of waiters we need to wait for.
                waiters = int(this->waiters_ - 1 + this->itemTaken_.permits());
                //ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - Set Waiters %d %d\n", waiters, this->itemTaken_.permits()));
                while (this->value_ == value ? false : true) {
                    this->value_ = value;
                }
                this->notifyAll();
            } // scope guard exit

            this->itemTaken_.acquire(waiters);
        }

        this->setterGate_.release();

        return 0;
    }

    template <typename T> int
    SynchValue<T>::waitValue(const T &value)
    {
        if (this->value_ == value) { // DCL
            return 0;
        } else if(!this->shutdown_) {
            ACE_GUARD_RETURN( ACE_SYNCH_MUTEX, ace_mon, *this, -1 );
            ++this->waiters_;

            while (this->value_ == value ? false : true) try {
                if(this->shutdown_) {
                    break;
                } else {
                    this->wait();
                }
                this->itemTaken_.release();
            } catch(...) { // JB: Deliberate catch(...) - DON'T replace with DAF_CATCH_ALL
                --this->waiters_;
                // Are we in the middle of a setter cycle.
                if ( this->setterGate_.permits() == 0 )
                {
                    this->itemTaken_.release();
                }
                throw;
            }
            --this->waiters_;

            if (this->shutdown_) {
                throw DAF::IllegalThreadStateException();
            }
        } else {
            // shutdown ?
            throw DAF::IllegalThreadStateException();
        }
        return 0;
    }

    template <typename T> int
    SynchValue<T>::waitValue(const T &value, time_t msec)
    {
        const ACE_Time_Value end_time(DAF_OS::gettimeofday(msec));

        if (this->value_ == value) { // DCL
            return 0;
        } else if(!shutdown_) {
            ACE_GUARD_RETURN( ACE_SYNCH_MUTEX, ace_mon, *this, -1 );
            ++this->waiters_;
            bool timeout = false;

            while (this->value_ == value ? false : true) try {
                if(this->shutdown_) {
                    break;
                } else if (end_time > DAF_OS::gettimeofday() ) {
                    this->wait(end_time);
                } else {
                    timeout = true;
                    break;
                }
                this->itemTaken_.release();
            } catch(...) { // JB: Deliberate catch(...) - DON'T replace with DAF_CATCH_ALL
                --this->waiters_;
                // Are we in the middle of a setter cycle.
                if ( this->setterGate_.permits() == 0 )
                {
                    this->itemTaken_.release();
                }
                throw;
            }
            --this->waiters_;

            if (timeout) {
                throw TimeoutException();
            }

            if (this->shutdown_){
                throw IllegalThreadStateException();
            }
        } else {
            // shutdown ?
            throw IllegalThreadStateException();
        }

        return 0;
    }
} // namespace DAF

#endif // DAF_SYNCHVALUE_T_CPP
