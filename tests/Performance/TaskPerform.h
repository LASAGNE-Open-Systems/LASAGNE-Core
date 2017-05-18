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
#ifndef PERF_TASKPERFORM_H
#define PERF_TASKPERFORM_H

#include "daf/Runnable.h"
#include "daf/Semaphore.h"

#include "ace/High_Res_Timer.h"
#include "ace/Semaphore.h"

#include "STATSData.h"

namespace PERF
{
    class TaskPerform : public DAF::Runnable
    {
    public:
        DAF_DEFINE_REFCOUNTABLE(TaskPerform);

        enum {
            TASK_END_SEED = 19570826
        };

        TaskPerform(PERF::STATSData &statsData);
        ~TaskPerform(void);

    protected:

        virtual long    runPriority(void) const
        {
            return long(ACE_DEFAULT_THREAD_PRIORITY + 1);
        }

        PERF::STATSData &   statsData_;
        ACE_High_Res_Timer  statsTimer_;
        DAF::Semaphore      statsLock_;
        int                 statsCount_;
    };

    DAF_DECLARE_REFCOUNTABLE(TaskPerform);

    inline TaskPerform::TaskPerform(PERF::STATSData &statsData)
        : statsData_(statsData), statsLock_(1), statsCount_(-10)
    {
        this->statsTimer_.reset();
    }

    inline TaskPerform::~TaskPerform(void)
    {
        this->statsData_.release();
    }

} // namespace PERF

#endif // PERF_TASKPERFORM_H
