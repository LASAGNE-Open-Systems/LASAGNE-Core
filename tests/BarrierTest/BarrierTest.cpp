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
#include "daf/Barrier.h"
#include "daf/TaskExecutor.h"
#include "ace/Thread.h"
#include "ace/Get_Opt.h"
#include <iostream>

namespace test
{
bool debug = false;
const char *TEST_NAME = "BarrierTest";

struct TestBarrier : public DAF::Runnable
{
    DAF::Barrier &barrier_;
    int broken_;
    int timeout_;
    int illegal_;
    int unknown_;
    time_t msecs_;
    DAF::Semaphore &sema_;

    TestBarrier(DAF::Semaphore &sema_in, DAF::Barrier &barrier, time_t msecs = 0) : DAF::Runnable()
    , barrier_(barrier)
    , broken_(0)
    , timeout_(0)
    , illegal_(0)
    , unknown_(0)
    , msecs_(msecs)
    , sema_(sema_in)
    {
    }

    virtual int run(void) {
        if ( debug) ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) %T - 0x%08X Entering Barrier\n"), this));

        int barrierCount = 0;
        try {
            sema_.release();
            if (this->msecs_ == 0 )
                barrierCount = this->barrier_.barrier();
            else
                barrierCount = this->barrier_.barrier(this->msecs_);
        } catch ( const DAF::BrokenBarrierException &e) {
            this->broken_++;
            if (debug) ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X Broken on Barrier %s %d\n"), this,  e.what(), this->broken_));
        } catch ( const DAF::TimeoutException &te ) {
            this->timeout_++;
            if (debug) ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X Timeout on Barrier %s\n"), this, te.what()));
        } catch ( const DAF::IllegalThreadStateException &te ) {
            // This occurs when the PooledExecutor dies
            // and our underlying thread bombs out.
            this->illegal_++;
            if(debug) ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X IllegalThreadState on Barrier %s\n"), this, te.what()));
        } catch ( const std::exception & se) {
            if (debug) ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X std::exception %s\n"), this, se.what()));
        } DAF_CATCH_ALL {
            this->unknown_++;
            if(debug) ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X Unknown Exception on Barrier\n"), this));
        }

        if ( debug) ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) %T - 0x%08X Barrier Count %d\n"), this, barrierCount));
        return 0;
    }
};

int count;
struct TestTrigger : DAF::Runnable
{
    //int count;
    TestTrigger() : DAF::Runnable()
    //, count(0)
    {}

    virtual int run(void)
    {
        count++;
        if ( debug) ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) %T - 0x%08X Trigger Count %d\n"), this, count));
        return 0;
    }
};

/**
 * TEST
 *
 * No Barrier Command, will the barrier still function with when a
 * BarrierCommand has not been set.
 */
int test_NoBarrierCommand(int threadCount )
{
    int result = 1;
    int expected = 0;
    int value = 0;

    DAF::Barrier barrier(threadCount + 1);
    DAF::Semaphore counter(0);

    {
        count = 0;
        DAF::TaskExecutor executor;

        for ( int i = 0 ; i < threadCount; ++i )
        {
            executor.execute(new TestBarrier(counter, barrier));
        }

        const int barrierResult = barrier.barrier();
        ACE_UNUSED_ARG(barrierResult);

        DAF_OS::thr_yield();

        value = count;
    }

    result &= (expected == value);

    std::cout << __FUNCTION__ <<  " Expected " << 0 << " result " << count << " " << (result ? "OK" : "FAILED" ) << std::endl;

    return result;
}

/**
 * TEST
 * With Barrier Command does the Barrier Command get called ?
 */
int test_BarrierCommand(int threadCount)
{
    int result = 0;
    const int expected = 1;
    int value = 0;

    DAF::Barrier barrier(threadCount + 1);
    DAF::Semaphore counter(0);

    {
        DAF::TaskExecutor executor;

        count = 0;
        barrier.setBarrierCommand(new TestTrigger());

        for ( int i = 0 ; i < threadCount; ++i )
        {
            executor.execute(new TestBarrier(counter, barrier));
        }

        const int barrierResult = barrier.barrier();
        ACE_UNUSED_ARG(barrierResult);

        DAF_OS::thr_yield();

        value = count ;
    }

    result = (value == expected);

    std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

    return result;
}

