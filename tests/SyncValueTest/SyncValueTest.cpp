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
#include "daf/SynchValue_T.h"
#include "daf/TaskExecutor.h"
#include "daf/Runnable.h"
#include "ace/Thread.h"
#include "ace/Get_Opt.h"
#include <iostream>

namespace test
{
bool debug = false;
const char *TEST_NAME = "SynchValueTest";

typedef DAF::SynchValue<int> SynchTest_t;

//TEST
// TODO
// getValue
// waitValue
// ONE improvement would be to have a waitValue Timeout.
// ANOTHER maybe to throw Interrupted State is the SynchValue is shutting down.
// Tests on invalid initialization.
// Tests on the Operators.
//
// So How I understand this working it.
// - One waiter is looking of Value 'X'
// - ANother thread is changing 1.. X.. Y
// when the Waiter gets the Value its released and runs a riot.

// 1. Get Value
// 2. Wait on Value.
// 3. Recorder keeps the list of values.
//

struct TestSyncWaiter : DAF::Runnable
{
    int value;
    SynchTest_t &sync;
    int illegal;
    int timeout;
    time_t timey;
    int result;
    DAF::Semaphore &sema;

    TestSyncWaiter(DAF::Semaphore &sema_in, int value_in,SynchTest_t &sync_in, time_t tv = 0) : DAF::Runnable()
    , value(value_in)
    , sync(sync_in)
    , illegal(0)
    , timeout(0)
    , timey(tv)
    , result(0)
    , sema(sema_in)
    {
    }

