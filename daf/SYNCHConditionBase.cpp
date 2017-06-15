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
#define DAF_SYNCHCONDITIONBASE_CPP

#include "SYNCHConditionBase.h"

#include "Exception.h"

namespace DAF
{
#if defined(ACE_WIN32)

    SYNCHConditionBase::SYNCHConditionRepository    SYNCHConditionBase::condition_repo_;

    int
    SYNCHConditionBase::SYNCHConditionRepository::_insert(const key_type & thr_id, const mapped_type & base)
    {
        ACE_GUARD_REACTION(ACE_SYNCH_MUTEX, guard, *this, DAF_THROW_EXCEPTION(DAF::LockFailureException));
        return ++((*this)[thr_id] = base)->waiters_;
    }

    int
    SYNCHConditionBase::SYNCHConditionRepository::_remove(const key_type & thr_id)
    {
        ACE_GUARD_REACTION(ACE_SYNCH_MUTEX, guard, *this, DAF_THROW_EXCEPTION(DAF::LockFailureException));
        int waiters = --this->at(thr_id)->waiters_;
        this->erase(thr_id);
        return waiters;
    }

    int threadSYNCHTerminate(const ACE_thread_t & thr_id)
    {
        try {
            DAF::SYNCHConditionBase::condition_repo_._remove(thr_id); return 0;
        } catch (const std::out_of_range &) {
            return 0; // Not found
        } catch (const std::exception &) {
            // Something went wrong 
        }
        return -1;
    }

#endif

    /*********************************************************************************/

    int
    SYNCHConditionBase::inc_waiters(void)
    {
#if defined(ACE_WIN32)
        try {
            return SYNCHConditionBase::condition_repo_._insert(DAF_OS::thr_self(), this);
        } catch (const std::exception &) {
            // Something went wrong??
        }
#endif
        return ++this->waiters_;
    }

    int
    SYNCHConditionBase::dec_waiters(void)
    {
#if defined(ACE_WIN32)
        try {
            return SYNCHConditionBase::condition_repo_._remove(DAF_OS::thr_self());
        } catch (const std::exception &) {
            // Something went wrong??
        }
#endif
        return --this->waiters_;
    }

}   // namespace DAF
