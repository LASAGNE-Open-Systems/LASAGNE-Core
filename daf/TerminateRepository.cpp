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

#if defined (DAF_HAS_WAIT_FOR_TERMINATE_WTHREAD) && (DAF_HAS_WAIT_FOR_TERMINATE_WTHREAD > 0)

#include <ace/Thread_Mutex.h>
#include <ace/Synch_Traits.h>
#include <ace/Singleton.h>
#include <ace/Event.h>

#include <map>
#include <typeinfo>

namespace { // Anonymous

    struct TerminateEvent : ACE_Event {
        TerminateEvent(void) : ACE_Event(true, false) {}
    };

    class TerminateRepository : std::map<ACE_thread_t, TerminateEvent>
    {
        ACE_UNIMPLEMENTED_FUNC(void operator = (const TerminateRepository &))
        ACE_UNIMPLEMENTED_FUNC(TerminateRepository(const TerminateRepository &))

    public:

        typedef ACE_DLL_Singleton_T<TerminateRepository, ACE_SYNCH_MUTEX>   _singleton_type;

        typedef ACE_SYNCH_RW_MUTEX  _mutex_type;

        TerminateRepository(void)
        {
        }

        virtual ~TerminateRepository(void)
        {
            TerminateRepository::repo_instance_ = 0;
        }

        static TerminateRepository * instance(void);

        int         insertTerminateEvent(ACE_thread_t);
        int         removeTerminateEvent(ACE_thread_t);
        int         signalTerminateEvent(ACE_thread_t);

        ACE_HANDLE  locateTerminateEvent(ACE_thread_t) const;

        operator _mutex_type & () const
        {
            return this->repo_lock_;
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

        mutable _mutex_type  repo_lock_;

    private:

        static TerminateRepository * repo_instance_;
    };

    TerminateRepository * TerminateRepository::repo_instance_ = 0;

    TerminateRepository *
    TerminateRepository::instance(void)
    {
        // Stops multiple AFR registrations warnings in ACE (go-figure) ???
        do {
            static ACE_Thread_Mutex singleton_lock; // Stops thread Race Condition
            if (TerminateRepository::repo_instance_ == 0) {
                ACE_GUARD_REACTION(ACE_Thread_Mutex, guard, singleton_lock, break);
                if (TerminateRepository::repo_instance_ == 0) { // DCL
                    TerminateRepository::repo_instance_ = TerminateRepository::_singleton_type::instance();
                }
            }
        } while (false);

        return TerminateRepository::repo_instance_;
    }

