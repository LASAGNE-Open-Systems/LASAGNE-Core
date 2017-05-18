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
#define PERFSYNCHRONOUSCHANNEL_CPP

#include "daf/SynchronousChannel_T.h"
#include "daf/TaskExecutor.h"
#include "daf/Runnable.h"

#include "TaskChannelPerform_T.h"

#include <iostream>

typedef DAF::SynchronousChannel<int>                TestChannelType;
typedef PERF::TaskChannelPerform<TestChannelType>   TaskChannel;

namespace {

    DAF::TaskExecutor    _taskExecutor;

    class TaskChannelRunnable : public TaskChannel
    {
    public:
        DAF_DEFINE_REFCOUNTABLE(TaskChannelRunnable);
        TaskChannelRunnable(PERF::STATSData &statsData)
            : TaskChannel(statsData)
        {}

    private:

        TestChannelType testChannel_;
    };
    DAF_DECLARE_REFCOUNTABLE(TaskChannelRunnable);
}

int main(int argc, char *argv[])
{
    ACE_UNUSED_ARG(argc); ACE_UNUSED_ARG(argv);
    PERF::STATSData statsData("PerfSynchronousChannel");

    {
        TaskChannelRunnable_ref tp(new TaskChannelRunnable(statsData));

        _taskExecutor.execute(tp);

        for (TestChannelType::_value_type i = -10; i < PERF::STATSData::MAX_COUNT; i++) {
            tp->push(i);
        }
        tp->push(TestChannelType::_value_type(PERF::TaskPerform::TASK_END_SEED)); // Stops the looper
    }

    if (statsData.acquire()) {
        return -1;
    }

    ACE_ERROR_RETURN((LM_INFO, ACE_TEXT("%s: %s\n")
        , statsData.ident().c_str()
        , statsData.calculate_stats().c_str()), 0);
}
