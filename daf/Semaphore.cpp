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
*
*(c)Copyright 2011,
*   Defence Science and Technology Organisation,
*   Department of Defence,
*   Australia.
*
* All rights reserved.
*
* This is unpublished proprietary source code of DSTO.
* The copyright notice above does not evidence any actual or
* intended publication of such source code.
*
* The contents of this file must not be disclosed to third
* parties, copied or duplicated in any form, in whole or in
* part, without the express prior written permission of DSTO.
*
*
* @file     Semaphore.cpp
* @author   Derek Dominish
* @author   $LastChangedBy$
* @date     1st September 2011
* @version  $Revision$
* @ingroup
*
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
* <p>
* NOTE: Cant Use ACE_Semaphore in many cases as Solaris does not
* support the timed wait semantics, which is important in many
* instances.
*************************************************************/

#include "Semaphore.h"

namespace DAF
{
    Semaphore::Semaphore(int permits)
        : permits_(permits)
    {
    }

    int Semaphore::acquire(void)
    {
        ACE_GUARD_REACTION(ACE_SYNCH_MUTEX, guard, *this, DAF_THROW_EXCEPTION(ResourceExhaustionException));

        if (this->interrupted()) {
            DAF_OS::last_error(EINTR); DAF_THROW_EXCEPTION(DAF::InterruptedException);
        } else if (this->permits_ > this->waiters()) {
            --this->permits_; return 0;
        }

        for (;;) {
            this->wait();
            if (this->permits_ > 0) {
                --this->permits_; return 0;
            }
        }

        return 0;
    }

    int Semaphore::attempt(time_t msecs)
    {
        ACE_GUARD_REACTION(ACE_SYNCH_MUTEX, guard, *this, DAF_THROW_EXCEPTION(ResourceExhaustionException));

        if (this->interrupted()) {
            DAF_OS::last_error(EINTR); DAF_THROW_EXCEPTION(DAF::InterruptedException);
        } else if (this->permits_ > this->waiters()) {
            --this->permits_; return 0;
        }

        if (msecs > time_t(0)) {

            const ACE_Time_Value end_time(DAF_OS::gettimeofday(msecs));

            do {
                this->wait(end_time);
                if (this->permits_ > 0) {
                    --this->permits_; return 0;
                }
            } while (end_time > DAF_OS::gettimeofday());
        }

        DAF_OS::last_error(ETIME); return -1;
    }

    int Semaphore::release(void)
    {
        ACE_GUARD_REACTION(ACE_SYNCH_MUTEX, guard, *this, DAF_THROW_EXCEPTION(ResourceExhaustionException));
        ++this->permits_;
        return this->notify();
    }

    int Semaphore::release(int n)
    {
        while (n-- > 0) {
            this->release(); // Always call our release (i.e. it may be overloaded)
        }
        return 0;
    }
} // namespace DAF
