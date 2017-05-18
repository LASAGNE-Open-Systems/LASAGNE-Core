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
#define PERF_MONITOR_CPP

#include "daf/Monitor.h"
#include "daf/Runnable.h"
#include "daf/TaskExecutor.h"

#include "TaskPerform.h"

namespace {

    class TaskMonitor : public PERF::TaskPerform
    {
        DAF::Monitor    monitor_;
        volatile int    val_;

    public:

        DAF_DEFINE_REFCOUNTABLE(TaskMonitor);

        TaskMonitor(PERF::STATSData &statsData)
            : PERF::TaskPerform(statsData), val_(TASK_END_SEED)
        {
            this->statsLock_.acquire();
        }

        virtual int run(void);

        virtual int push(int i);
    };

    DAF_DECLARE_REFCOUNTABLE(TaskMonitor);

    int
    TaskMonitor::push(int i)
    {
        if (this->statsLock_.acquire() == 0) {
            ACE_GUARD_REACTION(ACE_SYNCH_MUTEX, mon, this->monitor_, return -1);
            this->val_ = i;
            this->statsTimer_.start();
            return this->monitor_.notify();
        }
        return -1;
    }

    int
    TaskMonitor::run(void)
    {
        ACE_GUARD_REACTION(ACE_SYNCH_MUTEX, mon, this->monitor_, return -1);
        for (int i = TASK_END_SEED;;) {
            ACE_hrtime_t nsecs;
            {
                this->statsLock_.release();
                if (this->monitor_.wait() || i == this->val_) {
                    continue;
                }
                this->statsTimer_.stop();
                this->statsTimer_.elapsed_time(nsecs);

                i = this->val_;
            }

            double usecs = double(nsecs / 1000.0);

            this->statsTimer_.reset();

            if (0 > i) {
                ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t)\t%s[%02d]: usec=%4.4f\n")
                    , this->statsData_.ident().c_str()
                    , DAF_OS::abs(i), usecs)); std::cout << std::flush;
            } else try {
                this->statsData_[i] = usecs;
            } DAF_CATCH_ALL{
                break;
            }
        }

        this->statsLock_.release(); return 0;
    }

}

int main(int argc, char *argv[])
{
    ACE_UNUSED_ARG(argc); ACE_UNUSED_ARG(argv);
    PERF::STATSData statsData("PerfMonitor");

    {

        TaskMonitor_ref tm(new TaskMonitor(statsData));

        DAF::SingletonExecute(tm);

        for (int i = -10; i < PERF::STATSData::MAX_COUNT; i++) {
            tm->push(i);
        }
        tm->push(int(PERF::TaskPerform::TASK_END_SEED)); // Stops the looper

    }

    if (statsData.acquire()) {
        return -1;
    }

    ACE_ERROR_RETURN((LM_INFO, ACE_TEXT("%s: %s\n")
        , statsData.ident().c_str()
        , statsData.calculate_stats().c_str()), 0);
}
