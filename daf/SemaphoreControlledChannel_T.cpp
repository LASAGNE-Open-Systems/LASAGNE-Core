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
#ifndef DAF_SEMAPHORECONTROLLEDCHANNEL_T_CPP
#define DAF_SEMAPHORECONTROLLEDCHANNEL_T_CPP

#include "SemaphoreControlledChannel_T.h"

namespace DAF
{
    template <typename T> ACE_INLINE
    SemaphoreControlledChannel<T>::SemaphoreControlledChannel(size_t capacity) : Channel<T>()
        , putGuard_(int(capacity)), takeGuard_(0)
    {
    }

    template <typename T> ACE_INLINE
    SemaphoreControlledChannel<T>::~SemaphoreControlledChannel(void)
    {
        this->interrupt();
    }

    template <typename T> ACE_INLINE size_t
    SemaphoreControlledChannel<T>::capacity(void) const
    {
        ACE_GUARD_RETURN(DAF_SYNCH_MUTEX, mon, *this, 0);
        return size_t(this->putGuard_.permits() + this->takeGuard_.permits());
    }

    template <typename T> ACE_INLINE size_t
    SemaphoreControlledChannel<T>::size(void) const
    {
        ACE_GUARD_RETURN(_mutex_type, mon, *this, 0);  return size_t(this->takeGuard_.permits());
    }

    template<typename T> int
    SemaphoreControlledChannel<T>::put(const T &t, const ACE_Time_Value *abstime)
    {
        if (this->interrupted()) {
            DAF_THROW_EXCEPTION(InterruptedException);
        }

        if (this->putGuard_.acquire(abstime) == 0) try {

            if (this->insert(t) == 0) {
                this->takeGuard_.release(); return 0;
            }

            this->putGuard_.release();
        }
        catch (...) { // JB: Deliberate catch(...) - DON'T replace with DAF_CATCH_ALL
            ACE_Errno_Guard g(errno); ACE_UNUSED_ARG(g);
            this->putGuard_.release();
            throw;
        }

        return -1;
    }

    template<typename T> T
    SemaphoreControlledChannel<T>::take(const ACE_Time_Value * abstime)
    {
        if (this->interrupted()) {
            DAF_THROW_EXCEPTION(InterruptedException);
        }

        if (this->takeGuard_.acquire(abstime)) {
            switch (DAF_OS::last_error()) {
            case ETIME: DAF_THROW_EXCEPTION(TimeoutException);
            default:    DAF_THROW_EXCEPTION(InternalException);
            }
        }

        T t = T();

        try {

            if (this->extract(t)) {
                DAF_THROW_EXCEPTION(InternalException);
            }

            this->putGuard_.release();

        } catch (...) { // JB: Deliberate catch(...) - DON'T replace with DAF_CATCH_ALL
            ACE_Errno_Guard g(errno); ACE_UNUSED_ARG(g);
            this->takeGuard_.release();
            throw;
        }

        return t;
    }

    template<typename T> int
    SemaphoreControlledChannel<T>::interrupt(void)
    {
        int result = Monitor::interrupt();
        {
            result += this->putGuard_.interrupt();
            result += this->takeGuard_.interrupt();
        }
        return result ? -1 : 0;
    }


} // namespace DAF

#endif // DAF_SEMAPHORECONTROLLEDCHANNEL_T_CPP