/**
 * TEST
 *
 * Check the BrokenBarrierException gets called
 */
int test_BarrierBrokenException(int threadCount)
{
    ACE_UNUSED_ARG(threadCount);
    int result = 0;
    int expected = 1;
    int value = 0;

    DAF::Semaphore counter(0);

    {
        DAF::Barrier barrier(3);
        //DAF::Runnable_ref broken = new TestBarrier(barrier);
        DAF::Runnable_ref breaker = new TestBarrier(counter,barrier, 500);

        DAF::TaskExecutor executor;

        //executor.execute(broken);
        executor.execute(breaker);

        // Give enough time for the breaker to leave.
        try {
            barrier.barrier();
        } catch (const DAF::BrokenBarrierException & ) {
            value++;
        }

        DAF_OS::sleep(1);
        // Not sure why this value is not correct ? -> 0 ? coz the exception is thrown
        //value = reinterpret_cast<TestBarrier*>(broken.ptr())->broken_ ;
        //if (debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - 0x%08X Broken Count %d\n", broken.ptr(), value));
    }

    result = (value == expected);

    std::cout << __FUNCTION__ <<  "Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

    return result;
}

/**
 * TEST
 *
 * Check the IllegalException gets called
 * this is hard to force as the Barrier needs to go out of scope
 * so we are using a pointer and deleting while waiting.
 */
int test_BarrierIllegalStateException(int threadCount)
{
    ACE_UNUSED_ARG(threadCount);

    int result = 0;
    const int expected = 1;
    int value = 0;

    DAF::Semaphore blocker(0);

    DAF::Barrier *barrier = new DAF::Barrier(2);
    DAF::Runnable_ref illegal = new TestBarrier(blocker, *barrier);

    // Got the Scope on the executor to force an IllegalState

    {
        DAF::TaskExecutor executor;

        executor.execute(illegal);

        blocker.acquire();
        DAF_OS::sleep(1);

        delete barrier;
        DAF_OS::sleep(1);
    }

    value = reinterpret_cast<TestBarrier*>(illegal.ptr())->illegal_;

    result = (value == expected);

    std::cout << __FUNCTION__ <<  "Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

    return result;
}

/**
 * TEST
 *
 * Check the TimeoutException gets called
 */
int test_BarrierTimeoutException(int threadCount )
{
    int result = 0;
    int expected = threadCount;
    int value = 0;

    DAF::Semaphore counter(0);

    {
        DAF::TaskExecutor executor;
        DAF::Barrier barrier(2);
        DAF::Runnable_ref timeout = new TestBarrier(counter, barrier, 100);

        for (int i = 0 ; i < threadCount; ++i)
        {
            executor.execute(timeout);
            DAF_OS::sleep(ACE_Time_Value(0, 500000));
        }

        value = reinterpret_cast<TestBarrier*>(timeout.ptr())->timeout_;
    }
    result = (value == expected);
    std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

    return result;
}

/**
 * TEST
 *
 * Does waitReset work.
 */
int test_BarrierWaitResetClean(int threadCount)
{
    ACE_UNUSED_ARG(threadCount);

    int result = 1;
    const int expected = 0;
    int value = 0;
    DAF::Semaphore blocker(0);
    DAF::Barrier barrier(2);

    {
        DAF::TaskExecutor executor;

        DAF::Runnable_ref broken = new TestBarrier(blocker, barrier);

       // for ( int i = 0 ; i < threadCount; ++i )
        {
            executor.execute(broken);
            executor.execute(broken);

            blocker.acquire();
            blocker.acquire();

            DAF_OS::sleep(ACE_Time_Value(0, 5000));

            bool clean = barrier.waitReset();
            result &= !barrier.broken() && clean;
        }

        value = reinterpret_cast<TestBarrier*>(broken.ptr())->broken_  +
                reinterpret_cast<TestBarrier*>(broken.ptr())->unknown_ +
                reinterpret_cast<TestBarrier*>(broken.ptr())->illegal_  ;
    }

    result &= (value == expected);

    std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

    return result;
}

/**
 * TEST
 *
 * BarrierWaitReset with Timeout
 * There is a problem with waitReset when nobody has entered the
 * Barrier, waitReset will return true. But then if
 * somebody enters after...
 */