    int
    TerminateRepository::insertTerminateEvent(ACE_thread_t thr_id)
    {
        ACE_WRITE_GUARD_RETURN(_mutex_type, guard, *this, -1);

        if ((*this)[thr_id].handle()) {
            return 0;
        }

        this->erase(thr_id); return -1;
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
DAF_OS::cond_timedwait(ACE_cond_t * cv, ACE_thread_mutex_t * external_mutex, ACE_Time_Value * timeout)
{
    {  // Prevent race conditions - try for lock on the <waiters_> count.
        bool waiters_locked = (ACE_OS::thread_mutex_lock(&cv->waiters_lock_) == 0);
        ++cv->waiters_; // Update waiters
        if (waiters_locked) {
            ACE_OS::thread_mutex_unlock(&cv->waiters_lock_);
        }
    }

    // We keep the lock held just long enough to increment the count of
    // waiters by one.  Note that we can't keep it held across the call
    // to WaitForSingleObject since that will deadlock other calls to
    // ACE_OS::cond_signal().

    if (ACE_OS::thread_mutex_unlock(external_mutex)) {
        // Just reverse the waiters (unlocked!!)
        --cv->waiters_; return -1;
    }

    int result = 0, error = 0, msec_timeout = 0;

    if (timeout) {
        if (*timeout != ACE_Time_Value::zero) {
            // Note that we must convert between absolute time (which is
            // passed as a parameter) and relative time (which is what
            // WaitForSingleObjects() expects).
            for (ACE_Time_Value relative_time = timeout->to_relative_time(); relative_time > ACE_Time_Value::zero;) {
                // Watchout for situations where a context switch has caused thecurrent time to be > the timeout.
                msec_timeout = relative_time.msec(); break;
            }
        }
    } else {
        msec_timeout = INFINITE;
    }

    ACE_HANDLE events[2] = { cv->sema_, ACE_INVALID_HANDLE };

    int event_count = 1;

    if ((events[1] = DAF_OS::locateTerminateEvent(DAF_OS::thr_self())) != ACE_INVALID_HANDLE) {
        ++event_count;
    }

    result = ::WaitForMultipleObjects(event_count, events, false, msec_timeout);

    bool last_waiter = false;

    {  // Prevent race conditions on the <waiters_> count.
        bool waiters_locked = (ACE_OS::thread_mutex_lock(&cv->waiters_lock_) == 0);

        if (--cv->waiters_ == 0) {
            last_waiter = (cv->was_broadcast_ ? true : false);
        }

        if (waiters_locked) {
            ACE_OS::thread_mutex_unlock(&cv->waiters_lock_);
        }
    }

    const int WAIT_TERMINATE = int(WAIT_OBJECT_0 + 1);

    if (result != int(WAIT_OBJECT_0)) {
        switch (result) {
        case WAIT_TIMEOUT:  error = ETIME; break;
        case WAIT_TERMINATE:error = EINTR; break;
        default:            error = ::GetLastError(); break;
        }
        result = -1;
    }

    if (last_waiter) { // Release the signaler/broadcaster if we're the last waiter.
        if (ACE_OS::event_signal(&cv->waiters_done_)) {
            result = -1; // Return an Error indication
        }
    }

    // We must always regain the <external_mutex>, even when errors
    // occur because that's the guarantee that we give to our callers.
    if (ACE_OS::thread_mutex_lock(external_mutex)) {
        result = -1; // Return an Error indication
    }

    if (error) switch (DAF_OS::last_error(error)) {
#  if defined(DAF_HAS_ABI_FORCED_UNWIND_EXCEPTION) && (DAF_HAS_ABI_FORCED_UNWIND_EXCEPTION > 0)
    case EINTR: throw ::abi::__forced_unwind();
#  endif
    }

    return result;
}

int
DAF_OS::sleep(const ACE_Time_Value & tv)
{
    for (ACE_HANDLE hEvent = DAF_OS::locateTerminateEvent(DAF_OS::thr_self()); hEvent != ACE_INVALID_HANDLE;) {
        switch (::WaitForSingleObject(hEvent, DWORD(tv.msec()))) {
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

    return ACE_OS::sleep(tv);
}

#endif // defined (DAF_HAS_WAIT_FOR_TERMINATE_WTHREAD) && (DAF_HAS_WAIT_FOR_TERMINATE_WTHREAD > 0)

int
DAF_OS::insertTerminateEvent(ACE_thread_t thr_id)
{
#if defined (DAF_HAS_WAIT_FOR_TERMINATE_WTHREAD) && (DAF_HAS_WAIT_FOR_TERMINATE_WTHREAD > 0)
    return TerminateRepository::instance()->insertTerminateEvent(thr_id);
#else
    ACE_UNUSED_ARG(thr_id); ACE_NOTSUP_RETURN(-1);
#endif
}

int
DAF_OS::removeTerminateEvent(ACE_thread_t thr_id)
{
#if defined (DAF_HAS_WAIT_FOR_TERMINATE_WTHREAD) && (DAF_HAS_WAIT_FOR_TERMINATE_WTHREAD > 0)
    return TerminateRepository::instance()->removeTerminateEvent(thr_id);
#else
    ACE_UNUSED_ARG(thr_id); ACE_NOTSUP_RETURN(-1);
#endif
}

int
DAF_OS::signalTerminateEvent(ACE_thread_t thr_id)
{
#if defined (DAF_HAS_WAIT_FOR_TERMINATE_WTHREAD) && (DAF_HAS_WAIT_FOR_TERMINATE_WTHREAD > 0)
    return TerminateRepository::instance()->signalTerminateEvent(thr_id);
#else
    ACE_UNUSED_ARG(thr_id); ACE_NOTSUP_RETURN(-1);
#endif
}

ACE_HANDLE
DAF_OS::locateTerminateEvent(ACE_thread_t thr_id)
{
#if defined (DAF_HAS_WAIT_FOR_TERMINATE_WTHREAD) && (DAF_HAS_WAIT_FOR_TERMINATE_WTHREAD > 0)
    return TerminateRepository::instance()->locateTerminateEvent(thr_id);
#else
    ACE_UNUSED_ARG(thr_id); ACE_NOTSUP_RETURN(ACE_INVALID_HANDLE);
#endif
}

