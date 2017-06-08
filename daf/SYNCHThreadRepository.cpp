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
#define DAF_SYNCHTHREADREPOSITORY_CPP

#include "SYNCHThreadRepository.h"

namespace DAF_OS
{
    int
    thread_SYNCHTerminate(const ACE_thread_t &thr_id)
    {
        return DAF::SYNCHThreadRepository::condition_repo_._remove(thr_id);
    }
}

namespace DAF
{
    SYNCHThreadRepository::SYNCHConditionRepository SYNCHThreadRepository::condition_repo_;

    int
    SYNCHThreadRepository::SYNCHConditionRepository::_insert(const key_type & thr_id, const mapped_type & val)
    {
        ACE_GUARD_RETURN(ACE_SYNCH_MUTEX, mon, *this, (DAF_OS::last_error(ENOLCK), -1));
        try {
            ++((*this)[thr_id] = val)->waiters_; return 0;
        } catch (const std::exception &) {
            // Something went wrong
        }
        return -1;
    }

    int
    SYNCHThreadRepository::SYNCHConditionRepository::_remove(const key_type & thr_id)
    {
        ACE_GUARD_RETURN(ACE_SYNCH_MUTEX, mon, *this, (DAF_OS::last_error(ENOLCK), -1));
        try {
            --this->at(thr_id)->waiters_; this->erase(thr_id); return 0;
        } catch (const std::out_of_range &) {
            return 0; // Not Found - OK
        } catch (const std::exception &) {
            // Something went wrong
        }
        return -1;
    }

    /************************************************************************************************************/

    int
    SYNCHThreadRepository::inc_waiters(const ACE_thread_t &thr_id)
    {
        if (thr_id) {
            SYNCHThreadRepository::condition_repo_._insert(thr_id, this);
        }
        return this->waiters();
    }

    int
    SYNCHThreadRepository::dec_waiters(const ACE_thread_t &thr_id)
    {
        if (thr_id) {
            SYNCHThreadRepository::condition_repo_._remove(thr_id);
        }
        return this->waiters();
    }

}   // namespace DAF
