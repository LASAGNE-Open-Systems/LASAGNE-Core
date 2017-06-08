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
#ifndef DAF_SYNCHTHREADREPOSITORY_H
#define DAF_SYNCHTHREADREPOSITORY_H

#include "OS.h"

#include <ace/Thread.h>
#include <ace/Guard_T.h>
#include <ace/Thread_Mutex.h>
#include <ace/Synch_Traits.h>

#include <map>

namespace DAF
{
    class DAF_Export SYNCHThreadRepository
    {
        typedef std::map<ACE_thread_t, SYNCHThreadRepository *> SYNCHCondition_map_type;

    public:

        SYNCHThreadRepository(void);

        virtual ~SYNCHThreadRepository(void)
        {
            /* Ensure Propper destruction */
        }

        int waiters(void) const
        {
            return this->waiters_;
        }

    protected:

        int inc_waiters(const ACE_thread_t & = ACE_Thread::self());
        int dec_waiters(const ACE_thread_t & = ACE_Thread::self());

    private:

        int waiters_;

        static class DAF_Export SYNCHConditionRepository : SYNCHCondition_map_type
        {
            mutable ACE_SYNCH_MUTEX lock_;

        public:

            int _insert(const key_type & thr_id, const mapped_type & val);
            int _remove(const key_type & thr_id);

            operator ACE_SYNCH_MUTEX & () const
            {
                return this->lock_;
            }

        } condition_repo_;

        friend int DAF_OS::thread_SYNCHTerminate(const ACE_thread_t &);

    private:

        ACE_UNIMPLEMENTED_FUNC(void operator= (const SYNCHThreadRepository &));
        ACE_UNIMPLEMENTED_FUNC(SYNCHThreadRepository(const SYNCHThreadRepository &));
    };

    inline
    SYNCHThreadRepository::SYNCHThreadRepository(void) : waiters_(0)
    {
    }

}   // namespace DAF

#endif // DAF_SYNCHTHREADREPOSITORY_H