    virtual int run(void) {
        if (debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - 0x%08X Waiting Sync for Value %d %d\n", this, value, this->sync.getValue()));

        try {
            sema.release();
            if ( timey) {
               result = this->sync.waitValue(value, timey);
            } else {
               result = this->sync.waitValue(value);
            }

            if (debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - 0x%08X Exit  Sync for   Value %d \n", this, value));
        } catch (const DAF::TimeoutException &) {
            this->timeout++;
            if (debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - 0x%08X TimeoutException \n", this));
        } catch (const DAF::IllegalThreadStateException &) {
            this->illegal++;
            if (debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - 0x%08X IllegalThreadStateException \n", this));
        }

        return 0;
    }
};

struct TestSyncWriter : TestSyncWaiter
{

    TestSyncWriter(DAF::Semaphore &sema_in, int value_in,SynchTest_t &sync_in, time_t tv = 0) : TestSyncWaiter(sema_in, value_in, sync_in, tv)
    {

    }

    virtual int run(void)
    {
    if (debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - 0x%08X Writer Sync for Value %d %d\n", this, value, this->sync.getValue()));

        try {
            sema.acquire();
            result = this->sync.setValue(value);

            if (debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - 0x%08X Exit  Sync for   Value %d \n", this, value));
        } catch (const DAF::TimeoutException &) {
            this->timeout++;
            if (debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - 0x%08X TimeoutException \n", this));
        } catch (const DAF::IllegalThreadStateException &) {
            this->illegal++;
            if (debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - 0x%08X IllegalThreadStateException \n", this));
        }

        return 0;
    }
};


/**
 * TEST
 *
 * Test for Basic Working of the SyncValue.
 */
int test_SyncValueBasicWorking(int threadCount )
{
    ACE_UNUSED_ARG(threadCount);
    int result = 1;
    const int expected = 0;
    int value = 0;
    int trigger = 3;

    SynchTest_t sync(0);
    DAF::Semaphore counter(0);
    TestSyncWaiter *waiter = new TestSyncWaiter(counter,trigger, sync);
    DAF::Runnable_ref runner(waiter);

    {
        result &= (sync.getValue() != trigger);

        DAF::TaskExecutor executor;

        executor.execute(runner);

        counter.acquire();

        sync.setValue(trigger);
    }
    value = waiter->result;

    result &= (expected == value);

    std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

    return result;
}

/**
 * TEST
 *
 * Test for Multiple waiters.
 * This is a crap test for failure... -> Deadlock.
 * But what its looking for is the setValue. gives time to the
 * waiters to check the value prior to the next setValue.
 */
int test_SyncValueMultipleWaiters(int threadCount )
{
    int count = (threadCount > 10 ? threadCount : 10);
    int result = 0;
    const int expected = 0;
    int value = 0;

    DAF::Semaphore blocker(0);
    SynchTest_t sync(0);

    {
        DAF::TaskExecutor executor;

        for ( int i = 1; i <= count ; ++i ) {
            executor.execute(new TestSyncWaiter(blocker,i, sync));
        }

        for ( int i = 1; i <= count ; ++i ) {
            blocker.acquire();
        }

        for (int i = count; i > 0 ; --i ) {
            value += sync.setValue(i);
        }

        if (debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - Value from Sync %d\n", value));
    }

    result = (expected == value);

    std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

    return result;
}

/**
 * TEST
 *
 * Test for Multiple waiters.
 * This is a crap test for failure... -> Deadlock.
 * But what its looking for is the setValue. gives time to the
 * waiters to check the value prior to the next setValue.
 *
 */
int test_SyncValueMultipleWaitersWithTimeout(int threadCount )
{
    int count = (threadCount > 10 ? threadCount : 10);
    int result = 0;
    const int expected = 1;
    int value = 0;

    DAF::Semaphore blocker(0);
    SynchTest_t sync(0);

    TestSyncWaiter *waiter = new TestSyncWaiter(blocker, 111, sync, 10);

    DAF::Runnable_ref runner(waiter);

    {
        DAF::TaskExecutor executor;

        for ( int i = 1; i <= count ; ++i ) {
            executor.execute(new TestSyncWaiter(blocker, i, sync));
        }

        executor.execute(runner);

        // Make sure everybody gets to wait.
        for ( int i = 0; i <= count ; ++i ) {
            blocker.acquire();
        }

        for (int i = count; i > count/2 ; --i ) {
            value += sync.setValue(i);
        }

        // Allow Timeout on waiter
        DAF_OS::sleep(ACE_Time_Value(0, 50000));

        for (int i = count/2; i > 0 ; --i ) {
            value += sync.setValue(i);
        }

        if (debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - Value from Sync %d\n", value));
    }

    value = waiter->timeout;

    result = (expected == value);

    std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

    return result;
}

int test_SyncValueMultipleWaitersWithTimeoutEarly(int threadCount )
{
    int count = (threadCount > 10 ? threadCount : 10);
    int result = 0;
    const int expected = 1;
    int value = 1;

    DAF::Semaphore blocker(0);
    SynchTest_t sync(0);

    {
        DAF::TaskExecutor executor;

        for ( int i = 0; i <= count ; ++i ) {
            executor.execute(new TestSyncWaiter(blocker, i, sync));
        }

        // Make sure everybody gets to wait.
        for ( int i = 0; i <= count ; ++i )
        {
            blocker.acquire();
        }

        try {
            sync.waitValue(123, 0);
        } catch (const DAF::TimeoutException &) {
           // do nothing
            //std::cout << "Timeout" << std::endl;
        }

        for (int i = count; i > count/2 ; --i ) {
            value += sync.setValue(i);
        }

        // Allow Timeout on waiter
        DAF_OS::sleep(ACE_Time_Value(0, 50000));

        for (int i = count/2; i > 0 ; --i ) {
            value += sync.setValue(i);
        }

        if (debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - Value from Sync %d\n", value));
    }

    result = (expected == value);

    std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

    return result;
}

/**
 * TEST
 *
 * Test for Destruction
 * No Deadlocks on exit
 */
int test_SyncValueDestruction(int threadCount )
{
    int count = (threadCount > 10 ? threadCount : 10);
    int result = 0;
    const int expected = 1;
    int value = 0;

    DAF::Semaphore blocker(0);
    SynchTest_t *sync = new SynchTest_t(0);

    TestSyncWaiter *waiter = new TestSyncWaiter(blocker, 111, *sync);

    DAF::Runnable_ref runner(waiter);

    {
        DAF::TaskExecutor executor;

        executor.execute(runner);

        for ( int i = 1; i <= count ; ++i ) {
            executor.execute(new TestSyncWaiter(blocker, i, *sync));
        }

        // Make sure everybody gets to wait.
        for ( int i = 0; i <= count ; ++i)
        {
            blocker.acquire();
        }

        // force the shutdown - by killing the SyncValue;
        delete sync;
    }

    // Was IllegalThreadStateException thrown ?
    value = waiter->illegal;

    result = (expected == value);

    std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

    return result;
}

/**
 * TEST
 *
 * Test for ThreadKill. does it exit cleanly
 * what behaviour do we want here.... it should break only that thread.
 * It should just make sure we have consistent state.
 */
int test_SyncValueThreadKill(int threadCount )
{
    int count = (threadCount > 10 ? threadCount : 10);
    int result = 0;
    const int expected = 111;
    int value = 0;

    DAF::Semaphore blocker(0);
    SynchTest_t sync(0);

    TestSyncWaiter *waiter = new TestSyncWaiter(blocker, 111, sync);

    DAF::Runnable_ref runner(waiter);

    {
        DAF::TaskExecutor executor;
        DAF::TaskExecutor *kill_executor = new DAF::TaskExecutor;

        executor.execute(runner);

        for ( int i = 1; i <= count ; ++i ) {
            executor.execute(new TestSyncWaiter(blocker, 111, sync));
        }

        // Make sure everybody gets to wait.
        for ( int i = 0; i <= count ; ++i)
        {
            blocker.acquire();
        }

        kill_executor->execute(new TestSyncWaiter(blocker, count+1, sync));

        blocker.acquire();

        DAF_OS::sleep(ACE_Time_Value(1));

        if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T - Killing Executor\n"));

        delete kill_executor;

        sync.setValue(expected);
    }

    value = waiter->value;

    result = (expected == value);

    std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

    return result;
}

/**
 * TEST
 *
 * Test for MultipleSetters. does it exit cleanly
 * what behaviour do we want here.... it should break only that thread.
 * It should just make sure we have consistent state.
 */
int test_SyncValueMultipleSetters(int threadCount )
{
   int count = (threadCount > 10 ? threadCount : 10);
    int result = 0;
    const int expected = 1;
    int value = 0;

    DAF::Semaphore blocker(0);
    SynchTest_t *sync = new SynchTest_t(0);

    TestSyncWaiter *waiter = new TestSyncWaiter(blocker, 111, *sync);

    DAF::Runnable_ref runner(waiter);

    {
        DAF::TaskExecutor executor;

        executor.execute(runner);

        for ( int i = 1; i < count ; ++i ) {
            executor.execute(new TestSyncWaiter(blocker, i, *sync));
        }

        // Make sure everybody gets to wait.
        for ( int i = 0; i < count ; ++i)
        {
            blocker.acquire();
        }

        // Multiple Setters
        for ( int i = 0; i < count; ++i )
        {
            executor.execute(new TestSyncWriter(blocker, count + i, *sync));
        }

        // release the setters - put one into the setting
        blocker.release(2);

        blocker.release(count);

        // force the shutdown - by killing the SyncValue;
        delete sync;
    }

    // Was IllegalThreadStateException thrown ?
    value = waiter->illegal;

    result = (expected == value);

    std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

    return result;
}


}//namespace test

void print_usage(const ACE_Get_Opt &cli_opt)
{
    ACE_UNUSED_ARG(cli_opt);
    std::cout << test::TEST_NAME
              << " -h --help              : Print this message \n"
              << " -z --debug             : Debug \n"
              << " -n --count             : Number of Threads/Test\n"
              << std::endl;
}

int main(int argc, char *argv[])
{
    int result = 1, threadCount = 10;

    ACE_Get_Opt cli_opt(argc, argv, "hzn:");
    cli_opt.long_option("help",'h', ACE_Get_Opt::NO_ARG);
    cli_opt.long_option("debug",'z', ACE_Get_Opt::NO_ARG);
    cli_opt.long_option("count",'n', ACE_Get_Opt::ARG_REQUIRED);

    for( int i = 0; i < argc; ++i ) switch(cli_opt()) {
        case -1: break;
        case 'h': print_usage(cli_opt); return 0;
        case 'z': DAF::debug(true); test::debug=true; break;
        case 'n': threadCount = DAF_OS::atoi(cli_opt.opt_arg());
    }

    std::cout << test::TEST_NAME << std::endl;
    result &= test::test_SyncValueBasicWorking(threadCount);
    result &= test::test_SyncValueMultipleWaiters(threadCount);
    result &= test::test_SyncValueMultipleWaitersWithTimeout(threadCount);
    result &= test::test_SyncValueMultipleWaitersWithTimeoutEarly(threadCount);
    result &= test::test_SyncValueDestruction(threadCount);
#ifndef ACE_WIN32
    result &= test::test_SyncValueThreadKill(threadCount);
    result &= test::test_SyncValueMultipleSetters(threadCount);
#endif

    return !result;
}
