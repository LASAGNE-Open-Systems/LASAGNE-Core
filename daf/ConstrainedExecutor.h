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
#ifndef DAF_CONSTRAINEDEXECUTOR_H
#define DAF_CONSTRAINEDEXECUTOR_H

#include "Executor.h"
#include "Semaphore.h"

namespace DAF
{
    /** @class ConstrainedExecutor
    * @brief ConstrainedExecutor encapsulates an Executor-derived class instance and constrains
    * that executor instance to only allowing a maximum number of threads to be run at any
    * one time.
    *
    * The ConstrainedExecutor is configured with this number on creation. Attempts
    * to run more than the maximum number of threads will block until currently running
    * threads exit from the encapsulated executor. Note that the ConstrainedExecutor does not
    * start its own threads, but uses the threads (and threading model) provided by the
    * encapsulated executor.
    *
    * ATTRIBUTION: Doug Lee Based On 'Concurrency Patterns in Java'
    * \ingroup executor
    */
    class DAF_Export ConstrainedExecutor : public Executor
    {
        DAF::Semaphore  semaphore_;
        DAF::Executor   &executor_;
        size_t          permits_;

        public:
            /**
            Construction of ConstrainedExecutor requires an existing executor
            manager.
            \param executor the execution manager to constrain.
            \param permits the number of concurrent jobs to constrain.
            */
            ConstrainedExecutor(Executor &executor, size_t permits = 1);

            virtual int     execute(const Runnable_ref &command);

            /// Remaining number of permitted concurrent jobs
            virtual size_t  available(void) const;
            /// Total number of permitted concurrent jobs
            virtual size_t  size(void) const;
    };
} // namespace DAF

#endif //  DAF_CONSTRAINEDEXECUTOR_H
