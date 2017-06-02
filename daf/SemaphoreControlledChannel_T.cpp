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
    template <typename T>
    SemaphoreControlledChannel<T>::SemaphoreControlledChannel(size_t capacity)
        : putGuard_(int(capacity)), takeGuard_(0)
    {
    }

    template <typename T> size_t
    SemaphoreControlledChannel<T>::capacity(void) const
    {
        return size_t(this->putGuard_.permits() + this->takeGuard_.permits());
    }

    template <typename T> size_t
    SemaphoreControlledChannel<T>::size(void) const
    {
        return size_t(this->takeGuard_.permits());
    }

    template<typename T> int
    SemaphoreControlledChannel<T>::put(const T &t)
    {
        if (this->putGuard_.acquire() == 0) try {
            if (this->insert(t) == 0) {
                this->takeGuard_.release(); return 0;
            }
        } catch(...) { // JB: Deliberate catch(...) - DON'T replace with DAF_CATCH_ALL
             this->putGuard_.release(); throw;
        }
        return -1;
    }

    template<typename T> int
    SemaphoreControlledChannel<T>::offer(const T &t, time_t msecs)
    {
        if (this->putGuard_.attempt(msecs) == 0) try {
            if (this->insert(t) == 0) {
                this->takeGuard_.release(); return 0;
            }
        } catch(...) { // JB: Deliberate catch(...) - DON'T replace with DAF_CATCH_ALL
            this->putGuard_.release(); throw;
        }
        return -1;
    }

    template<typename T> T
    SemaphoreControlledChannel<T>::take(void)
    {
        this->takeGuard_.acquire();

        try {

            T t(this->extract()); this->putGuard_.release(); return t;

        } catch(...) { // JB: Deliberate catch(...) - DON'T replace with DAF_CATCH_ALL
            // Error condition - Thread Kill or similar - Attempt to reclaim the resource
            ACE_Errno_Guard g(errno); ACE_UNUSED_ARG(g);
            this->takeGuard_.release();
            throw;
        }
    }

    template<typename T> T
    SemaphoreControlledChannel<T>::poll(time_t msecs)
    {
        if (this->takeGuard_.attempt(msecs)) switch (DAF_OS::last_error()) {
            case ETIME: DAF_THROW_EXCEPTION(DAF::TimeoutException); // Reverse the fact that we have been here and exit with error
            default:    DAF_THROW_EXCEPTION(DAF::InternalException);
        }

        try {

            T t(this->extract()); this->putGuard_.release(); return t;

        } catch(...) { // JB: Deliberate catch(...) - DON'T replace with DAF_CATCH_ALL
            // Error condition - Thread Kill or similar - Attempt to reclaim the resource
            ACE_Errno_Guard g(errno); ACE_UNUSED_ARG(g);
            this->takeGuard_.release();
            throw;
        }
    }

    template<typename T> int
    SemaphoreControlledChannel<T>::interrupt(void)
    {
        return this->putGuard_.interrupt() + this->takeGuard_.interrupt() ? -1 : 0;
    }

} // namespace DAF

#endif // DAF_SEMAPHORECONTROLLEDCHANNEL_T_CPP
