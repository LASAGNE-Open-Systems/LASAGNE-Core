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
#define SEMAPHORECHANNELTEST_CPP

/*
    Simple Semaphore Channel Test
*/

#include "daf/DAF.h"
#include "daf/SemaphoreControlledQueue_T.h"
#include "daf/SemaphoreControlledPriorityChannel_T.h"

#include "daf/TaskExecutor.h"

#include <iostream>

const size_t ChannelCapacity = 100;

namespace test
{
    class TestValue {
        int val_;
    public:
        TestValue(int val = 0) : val_(val) {}
        int getValue(void) const {
            return this->val_;
        }
        int operator < (const TestValue &val) const {
            return this->getValue() < val.getValue();
        }
    };

    class ChannelTesterBase  : protected DAF::TaskExecutor
    {
    public:

        virtual ~ChannelTesterBase(void)
        {
            this->module_closed(); this->wait();
        }

        virtual DAF::Channel<TestValue> & testChannel(void) = 0;

        int channelTest(void)
        {
            for(int i = 0; i < 10; i++) {
                this->testChannel().put(TestValue(i));
            }
            DAF_OS::sleep(2); return 0;
        }

    protected:

        virtual int svc(void);
    };

    int
    ChannelTesterBase::svc(void)
    {
        DAF_OS::thread_0_SIGSET_T(); // Ignore all signals to avoid ERROR:

        while (this->isAvailable()) try {
            ACE_DEBUG((LM_INFO, ACE_TEXT("TestValue=%d\n"), this->testChannel().poll(time_t(100)).getValue()));
        } catch (const DAF::TimeoutException &) {
            continue;
        } DAF_CATCH_ALL{
            break;
        }

        return 0;
    }

    class QueueChannelTester : public ChannelTesterBase
    {
        DAF::SemaphoreControlledQueue<TestValue>    queue_;

        virtual DAF::Channel<TestValue>&  testChannel(void)
        {
            return this->queue_;
        }

    public:
        QueueChannelTester(size_t capacity) : queue_(capacity)
        {
            this->execute(1);
        }

        virtual ~QueueChannelTester(void)
        {
            this->module_closed(); this->wait();
        }
    };

    class PriorityChannelTester : public ChannelTesterBase
    {
        DAF::SemaphoreControlledPriorityChannel<TestValue>  queue_;

        virtual DAF::Channel<TestValue>&  testChannel(void)
        {
            return this->queue_;
        }

    public:
        PriorityChannelTester(size_t capacity) : queue_(capacity)
        {
            this->execute(1);
        }

        virtual ~PriorityChannelTester(void)
        {
            this->module_closed(); this->wait();
        }
    };

} // namespace test

int main(int argc, char *argv[])
{
    ACE_UNUSED_ARG(argc); ACE_UNUSED_ARG(argv);

    ACE_DEBUG((LM_INFO, ACE_TEXT("TEST DAF::SemaphoreControlledQueue<> (Result 0-9).\n")));
    if (test::QueueChannelTester(ChannelCapacity).channelTest()) {
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: QueueChannelTester Failed.\n")), -1);
    }

    ACE_DEBUG((LM_INFO, ACE_TEXT("TEST DAF::SemaphoreControlledPriorityChannel<> (Result 9-0).\n")));
    if (test::PriorityChannelTester(ChannelCapacity).channelTest()) {
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: PriorityChannelTester Failed.\n")), -1);
    }

    return 0;
}
