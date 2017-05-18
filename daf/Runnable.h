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

#ifndef DAF_RUNNABLE_H
#define DAF_RUNNABLE_H

#include "RefCount.h"

namespace DAF
{
    /**
    * \struct Runnable
    * \brief A lightweight Thread Class
    *
    * Runnable provides a lightweight execution logic to be run in its own thread.
    * A developer overloads the #run() method and "executes" the Runnable on a
    * DAF::Executor to bring it to life.
    *
    * In Service level code it is recommended to bound all Runnables to a
    * DAF::TaskExecutor thereby coupling the service lifecycle with the Runnable
    * thread lifecycle. A loose guideline is runnables are short lived tasks.
    *
    * ATTRIBUTION: Based on Java Runnable
    * \ingroup executor
    */
    struct DAF_Export Runnable : virtual DAF::RefCount
    {
        DAF_DEFINE_REFCOUNTABLE(Runnable);


        virtual ~Runnable(void) { /* force destruction */ }

        /**
        Runnable Thread entry method. When passed to an DAF::Executor it will
        start processing in the run method.
        \return exit code for the thread
        */
        virtual int     run(void) = 0;

        /**
        Allows the Runnable to be run at a pre-configured thread priority.
        */
        virtual long    runPriority(void) const
        {
            return long(ACE_DEFAULT_THREAD_PRIORITY);
        }
    };

    DAF_DECLARE_REFCOUNTABLE(Runnable);

    /**
    * \struct NullRunnable
    * \brief Do Nothing Runnable
    */
    struct DAF_Export NullRunnable : virtual DAF::Runnable {
        virtual int run(void) {
            return 0; /* Do Nothing Runnable */
        }
    };
} // namespace DAF

#endif
