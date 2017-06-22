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
#define DAF_OS_CPP

#include "OS.h"

#include "TerminateRepository.h"

#include <ace/Thread_Mutex.h>
#include <ace/Synch_Traits.h>
#include <ace/Singleton.h>

#include <typeinfo>
#include <map>

namespace DAF_OS
{
    int     last_error(void)
    {
        int error = ACE_OS::last_error();

#if defined (ACE_WIN32)

        if (error) do {

            // Remap some windows error messages as appropriate

            switch (error) {
            case ERROR_NOT_ENOUGH_MEMORY:   error = ENOMEM; break;
            case ERROR_FILE_EXISTS:         error = EEXIST; break;
            case ERROR_SHARING_VIOLATION:   error = EACCES; break;
            case ERROR_PATH_NOT_FOUND:      error = ENOENT; break;
            case ERROR_ACCESS_DENIED:       error = EPERM;  break;
            case ERROR_SEM_TIMEOUT:         error = ETIME;  break;
            case ERROR_TIMEOUT:             error = ETIME;  break;
            default: continue;
            }

            ACE_OS::last_error(error);

        } while (false);
#endif

        return error;
    }

    ACE_hthread_t   thread_HANDLE(void)
    {
        ACE_hthread_t thread_h; ACE_OS::thr_self(thread_h); return thread_h;
    }

    long            thread_PRIORITY(ACE_hthread_t ht_id)
    {
        int priority; if (ACE_OS::thr_getprio(ht_id, priority)) {
            return long(ACE_DEFAULT_THREAD_PRIORITY);
        }
        return long(priority);
    }

    const std::string & gethostname(void)
    {
        static const struct _HostName : std::string {
            _HostName(void) {
                char host_name[MAXHOSTNAMELEN]; *host_name = 0; do {
                    if (ACE_OS::hostname(host_name, sizeof(host_name))) {
                        break;
                    } else if (ACE_OS::strlen(host_name) > 0) {
                        this->assign(host_name);
                    }
                } while(false);
            }
        } host_name_; return host_name_;
    }

    ACE_Time_Value  gettimeofday(time_t msec)
    {
        ACE_Time_Value c_time(DAF_OS::gettimeofday()); if (msec) {
            const ACE_Time_Value offset(time_t(msec / 1000), suseconds_t((msec % 1000) * 1000));
            if (msec > time_t(0)) {
                c_time += offset;
            } else {
                c_time -= offset;
            }
        }
        return c_time;
    }

    int     thread_0_SIGSET_T(void)
    {
        static const struct _SSIG {
            sigset_t set_;
            _SSIG(void) {
                ACE_OS::sigfillset(&this->set_);
            }
        } SSIG_;

        return ACE_OS::thr_sigsetmask(SIG_SETMASK, &SSIG_.set_, 0);
    }

#if defined(DAF_HAS_WAIT_FOR_TERMINATE_WTHREAD) && (DAF_HAS_WAIT_FOR_TERMINATE_WTHREAD > 0)

    int cond_timedwait(ACE_cond_t * cv, ACE_thread_mutex_t * external_mutex, ACE_Time_Value * timeout)
    {
        do {  // Prevent race conditions on the <waiters_> count.

            if (ACE_OS::thread_mutex_lock(&cv->waiters_lock_) == 0) {
                ++cv->waiters_;
                if (ACE_OS::thread_mutex_unlock(&cv->waiters_lock_) == 0) {
                    break;
                }
            }

            return -1;

        } while (false);

        // We keep the lock held just long enough to increment the count of
        // waiters by one.  Note that we can't keep it held across the call
        // to WaitForSingleObject since that will deadlock other calls to
        // ACE_OS::cond_signal().

        if (ACE_OS::thread_mutex_unlock(external_mutex)) {
            return -1;
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
        }
        else {
            msec_timeout = INFINITE;
        }

        ACE_HANDLE events[2] = { cv->sema_, ACE_INVALID_HANDLE };

        int event_count = 1;

        if ((events[1] = DAF_OS::locateTerminateEvent(DAF_OS::thr_self())) != ACE_INVALID_HANDLE) {
            ++event_count;
        }

        result = ::WaitForMultipleObjects(event_count, events, false, msec_timeout);

        bool last_waiter = false; 

        do {  // Prevent race conditions on the <waiters_> count.

            if (ACE_OS::thread_mutex_lock(&cv->waiters_lock_) == 0) {

                if (--cv->waiters_ == 0) {
                    last_waiter = (cv->was_broadcast_ ? true : false);
                }

                if (ACE_OS::thread_mutex_unlock(&cv->waiters_lock_) == 0) {
                    break;
                }
            }

            return -1;

        } while (false);

        const int WAIT_TERMINATE = int(WAIT_OBJECT_0 + 1);

        if (result != int(WAIT_OBJECT_0)) {
            switch (result) {
            case WAIT_TIMEOUT:  error = ETIME; break;
            case WAIT_TERMINATE:error = EINTR; break;
            default:            error = ::GetLastError(); break;
            }
            result = -1;
        }

        if (last_waiter) {
            // Release the signaler/broadcaster if we're the last waiter.
            if (ACE_OS::event_signal(&cv->waiters_done_)) {
                return -1;
            }
        }

        // We must always regain the <external_mutex>, even when errors
        // occur because that's the guarantee that we give to our callers.
        if (ACE_OS::thread_mutex_lock(external_mutex)) {
            result = -1;
        }

        if (error) switch (DAF_OS::last_error(error)) {
#  if defined(DAF_HAS_ABI_FORCED_UNWIND_EXCEPTION) && (DAF_HAS_ABI_FORCED_UNWIND_EXCEPTION > 0)
        case EINTR: throw ::abi::__forced_unwind();
#  endif
        }

        return result;
    }
#endif // defined(DAF_HAS_WAIT_FOR_TERMINATE_WTHREAD) && (DAF_HAS_WAIT_FOR_TERMINATE_WTHREAD > 0)

}  // namespace DAF_OS