int test_BarrierWaitResetTimeout(int threadCount)
{
    std::cout << __FUNCTION__;

    ACE_UNUSED_ARG(threadCount);
    int result = 1;
    const int expected = 1;
    int value = 0;
    bool clean = false;

    DAF::Semaphore counter(0);
    DAF::Barrier barrier(threadCount+1);
    TestBarrier *tester = new TestBarrier(counter, barrier);
    DAF::Runnable_ref broken(tester);

    {
        DAF::TaskExecutor executor;

        for ( int i = 1 ; i < threadCount; ++i )
        {
            executor.execute(new TestBarrier(counter, barrier));
        }

        clean = barrier.waitReset(1000);

        result &= barrier.broken() && !clean;

        if (debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - Clean Wait %s %d\n", (clean ? "YES" : "NO"), result));
        DAF_OS::sleep(ACE_Time_Value(1));
    }

    value = tester->broken_  + tester->unknown_ + tester->illegal_  ;

    result &= (value == expected);

    std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

    return result;
}

/**
 * TEST
 *
 * Barrier Construction of Zero Expecting a throw InitializationException
 * similar kindof design decision to the Rendezvous. Upon init of 0 parties
 * we could
 * 1. Throw InitializationException
 * 2. change 0 -> 1.
 * 3. Throw an exception on rendezvous entry ie IllegalState
 *
 * Currently adopting Option 2.
 */
int test_BarrierCtorZero(int threadCount)
{
    ACE_UNUSED_ARG(threadCount);
    int result = 0;
    const int expected = 1;
    int value = 0;

    try {
        DAF::Barrier barrier(0);

        barrier.barrier(100);
    } catch ( const DAF::InitializationException& ) {
        value = 1;
    } catch ( const DAF::TimeoutException &) {
        value = 2;
    }

    result = (value == expected);

    std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

    return result;
}

/**
 * TEST
 *
 * Barrier_ThreadKill
 * Testing the is resilient to threads being abruptly interrupted and killed.
 */
int test_BarrierThreadKill(int threadCount)
{
    std::cout << __FUNCTION__;

    ACE_UNUSED_ARG(threadCount);
    int result = 1;
    const int expected = 1;
    int value = 0;
    bool clean = false;

    DAF::Semaphore counter(0);
    DAF::Barrier barrier(threadCount+2); // threadCount + tester + thread_killer
    TestBarrier *tester = new TestBarrier(counter, barrier);
    DAF::Runnable_ref broken(tester);

    {
        DAF::TaskExecutor executor;
        DAF::TaskExecutor *kill_executor = new DAF::TaskExecutor;

        executor.execute(broken);

        for ( int i = 2 ; i < threadCount; ++i )
        {
            executor.execute(new TestBarrier(counter, barrier));
        }



        kill_executor->execute(new TestBarrier(counter, barrier));

        for ( int i = 0; i < threadCount; ++i )
        {
            counter.acquire();
        }

        // Kill the TaskExecutor - This should lead to a hard thread kill
        // Can the Barrier recover?
        if (debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - Killing the TaskExecutor\n"));
        delete kill_executor;

        DAF_OS::sleep(ACE_Time_Value(1,0));

        //result &= barrier.broken();

        if (debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - Clean Wait %s %d\n", (clean ? "YES" : "NO"), result));
        DAF_OS::sleep(ACE_Time_Value(1));
    }

    value = tester->broken_  + tester->unknown_ + tester->illegal_  ;

    result &= (value == expected);

    std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

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
    int result = 1, threadCount = 2;

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

    result &= test::test_BarrierCommand(threadCount);
    result &= test::test_NoBarrierCommand(threadCount);
    result &= test::test_BarrierWaitResetClean(threadCount);
    //result &= test::test_BarrierWaitResetTimeout(threadCount);
    result &= test::test_BarrierBrokenException(threadCount);
    result &= test::test_BarrierTimeoutException(threadCount);
    result &= test::test_BarrierIllegalStateException(threadCount);
    result &= test::test_BarrierCtorZero(threadCount);
#ifndef ACE_WIN32
    result &= test::test_BarrierThreadKill(threadCount);
#endif

    return !result;
}
