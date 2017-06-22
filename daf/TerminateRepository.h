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
#ifndef DAF_TERMINATEREPOSITORY_H
#define DAF_TERMINATEREPOSITORY_H

#include "OS.h"

#include <ace/Event.h>

namespace DAF_OS
{
#if defined (DAF_HAS_WAIT_FOR_TERMINATE_WTHREAD) && (DAF_HAS_WAIT_FOR_TERMINATE_WTHREAD > 0)

    int         insertTerminateEvent(ACE_thread_t thr_id);
    int         removeTerminateEvent(ACE_thread_t thr_id);
    int         signalTerminateEvent(ACE_thread_t thr_id);
    ACE_HANDLE  locateTerminateEvent(ACE_thread_t thr_id);

#else

    ACE_INLINE int
    insertTerminateEvent(ACE_thread_t thr_id)
    {
        ACE_UNUSED_ARG(thr_id); ACE_NOTSUP_RETURN(-1);
    }

    ACE_INLINE int
    removeTerminateEvent(ACE_thread_t thr_id)
    {
        ACE_UNUSED_ARG(thr_id); ACE_NOTSUP_RETURN(-1);
    }

    ACE_INLINE int
    signalTerminateEvent(ACE_thread_t thr_id)
    {
        ACE_UNUSED_ARG(thr_id); ACE_NOTSUP_RETURN(-1);
    }

    ACE_INLINE ACE_HANDLE
    locateTerminateEvent(ACE_thread_t thr_id)
    {
        ACE_UNUSED_ARG(thr_id); ACE_NOTSUP_RETURN(ACE_INVALID_HANDLE);
    }

#endif
}  // namespace DAF_OS

#endif // DAF_TERMINATEREPOSITORY_H
