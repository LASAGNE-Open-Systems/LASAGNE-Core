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
#ifndef PERF_TASKCHANNEL_T_H
#define PERF_TASKCHANNEL_T_H

#include "STATSData.h"

#include "ace/High_Res_Timer.h"
#include "ace/Semaphore.h"

#include "TaskPerform.h"

namespace PERF
{
    template <typename T>
    class TaskChannelPerform : public PERF::TaskPerform
    {

    public:

        TaskChannelPerform(PERF::STATSData &statsData) : TaskPerform(statsData) {}

        virtual int push(const typename T::_value_type &);

    protected:

        virtual int run(void);

    protected:

        T channel_;
    };

    template <typename T> int
    TaskChannelPerform<T>::push(const typename T::_value_type &s)
    {
        if (this->statsLock_.acquire() == 0) {
            this->statsTimer_.start(); return this->channel_.put(s);
        }
        return -1;
    }

    template <typename T> int
    TaskChannelPerform<T>::run(void)
    {
        for (;;) try {

            int i = this->channel_.take();

            this->statsTimer_.stop();

            ACE_hrtime_t nsecs; this->statsTimer_.elapsed_time(nsecs);

            this->statsTimer_.reset();

            double usecs = double(nsecs / 1000.0);

            if (i == int(TASK_END_SEED)) {
                throw "Seed-End-Received";
            }
            else if (0 > i) {
                ACE_DEBUG((LM_INFO, ACE_TEXT("(%P | %t)\t%s[%02d]: usec=%4.4f\n")
                    , this->statsData_.ident().c_str()
                    , DAF_OS::abs(i), usecs)); std::cout << std::flush;
            }
            else this->statsData_[i] = usecs;

            this->statsLock_.release();

        } DAF_CATCH_ALL{
            this->statsLock_.release(); break;
        }

        return 0;
    }

} // namespace PERF

#endif // PERF_TASKCHANNEL_T_H
