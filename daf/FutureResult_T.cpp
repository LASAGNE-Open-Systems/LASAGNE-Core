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
#ifndef DAF_FUTURERESULT_T_CPP
#define DAF_FUTURERESULT_T_CPP

#include "FutureResult_T.h"

namespace DAF
{
    template <typename T, typename F>
    FutureResult<T,F>::FutureResult(void) : ready_(false)
        , error_(false)
        , waiters_(0)
    {
    }

    template <typename T, typename F>
    FutureResult<T,F>::~FutureResult(void)
    {
        ACE_Guard<ACE_SYNCH_MUTEX> ace_mon(*this);

        this->error_ = true;

        while(this->waiters_ > 0 ) {
            this->notifyAll();

            if ( this->wait(100) && errno == ETIME) {
                // do nothing? break?
            }
        }
        //this->wait(100);
    }

    template <typename T, typename F> const T&
    FutureResult<T,F>::peek(void) const
    {
        ACE_Guard<ACE_SYNCH_MUTEX> ace_mon(*this); return this->value_;
    }

    template <typename T, typename F> T
    FutureResult<T,F>::get(void) const
    {
        {
            ACE_Guard<ACE_SYNCH_MUTEX> ace_mon(*this);
            ++this->waiters_;
            while(!this->error_) try {
                if (this->ready_) {
                    --this->waiters_;
                    return this->value_;
                }
                this->wait();
            } catch(...) { // JB: Deliberate catch(...) - DON'T replace with DAF_CATCH_ALL
                --this->waiters_;
                this->error_ = true;
                throw;
            }

            --this->waiters_;
        }

        throw InvocationTargetException();
    }

    template <typename T, typename F> T
    FutureResult<T,F>::get(time_t msecs) const
    {
        bool timeout = false;
        {
            ACE_Guard<ACE_SYNCH_MUTEX> ace_mon(*this);

            ++this->waiters_;

            const ACE_Time_Value end_time(DAF_OS::gettimeofday(msecs));

            while (!this->error_ && !timeout) try {
                if (this->ready_) {
                    --this->waiters_;
                    return this->value_;
                } else if (end_time > DAF_OS::gettimeofday()) {
                    this->wait(end_time);
                } else {
                    timeout = true;
                }
            } catch(...) { // JB: Deliberate catch(...) - DON'T replace with DAF_CATCH_ALL
                --this->waiters_;
                this->error_ = true;
                throw;
            }

            --this->waiters_;
        }

        if ( timeout) {
            throw TimeoutException();
        }

        throw InvocationTargetException();
    }

    template <typename T, typename F> void
    FutureResult<T,F>::reset(void)
    {
        ACE_Guard<ACE_SYNCH_MUTEX> ace_mon(*this);
        //this->value_ = T();
        this->ready_ = this->error_ = false;
    }

    template <typename T, typename F> Runnable_ref
    FutureResult<T,F>::operator () (_functor_type& fn, const _value_type& value)
    {
        this->reset(); return new FutureFunctor(*this, fn, value);
    }

    template <typename T, typename F> Runnable_ref
    FutureResult<T,F>::operator () (const T& value)
    {
        this->reset(); return new FutureFunctor(*this,_default_setter, value);
    }

    template <typename T, typename F> void
    FutureResult<T,F>::setError(void)
    {
        ACE_Guard<ACE_SYNCH_MUTEX> ace_mon(*this);
        this->value_ = T();
        this->error_ = true;
        this->notifyAll();
    }

    template <typename T, typename F> void
    FutureResult<T,F>::setValue(const T &value)
    {
        ACE_Guard<ACE_SYNCH_MUTEX> ace_mon(*this);
        this->value_ = value;
        this->ready_ = true;
        this->notifyAll();
    }

    template <typename T, typename F> int
    FutureResult<T,F>::FutureSetter::run(void)
    {
        try {
            r_.setValue(fn_(args_)); return 0;
        } DAF_CATCH_ALL_ACTION(r_.setError()) {
            r_.setError();
        }

        return -1;
    }

    template <typename T, typename F> int
    FutureResult<T,F>::FutureFunctor::run(void)
    {
        try {
            r_.setValue(func_(value_)); return 0;
        } DAF_CATCH_ALL_ACTION(r_.setError()) {
            r_.setError();
        }

        return -1;
    }
} // namespace DAF

#endif // DAF_FUTURERESULT_T_CPP
