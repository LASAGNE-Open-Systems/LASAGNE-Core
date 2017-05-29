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
#include "daf/FutureResult_T.h"
#include "daf/Barrier.h"
#include "daf/TaskExecutor.h"
#include "daf/DirectExecutor.h"
#include "ace/Thread.h"
#include "ace/Get_Opt.h"
#include <iostream>
#include <vector>
#include <functional>

/**
 *
 * Want to refactor the FutureResult to allow use of a
 * - std::unary_function to FunctionObject
 * - removal of Runnable_ref and allow explicit setting ?
 * - Is there a race if more than one Runnable is setting the value?
 */

namespace test
{
bool debug = false;
const char* TEST_NAME = "FutureResultTest";

typedef DAF::FutureResult<int> FutureInt_t;

int FutureFuncCalculator(void *op)
{
    int *in1 = reinterpret_cast<int*>(op);
    if (debug) ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) %T - Func Value %d %d %d 0x%08X\n"),*in1, in1[0],in1[1], op));

    return *in1+1;
}

struct TestFunctor : DAF::FutureFunctor<int> {
    int value;

    TestFunctor(int val_in = 10) : value(val_in)
    {
    }

    int operator()(const int& input)
    {
        return value + input ;
    }
};

struct TestThrowFunctor : DAF::FutureFunctor<int> {
    int operator()(const int& )
    {
        throw DAF::IllegalThreadStateException();
    }
};


struct TestFutureGet : DAF::Runnable
{
    int result;
    FutureInt_t &future;
    int invoke;
    int timeout;
    time_t timey;
    DAF::Semaphore &sema;
    DAF::Semaphore *sema_post;

    TestFutureGet(DAF::Semaphore &sema_in, FutureInt_t &in, time_t tv = 0, DAF::Semaphore *sema_post_in = 0): DAF::Runnable()
    , result(0)
    , future(in)
    , invoke(0)
    , timeout(0)
    , timey(tv)
    , sema(sema_in)
    , sema_post(sema_post_in)
    {
    }

    virtual int run(void)
    {
        if (debug) ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) %T - 0x%08X Get Future 0x%08X ... \n"),this  , &future));

        try {
            sema.release();
            if ( timey ) {
                result = this->future.get(timey);
            } else {
                result = this->future.get();
            }
            if ( sema_post) sema_post->release();

            if (debug) ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) %T - 0x%08X Got Future 0x%08X ... \n"),this, &future));
        } catch( const DAF::InvocationTargetException &) {
            invoke++;
            if (debug) ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) %T - 0x%08X InvocationException 0x%08X ... \n"),this, &future));
        } catch( const DAF::TimeoutException &) {
            timeout++;
            if (debug) ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) %T - 0x%08X TimeoutException 0x%08X ... \n"),this, &future));
        }

        return 0;
    }
};

struct TestFutureBind : DAF::Runnable
{
    FutureInt_t &in_;
    int value;

    TestFutureBind(FutureInt_t &in) : in_(in), value(1)
    {
    }

    int testBind(void *op)
    {
        int *i = reinterpret_cast<int*>(op);
        if(debug) ACE_DEBUG((LM_INFO, "Bind Function Called 0x%08 %d 0x%08X\n", this, *i, op));
        return *i + 2;
    }

    virtual int run(void)
    {
        if (debug) ACE_DEBUG((LM_INFO, "Running 0x%08X %d 0x%08X\n", this, value, &value));

        // Want to see FutureResult changed to include either
        // std::bind compatibilty or Functor/std::unary_function support.
        // This would make it much more power, with being able to
        // bind objects to the call. TODO

        //DAF::SingletonExecute(in_(std::bind1st(TestFutureBind::testBind, this), &value));

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
        if (debug) ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) %T - 0x%08X Trigger Count %d\n"), this, count));
        return 0;
    }
};


/**
 * TEST
 *
 * Functor version
 */
int test_FutureResultBasicFunctor(int threadCount)
{
    int result = 0;
    int expected  = threadCount;
    int value = 0;

    {
        DAF::TaskExecutor executor;

        test::FutureInt_t future;
        TestFunctor theFunctor(expected-1);

        executor.execute(future(theFunctor,1));

        //executor.execute(future(i));

        value = future.get();

        if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T Functor Future Value  %d\n", value));
    }

    result = (value == expected) ;
    std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

    return result;
}

/**
 * TEST
 *
 * Basic Setter. Using the _default_setter.
 */
int test_FutureResultBasicSetter(int threadCount)
{
    int result = 0;
    int expected  = threadCount;
    int value = 0;

    {
        DAF::TaskExecutor executor;

        test::FutureInt_t future;
        TestFunctor theFunctor(expected-1);

        executor.execute(future(expected));

        //executor.execute(future(i));

        value = future.get();

        if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T Setter Future Value  %d\n", value));
    }

    result = (value == expected) ;
    std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

    return result;
}

/**
 * TEST
 *
 *  Does peek make any sense. It only makes sense when FutureResult has been used for multiple
 *  times. and subsequent reads will have the ready flag to true.
 */
