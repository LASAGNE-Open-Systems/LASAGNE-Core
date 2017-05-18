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
#define PERFTASKEXECUTOR_CPP

#include "daf/TaskExecutor.h"

#include "TaskPerform.h"

namespace {
    DAF::TaskExecutor _taskExecutor;
}

namespace PERF
{
    class TaskExecutorPerform : public PERF::TaskPerform
    {
    public:
        TaskExecutorPerform(PERF::STATSData &statsData)
            : PERF::TaskPerform(statsData)
        {
            this->statsTimer_.start();
        }

    protected:

        virtual int run(void);
    };

    int
    TaskExecutorPerform::run(void)
    {
        this->statsTimer_.stop();

        ACE_hrtime_t nsecs; this->statsTimer_.elapsed_time(nsecs);

        this->statsTimer_.reset();

        double usecs = double(nsecs / 1000.0);

        if (0 > this->statsCount_) {
            ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t)\t%s[%02d]: usec=%4.4f\n")
                , this->statsData_.ident().c_str()
                , DAF_OS::abs(this->statsCount_++)
                , usecs
                ));
        }
        else try {
            this->statsData_[this->statsCount_++] = usecs;
        }
        catch (...) {
            return 0; // Test Finished
        }

        this->statsTimer_.start(); return _taskExecutor.execute(*this);
    }
} // namespace PERF

int main(int argc, char *argv[])
{
    ACE_UNUSED_ARG(argc); ACE_UNUSED_ARG(argv);
    PERF::STATSData statsData("PerfTaskExecutor");

    if (_taskExecutor.execute(new PERF::TaskExecutorPerform(statsData))) {
        return -1;
    } else if (statsData.acquire()) {
        return -1;
    }

    ACE_ERROR_RETURN((LM_INFO, ACE_TEXT("%s: poolsize=%u,%s\n")
        , statsData.ident().c_str()
        , _taskExecutor.size()
        , statsData.calculate_stats().c_str()), 0);
}
