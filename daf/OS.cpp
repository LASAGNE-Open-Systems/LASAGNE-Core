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

}  // namespace DAF_OS
