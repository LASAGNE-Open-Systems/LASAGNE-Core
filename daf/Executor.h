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
#ifndef DAF_EXECUTOR_H
#define DAF_EXECUTOR_H

#include "DAF.h"
#include "Runnable.h"

namespace DAF
{
    /**
      \defgroup executor Thread Execution Management

      This group contains one of the core modules of DAF. It deals with the
      control and execution of threads in a controlled manner.
    */

    /**
    * \class Executor
    * \brief Abstract Executor handoff pattern for dynamic thread management
    *
    * This Executor abstract pattern allows for the conformity of all
    *           Executor interfacing strategies
    * \ingroup executor
    */
    class DAF_Export Executor : ACE_Copy_Disabled
    {
    public:

        virtual ~Executor(void) {}

        /**
        * Execute the given command. This method is only guaranteed only
        * to arrange for execution, that may only occur sometime later;
        * for example in a new thread.  However, in a fully generic use,
        * callers should be prepared for execution to occur in any
        * fashion at all, including immediate direct execution.  The
        * execute method is defined not to throw any exceptions during
        * execution of the command.  Generally any problems encountered
        * will be asynchronous and so must be dealt with via callbacks
        * or error handler objects.
        * @param cmd Executes the provided runnable.
        */
        virtual int     execute(const DAF::Runnable_ref&) = 0;

        /**
        * The number of Runnables under the control of the Executor.
        * In the case of some derived <code>Executor</code> classes such as a
        * LockedExecutor, it will return zero.
        */
        virtual size_t  size(void) const
        {
            return size_t(0);
        }
    };
} // namespace DAF

#endif
