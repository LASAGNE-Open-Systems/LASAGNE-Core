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
#define DAF_TERMINATEREPOSITORY_CPP

#include "TerminateRepository.h"

#include <ace/Thread_Mutex.h>
#include <ace/Synch_Traits.h>
#include <ace/Singleton.h>

#include <map>
#include <typeinfo>

#if defined (DAF_HAS_WAIT_FOR_TERMINATE_WTHREAD) && (DAF_HAS_WAIT_FOR_TERMINATE_WTHREAD > 0)

namespace { // Anonymous

    struct TerminateEvent : ACE_Event {
        TerminateEvent(void) : ACE_Event(true, false) {}
    };

    class TerminateRepository : std::map<ACE_thread_t, TerminateEvent>
    {
    public:

        typedef ACE_SYNCH_RW_MUTEX  _mutex_type;

        static TerminateRepository * instance(void);

        int         insertTerminateEvent(ACE_thread_t);
        int         removeTerminateEvent(ACE_thread_t);
        int         signalTerminateEvent(ACE_thread_t);

        ACE_HANDLE  locateTerminateEvent(ACE_thread_t) const;

        operator _mutex_type & () const
        {
            return this->lock_;
        }

        const ACE_TCHAR * dll_name(void) const
        {
            return DAF_DLL_NAME;
        }

        const ACE_TCHAR * name(void) const
        {
            return typeid(*this).name();
        }

    private:

        mutable _mutex_type  lock_;
    };

    TerminateRepository *
    TerminateRepository::instance(void)
    {
        return ACE_DLL_Singleton_T<TerminateRepository, ACE_SYNCH_MUTEX>::instance();
    }

    int
    TerminateRepository::insertTerminateEvent(ACE_thread_t thr_id)
    {
        ACE_WRITE_GUARD_RETURN(_mutex_type, guard, *this, -1); return ((*this)[thr_id].handle()) ? 0 : (this->erase(thr_id), -1);
    }

    int
    TerminateRepository::removeTerminateEvent(ACE_thread_t thr_id)
    {
        ACE_WRITE_GUARD_RETURN(_mutex_type, guard, *this, -1); return this->erase(thr_id) ? 0 : -1;
    }

    int
    TerminateRepository::signalTerminateEvent(ACE_thread_t thr_id)
    {
        ACE_READ_GUARD_RETURN(_mutex_type, guard, *this, -1);

        try {
            return this->at(thr_id).signal();
        }
        catch (const std::out_of_range &) {
            DAF_OS::last_error(ENOENT);
        }

        return -1;
    }

    ACE_HANDLE
    TerminateRepository::locateTerminateEvent(ACE_thread_t thr_id) const
    {
        ACE_READ_GUARD_RETURN(_mutex_type, guard, *this, ACE_INVALID_HANDLE);

        try {
            return this->at(thr_id).handle();
        }
        catch (const std::out_of_range &) {
            DAF_OS::last_error(ENOENT);
        }

        return ACE_INVALID_HANDLE;
    }

} // Anonymous

int
DAF_OS::insertTerminateEvent(ACE_thread_t thr_id)
{
    return TerminateRepository::instance()->insertTerminateEvent(thr_id);
}

int
DAF_OS::removeTerminateEvent(ACE_thread_t thr_id)
{
    return TerminateRepository::instance()->removeTerminateEvent(thr_id);
}

int
DAF_OS::signalTerminateEvent(ACE_thread_t thr_id)
{
    return TerminateRepository::instance()->signalTerminateEvent(thr_id);
}

ACE_HANDLE
DAF_OS::locateTerminateEvent(ACE_thread_t thr_id)
{
    return TerminateRepository::instance()->locateTerminateEvent(thr_id);
}

int
DAF_OS::sleep(const ACE_Time_Value & tv)
{
#if defined(ACE_WIN32)
    for (ACE_HANDLE hTerminate = DAF_OS::locateTerminateEvent(DAF_OS::thr_self()); hTerminate != ACE_INVALID_HANDLE;)
    {
        switch (::WaitForSingleObject(hTerminate, DWORD(tv.msec())))
        {
        case WAIT_TIMEOUT:  return 0;

        case WAIT_OBJECT_0:
#if defined(DAF_HAS_ABI_FORCED_UNWIND_EXCEPTION) && (DAF_HAS_ABI_FORCED_UNWIND_EXCEPTION > 0)
            throw ::abi::__forced_unwind();
#else
            DAF_OS::last_error(EINTR); break;
#endif

        default: DAF_OS::last_error(::GetLastError()); break;
        }

        return -1;
    }
#endif

    return ACE_OS::sleep(tv);
}

#endif // defined (DAF_HAS_WAIT_FOR_TERMINATE_WTHREAD) && (DAF_HAS_WAIT_FOR_TERMINATE_WTHREAD > 0)
