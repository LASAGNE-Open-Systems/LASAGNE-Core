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
#ifndef DAF_SEMAPHORECONTROLLEDPRIORITYCHANNEL_T_CPP
#define DAF_SEMAPHORECONTROLLEDPRIORITYCHANNEL_T_CPP

#include "SemaphoreControlledPriorityChannel_T.h"

namespace DAF
{
    template < typename T, typename P > inline
    SemaphoreControlledPriorityChannel<T,P>::SemaphoreControlledPriorityChannel(size_t capacity)
        : SemaphoreControlledChannel<T>(capacity), channelQ_(capacity)
    {
    }

    template < typename T, typename P > inline int
    SemaphoreControlledPriorityChannel<T,P>::insert(const T &t)
    {
        ACE_GUARD_RETURN(_mutex_type, guard, *this, -1);
        this->channelQ_.push(t);
        return 0;
    }

    template < typename T, typename P > inline int
    SemaphoreControlledPriorityChannel<T,P>::extract(T &t)
    {
        ACE_GUARD_RETURN(_mutex_type, guard, *this, -1);
        t = this->channelQ_.top();
        this->channelQ_.pop();
        return 0;
    }

} // namespace DAF

#endif // DAF_SEMAPHORECONTROLLEDPRIORITYCHANNEL_T_CPP