int test_FutureResultPeek(int threadCount)
{
    int result = 0;
    int expected  = threadCount;
    int value = 0;

    {
        DAF::DirectExecutor direct;

        FutureInt_t future;

        direct.execute(future(expected));

        value = future.peek();
    }

    result = (value == expected) ;
    std::cout << __FUNCTION__ << " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

    return result;
}

/**
 * TEST
 *
 * Test the FutureResult. Reset functionality.
 * Some design decisions. Should we just clear the status flags ?
 * ie Current Waiters will wait, New Entrants will wait
 * Should we clear out old waiters ?
 * ie Current Waiters get cleared, New Entrants only  ?
 */
int test_FutureResultReset(int threadCount)
{
    ACE_UNUSED_ARG(threadCount);
    int result = 0;
    int expected  = 1;
    int value = 0;

    DAF::Semaphore blocker(0);
    DAF::Semaphore blocker_post(0);

    const time_t dummy = 0;
    FutureInt_t future;
    TestFutureGet *getter = new TestFutureGet(blocker, future, dummy, &blocker_post);
    DAF::Runnable_ref runner(getter);

    {
        DAF::TaskExecutor executor;

        executor.execute(runner);

        blocker.acquire();

        executor.execute(future(expected+5));
        blocker_post.acquire();

        value = getter->result;

        future.reset(); // ready flags reset

        executor.execute(runner);

        blocker.acquire();

        executor.execute(future(expected));

        blocker_post.acquire();

        value = getter->result;
    }

    result = (value == expected) ;
    std::cout << __FUNCTION__ << " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

    return result;
}

int test_FutureResultTimeout(int threadCount)
{
    ACE_UNUSED_ARG(threadCount);
    int result = 0;
    int expected  = 1;
    int value = 0;

    {
        FutureInt_t future;

        try {
            future.get(100);
        } catch(const DAF::TimeoutException &) {
            value++;
        } DAF_CATCH_ALL {
            // Error
        }
    }

    result = (value == expected) ;
    std::cout << __FUNCTION__ << " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

    return result;
}

/**
 * TEST
 *
 * Deliberately throw inside the Function Call to respond
 * with an InvocationTargetException
 */
int test_FutureResultInvocationTargetExecption(int threadCount)
{
    ACE_UNUSED_ARG(threadCount);
    int result = 0;
    int expected  = 1;
    int value = 0;

    {
        DAF::TaskExecutor executor;

        FutureInt_t future;
        TestThrowFunctor theThrower;

        try {
           executor.execute(future(theThrower, 100));
           future.get();
        } catch (const DAF::InvocationTargetException &) {
            value++;
        }
    }

    result = (value == expected) ;
    std::cout << __FUNCTION__ << " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

    return result;
}

/**
 * TEST
 *
 * Deliberately throw inside the Function Call to respond
 * with an InvocationTargetException
 */
int test_FutureResultDestruction(int threadCount)
{
    int result = 0;
    int expected  = 1;
    int value = 0;
    DAF::Semaphore blocker(0);
    FutureInt_t *fut = new FutureInt_t();
    TestFutureGet *getter = new TestFutureGet(blocker, *fut);
    DAF::Runnable_ref runner(getter);

    {
        DAF::TaskExecutor executor;

        executor.execute(runner);

        for ( int i = 1; i < threadCount; ++i ) {
            executor.execute(new TestFutureGet(blocker, *fut));
        }

        for ( int i = 0; i < threadCount; ++i)
        {
            blocker.acquire();
        }

        delete fut;
    }

    value = getter->invoke;

    result = (value == expected) ;
    std::cout << __FUNCTION__ << " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

    return result;
}


/**
 * TEST
 *
 * Check for FutureResult stability when a thread exits
 */
int test_FutureResultThreadKill(int threadCount)
{
    int result = 0;
    int expected  = 1;
    int value = 0;
    DAF::Semaphore blocker(0);
    FutureInt_t *fut = new FutureInt_t();
    TestFutureGet *getter = new TestFutureGet(blocker, *fut);
    DAF::Runnable_ref runner(getter);

    {
        DAF::TaskExecutor executor;
        DAF::TaskExecutor *kill_executor = new DAF::TaskExecutor;

        executor.execute(runner);

        for ( int i = 1; i < threadCount; ++i ) {
            executor.execute(new TestFutureGet(blocker, *fut));
        }

        for ( int i = 0; i < threadCount; ++i)
        {
            blocker.acquire();
        }

        kill_executor->execute(new TestFutureGet(blocker, *fut));

        blocker.acquire();

        if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T - Killing TaskExecutor\n"));
        delete kill_executor;
    }

    value = fut->isError();

    result = (value == expected) ;
    std::cout << __FUNCTION__ << " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

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
    int result = 1, threadCount = 4;

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

    result &= test::test_FutureResultBasicFunctor(threadCount);
    result &= test::test_FutureResultBasicSetter(threadCount);
    result &= test::test_FutureResultPeek(threadCount);
    result &= test::test_FutureResultReset(threadCount);
    result &= test::test_FutureResultTimeout(threadCount);
    result &= test::test_FutureResultInvocationTargetExecption(threadCount);
    result &= test::test_FutureResultDestruction(threadCount);
    result &= test::test_FutureResultThreadKill(threadCount);

    return !result;
}
