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
#ifndef DAF_SYNCHCONDITIONBASE_H
#define DAF_SYNCHCONDITIONBASE_H

#include "OS.h"

#include <ace/Guard_T.h>
#include <ace/Thread_Mutex.h>
#include <ace/Synch_Traits.h>

#include <map>

namespace DAF
{
    class DAF_Export SYNCHConditionBase
    {
    public:

        SYNCHConditionBase(void);

        virtual ~SYNCHConditionBase(void)
        {
            /* Ensure Propper destruction */
        }

        int waiters(void) const;

    protected:

        int inc_waiters(void);
        int dec_waiters(void);

    private:

        volatile int waiters_;

    private:

        typedef std::map<ACE_thread_t, SYNCHConditionBase *>    SYNCHCondition_map_type;

        static class SYNCHConditionRepository : SYNCHCondition_map_type
        {
            mutable ACE_SYNCH_MUTEX lock_;

        public:

            int _insert(const key_type & thr_id, const mapped_type & base);
            int _remove(const key_type & thr_id);

            operator ACE_SYNCH_MUTEX & () const
            {
                return this->lock_;
            }

        } condition_repo_;

#if defined(ACE_WIN32)
        friend int threadSYNCHTerminate(const ACE_thread_t &);
#endif

    private:

        ACE_UNIMPLEMENTED_FUNC(void operator = (const SYNCHConditionBase &));
        ACE_UNIMPLEMENTED_FUNC(SYNCHConditionBase(const SYNCHConditionBase &));
    };

    inline
    SYNCHConditionBase::SYNCHConditionBase(void) : waiters_(0)
    {
    }

}   // namespace DAF

#endif // DAF_SYNCHCONDITIONBASE_H
