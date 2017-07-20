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
#define DAF_SEMAPHORE_CPP

/**********************  Semaphore *****************************
* Conceptually, a semaphore maintains a set of permits.
* Each acquire() blocks if necessary
* until a permit is available, and then takes it.
* Each release adds a permit. However, no actual permit objects
* are used; the Semaphore just keeps a count of the number
* available and acts accordingly.
* <p>
* A semaphore seed initialized to 1 can serve as a mutual
* exclusion lock. Negative seeds are also allowed, in which
* case no acquires will proceed until the number of releases
* has pushed the number of permits past 0.
* <p>
* Different implementation subclasses may provide different
* ordering guarantees (or lack thereof) surrounding which
* threads will be resumed upon a signal.
* <p>
* The default implementation makes NO
* guarantees about the order in which threads will
* acquire permits. It is often faster than other implementations.
*************************************************************/

#include "Semaphore.h"

namespace DAF
{
    int
    Semaphore::permits(void) const
    {
        return this->permits_;  // Keep this out-of-line
    }

    int
    Semaphore::acquire(const ACE_Time_Value * abstime)
    {
        while (!this->interrupted()) {

            ACE_GUARD_REACTION(_mutex_type, guard, *this, DAF_THROW_EXCEPTION(LockFailureException));

            if (this->permits() > this->waiters()) { // Preference any waiters
                --this->permits_; return 0;
            }
            else if (this->wait(abstime)) {
                switch (this->interrupted() ? EINTR : DAF_OS::last_error()) {
                case EINTR: continue;  // Retry interrupted test
                case ETIME:
                    if (this->permits() > 0) {
                        --this->permits_; return 0;
                    }

                    // Fall through

                default: return -1; // Report as errno error
                }
            }
            else if (this->permits() > 0) {
                --this->permits_; return 0;
            }
        }

        DAF_THROW_EXCEPTION(InterruptedException);
    }

    int
    Semaphore::release(int permits)
    {
        int result = 0;

        if (permits > 0) {

            ACE_GUARD_RETURN(_mutex_type, guard, *this, (DAF_OS::last_error(ENOLCK),-1));

            while (permits-- > 0) {
                ++this->permits_; if (this->signal()) {
                    result = -1;
                }
            }
        }

        return result;
    }

} // namespace DAF
