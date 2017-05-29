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
#ifndef DAF_LOCKEDEXECUTOR_H
#define DAF_LOCKEDEXECUTOR_H

#include "DirectExecutor.h"

namespace DAF
{
    template <typename LOCK = ACE_SYNCH_MUTEX>

    /**
    * \class LockedExecutor
    * \brief An implementation of executor that invokes the run method
    * of the supplied command after securing the passed lock then returns.
    *
    * The LockedExecutor is useful when wanting to run multiple jobs sequentially
    * after each other. Each job will sit on the lock waiting for a chance to
    * run on the underlying executor.
    *
    * ATTRIBUTION: Doug Lee Based On 'Concurrency Patterns in Java'
    * \ingroup executor
    */
    class DAF_Export LockedExecutor : public DAF::Executor
    {
        LOCK &lock_;

        public:

            LockedExecutor(LOCK &lock)
                : lock_(lock)
            {}

            virtual int execute(const DAF::Runnable_ref &cmd)
            {
                if (!is_nil(cmd)) {
                    ACE_GUARD_RETURN( LOCK, ace_mon, this->lock_, -1 );
                    return DAF::DirectExecutor().execute(cmd);
                }
                return 0;
            }
    };
} // namespace DAF

#endif
