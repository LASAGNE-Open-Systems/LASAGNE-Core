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

#include "daf/TaskExecutor.h"
#include <ace/Semaphore.h>

#include <iostream>

namespace
{
    ACE_Semaphore   test_lock(1); // Unlocked.

    DAF::Monitor    monitor_;

    class MonitorTest : DAF::TaskExecutor
    {
        struct MonitorRunnable : DAF::Runnable
        {
            virtual int run(void);
        };

    public:

        MonitorTest(void);
        ~MonitorTest(void);

        virtual int init(int argc, ACE_TCHAR *argv[]);

    private:

        virtual int svc(void);
    };

    MonitorTest::MonitorTest(void) : DAF::TaskExecutor()
    {
        test_lock.acquire();
    }

    MonitorTest::~MonitorTest(void)
    {
        this->module_closed(); this->wait();
    }

    int
    MonitorTest::init(int argc, ACE_TCHAR *argv[])
    {
        ACE_UNUSED_ARG(argc); ACE_UNUSED_ARG(argv);

        if (this->execute(new MonitorRunnable()) == 0) {
            DAF_OS::sleep(1); return this->execute(1);
        }
        return -1;
    }

    int
    MonitorTest::svc(void)
    {
//        DAF_OS::thread_0_SIGSET_T(); // Ignore all signals to avoid ERROR:

        ACE_DEBUG((LM_INFO, ACE_TEXT("%P | %t) - MonitorTest svc() Start\n")));

        try {
            ACE_GUARD_RETURN(ACE_SYNCH_MUTEX, mon, monitor_, -1);
            ACE_DEBUG((LM_INFO, ACE_TEXT("%P | %t) - MonitorTest svc() remove\n")));

            monitor_.interrupt();
        }
        catch (...) {
            ACE_ERROR_RETURN((LM_INFO, ACE_TEXT("%P | %t) - Monitor svc() Exception\n")), -1);
        }

        return 0;
    }

    int
    MonitorTest::MonitorRunnable::run(void)
    {
        ACE_DEBUG((LM_INFO, ACE_TEXT("%P | %t) - MonitorRunnable Start\n")));

        try {
            ACE_GUARD_RETURN(ACE_SYNCH_MUTEX, mon, monitor_, -1);
            int i = monitor_.wait(), j = DAF_OS::last_error();
            if (i) {
                ACE_ERROR_RETURN((LM_INFO, ACE_TEXT("%P | %t) - MonitorTest return =%d;errno=%d"), i, j), -1);
            }
        }
        catch (...) {
            ACE_DEBUG((LM_INFO, ACE_TEXT("%P | %t) - MonitorRunnable Exception\n")));
        }

        test_lock.release();

        ACE_ERROR_RETURN((LM_INFO, ACE_TEXT("%P | %t) - MonitorRunnable End\n")), 0);
    }

}

int main(int argc, char* argv[])
{
    try {
        MonitorTest mon_test; mon_test.init(argc, argv); test_lock.acquire();
    }
    catch (...) {
        ACE_ERROR_RETURN((LM_INFO, ACE_TEXT("%P | %t) - Main Exception\n")), -1);
    }

    return 0;
}
