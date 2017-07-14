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
#ifndef DAF_SEMAPHORECONTROLLEDQUEUE_T_CPP
#define DAF_SEMAPHORECONTROLLEDQUEUE_T_CPP

#include "SemaphoreControlledQueue_T.h"

namespace DAF
{
    template <typename T>
    SemaphoreControlledQueue<T>::SemaphoreControlledQueue(size_t capacity)
        : SemaphoreControlledChannel<T>(capacity), channelQ_(capacity)
    {
    }

    template <typename T> int
    SemaphoreControlledQueue<T>::insert(const T &t)
    {
        ACE_GUARD_REACTION(ACE_SYNCH_MUTEX, guard, *this, DAF_THROW_EXCEPTION(ResourceExhaustionException));
        this->channelQ_.push_back(t);
        return 0;
    }

    template <typename T> T
    SemaphoreControlledQueue<T>::extract(void)
    {
        ACE_GUARD_REACTION(ACE_SYNCH_MUTEX, guard, *this, DAF_THROW_EXCEPTION(ResourceExhaustionException));
        T t(this->channelQ_.front());
        this->channelQ_.pop_front();
        return t;
    }
} // namespace DAF

#endif // DAF_SEMAPHORECONTROLLEDQUEUE_T_CPP
