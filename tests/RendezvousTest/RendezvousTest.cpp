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
#include "daf/Rendezvous_T.h"
#include "daf/TaskExecutor.h"
#include "ace/Thread.h"
#include "ace/Get_Opt.h"
#include <iostream>
#include <vector>
#include <algorithm>

/**
 * At the moment the Rendezvous Template takes a copy of the Functor object,
 * prohibiting storing of state in the Functor and forcing
 * copyable properties. TODO revisit this.
 *
 * It looks like the use case is to return alternative slots to a thread.
 *
 */

namespace test
{
    bool debug = false;
    const char *TEST_NAME = "RendezvousTest";

    struct TestRendFunc
    {
        int value;
        bool ran;
        ACE_Time_Value delay;
        TestRendFunc(const ACE_Time_Value& delay_in = ACE_Time_Value()) : value(0), ran(false), delay(delay_in) {}

        struct InnerFunctor
        {
            int &valueInner;
            InnerFunctor(int &va) : valueInner(va) {}

            void operator()(int i )
            {
                valueInner += i;
            }
        };

        void operator()( std::vector<int> &v)
        {
            ran = true;
            if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T 0x%08X Running Rendezvous Functor\n", this));
            DAF_OS::sleep(this->delay);
            if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T 0x%08X Executing Rendezvous Functor\n", this));

            for ( size_t i = 0 ; i < v.size(); i++ )
            {
               if ( debug) std::cout << i << ":" << v[i] << std::endl;

               value += v[i];
            }
            if (debug) std::cout << "sum " << value << std::endl;

            //InnerFunctor inner(this->value);
            //std::for_each(v.begin(), v.end(), inner);
        }

        void operator()( int i)
        {
            value += i;
        }
    };

    typedef DAF::Rendezvous<int, TestRendFunc> RendezvousTest_t;

    struct TestRendezvous : public DAF::Runnable
    {
       RendezvousTest_t &rend;
       int id;
       int broken;
       int illegal;
       int unknown;
       int timeout;
       time_t msec;
       int result;
       time_t delay_msec;
       DAF::Semaphore &sema;

       TestRendezvous(DAF::Semaphore &sema_in, int idi, RendezvousTest_t &theRend, time_t mseci = 0, time_t delay_mseci = 0) : DAF::Runnable()
       , rend(theRend)
       , id(idi)
       , broken(0)
       , illegal(0)
       , unknown(0)
       , timeout(0)
       , msec(mseci)
       , result(0)
       , delay_msec(delay_mseci)
       , sema(sema_in)
       { }

       virtual int run(void)
       {
           if ( delay_msec ) DAF_OS::sleep(ACE_Time_Value(0, suseconds_t(delay_msec)));

           if ( debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T 0x%08X Entering Rendezvous %d\n", this, this->id));

           try {
               sema.release();
               if (this->msec == 0 )
                  result = this->rend.rendezvous(this->id);
               else
                  result = this->rend.rendezvous(this->id, this->msec);
           } catch ( const DAF::BrokenBarrierException &e) {
                this->broken++;
                if (debug) ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X Broken on Rend %s\n"), this,  e.what()));
            } catch (const DAF::TimeoutException &te ) {
                this->timeout++;
                if (debug) ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X Timeout on Rend %s\n"), this, te.what()));
            } catch ( const DAF::IllegalThreadStateException &te ) {
                this->illegal++;
                if(debug) ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X IllegalThreadState on Rend %s\n"), this, te.what()));
            } DAF_CATCH_ALL {
                this->unknown++;
                if(debug) ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - Unknown Exception on Rend\n")));
            }

           return 0;
       }
    };

    struct TestRotator : DAF::Runnable
    {
      DAF::Rendezvous<int> &rend;
      int id;
      int result;

      TestRotator(DAF::Rendezvous<int>& ren, int idin)
      : rend(ren)
      , id(idin)
      , result(0)
      { }

      virtual int run(void)
       {
           if ( debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T 0x%08X Entering Rendezvous %d\n", this, this->id));

           try {
                  result = this->rend.rendezvous(this->id);
           } catch ( const DAF::BrokenBarrierException &e) {
                if (debug) ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X Broken on Rend %s\n"), this,  e.what()));
            } catch ( const DAF::TimeoutException &te ) {
                if (debug) ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X Timeout on Rend %s\n"), this, te.what()));
            } catch ( const DAF::IllegalThreadStateException &te ) {
                if(debug) ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X IllegalThreadState on Rend %s\n"), this, te.what()));
            } DAF_CATCH_ALL {
                if(debug) ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - Unknown Exception on Rend\n")));
            }

           return 0;
       }
    };



    /**
     * TEST
     *
     * Testing the WaitReset functionality
     * This test is supposed to allow a waitReset to not expire
     * with an ETIME error. ie the Rendezvous succeeded.
     * and we are particularly loooking for the error
     * condition on "broken" it should false
     *
     */
    int test_RendezvousWaitResetTimeoutClean(int threadCount)
    {
        ACE_UNUSED_ARG(threadCount);
        int result = 1;
        int expected = 0;
        int value = 0;
        bool clean  = false;

        DAF::Semaphore counter(0);
        TestRendFunc functor;

        RendezvousTest_t rend(2, functor);
        TestRendezvous *tester = new TestRendezvous(counter, 1, rend);

        DAF::Runnable_ref runner(tester);

        {
            DAF::TaskExecutor executor;

            executor.execute(runner);
            executor.execute(new TestRendezvous(counter, 2, rend, 0, 100));

            counter.acquire();
            counter.acquire();


            try {
                // Want this one to work after a period of time.
                // TODO
                clean = rend.waitReset(1000);

                result &= !rend.broken() && clean;

                if(debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - Clean Wait %s %d\n", (clean ? "YES" : "NO"), result));
            } catch( const DAF::IllegalThreadStateException &te) {
                if(debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - Got Illegal State? %s\n", te.what()));
                result = 0;
            }
            DAF_OS::thr_yield();
        }

        value = tester->broken;

        result &= (value == expected) && functor.ran;

        std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

    /**
     * TEST
     *
     * Testing the WaitReset functionality
     * This test is supposed to allow a waitReset to expire
     * with an ETIME error. ie the Rendezvous succeeded.
     * and we are particularly loooking for the error
     * condition on "broken" it should true
     *
     */
    int test_RendezvousWaitResetTimeout(int threadCount)
    {
        ACE_UNUSED_ARG(threadCount);
        int result = 1;
        int expected = 1;
        int value = 0;
        bool clean  = false;

        TestRendFunc functor;
        DAF::Semaphore counter(0);

        RendezvousTest_t rend(2, functor);
        TestRendezvous *tester = new TestRendezvous(counter, 1, rend);

        DAF::Runnable_ref runner(tester);

        {
            DAF::TaskExecutor executor;

            executor.execute(runner);

            counter.acquire();


            try {
                // Want this one to work after a period of time.
                // TODO
                clean = rend.waitReset(1000);

                result &= rend.broken() && !clean;

                if(debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - Clean Wait %s %d\n", (clean ? "YES" : "NO"), result));
            } catch( const DAF::IllegalThreadStateException &te) {
                if(debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - Got Illegal State? %s\n", te.what()));
                result = 0;
            }
            DAF_OS::thr_yield();
        }

        value = tester->broken;

        result &= (value == expected) && !functor.ran;

        std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

    /**
     * TEST
     *
     * Testing the WaitReset functionality
     * Because waitReset sets the broken flag, the rendezvous is
     * nuffed. What we are particularly interested in here
     * is that the entrant parties exit cleanly.
     *
     *
     */
    int test_RendezvousWaitResetHard(int threadCount)
    {
        ACE_UNUSED_ARG(threadCount);
        int result = 1;
        int expected = 1;
        int value = 0;
        bool clean  = false;

        DAF::Semaphore counter(0);
        TestRendFunc functor;

        RendezvousTest_t rend(expected*2, functor);
        TestRendezvous *tester = new TestRendezvous(counter, 1, rend);

        DAF::Runnable_ref runner(tester);

        {
            DAF::TaskExecutor executor;

            executor.execute(runner);

            counter.acquire();


            try {
                clean = rend.waitReset();

                result &= rend.broken() && !clean;

                if(debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - Clean Wait %s\n", (clean ? "YES" : "NO")));
            } catch( const DAF::IllegalThreadStateException &te) {
                if(debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - Got Illegal State? %s\n", te.what()));
                result = 0;
            }
            DAF_OS::thr_yield();
        }

        value = tester->broken;

        result &= (value == expected) && !functor.ran;

        std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

    /**
     * TEST
     *
     * Testing the WaitReset functionality
     * Because waitReset sets the broken flag, the rendezvous is
     * nuffed. The Second rendezvous is enters but immediately
     * sees the broken condition and will return.
     * What we are particularly interested in here is the
     * the return value being true when the parties/rendezvous
     * has been completed
     *
     */
    int test_RendezvousWaitResetClean(int threadCount)
    {
        ACE_UNUSED_ARG(threadCount);
        int result = 1;
        int expected = 0;
        int value = 0;
        //bool clean  = false;

        DAF::Semaphore blocker(0);
        TestRendFunc functor;

        RendezvousTest_t rend(2, functor);
        TestRendezvous *tester = new TestRendezvous(blocker, 1, rend);

        DAF::Runnable_ref runner(tester);

        {
            DAF::TaskExecutor executor;

            executor.execute(runner);
            executor.execute(runner);

            blocker.acquire();
            blocker.acquire();

            result = rend.waitReset(100) && !rend.broken();

            DAF_OS::thr_yield();
        }

        value = tester->broken;

        result &= (value == expected) && functor.ran;

        std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

    /**
     * TEST
     *
     * Testing the rendezvous(value, timeout) functionality
     * and making sure the Timeout condition is set.
     */
    int test_RendezvousTimeout(int threadCount)
    {
        int result = 0;
        int expected = threadCount;
        int value = 0;

        DAF::Semaphore counter(0);
        TestRendFunc functor;

        {
            DAF::TaskExecutor executor;
            RendezvousTest_t rend(2, functor);

            DAF::Runnable_ref timeout = new TestRendezvous(counter, 1, rend, 100);

            for ( int i = 0; i < threadCount; ++i )
            {
                executor.execute(timeout);
                DAF_OS::sleep(ACE_Time_Value(0, 500000));
            }

            value = reinterpret_cast<TestRendezvous*>(timeout.ptr())->timeout;
        }

        result = (value == expected);

        std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

    /**
     * TEST
     *
     * Testing the Broken exception is thrown when another thread
     * exits the rendezvous.
     */
    int test_RendezvousBroken(int threadCount)
    {
        ACE_UNUSED_ARG(threadCount);
        int result = 0;
        int expected =1;
        int value = 0 ;

        DAF::Semaphore counter(0);
        TestRendFunc functor;

        {
            DAF::TaskExecutor executor;
            RendezvousTest_t rend(3, functor);

            DAF::Runnable_ref breaker = new TestRendezvous(counter, 2, rend, 500);

            executor.execute(breaker);

            try {
                rend.rendezvous(value);
            } catch (const DAF::BrokenBarrierException ) {
                value++;
            }
        }

        result = (value == expected);

        std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

    /**
     * TEST
     *
     * Simple functionality Test for the Rotator. The Rotator
     * is working as an Exchanger and give back a rotated slot value
     * to the incoming threads. For two rendezvous entrants we expect
     * values to be swapped.
     */
    int test_RendezvousRotator(int threadCount)
    {
        ACE_UNUSED_ARG(threadCount);
        int result = 0;
        int expected = 10;
        const int rotateValue = 20;
        int value = 0;
        DAF::RendezvousRotator<int> rotator;

        DAF::Rendezvous<int> rend(2,rotator);

        TestRotator *test1 = new TestRotator(rend,expected);

        DAF::Runnable_ref runner1(test1);

        {
            DAF::TaskExecutor executor;

            executor.execute(runner1);

            value = rend.rendezvous(rotateValue);
        }

        result = (test1->result == rotateValue) && (value  == expected);

        std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

    /**
     * TEST
     *
     * Rendezvous basic working should test that The Function
     * object can be used. It will fail if the
     * Function object is copied and not referenced. Disallowing
     * the return value to return.
     */
    int test_RendezvousBasicWorking(int threadCount)
    {
        int result = 0;
        int expected = 1;
        int value = 0;
        TestRendFunc functor;

        DAF::Semaphore counter(0);

        {
            DAF::TaskExecutor executor;

            RendezvousTest_t rendez(threadCount+1, functor);
            for(int i = 0; i < threadCount; ++i )
            {
                executor.execute(new TestRendezvous(counter, i+1, rendez));
                expected += i + 1;
            }

            rendez.rendezvous(1);
            DAF_OS::sleep(ACE_Time_Value(0, 500));
        }

        value = functor.value;
        result = (value == expected);

        std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

    /**
     * TEST
     *
     * Rendezvous Is getting shutdown and an IllegalThreadState should
     * be used. Wanting to use multiple Entrants here to ensure the
     * destruction correctly waits for all entrants.
     */
    int test_RendezvousDestruction(int threadCount)
    {
        std::cout << __FUNCTION__ ;

        int result = 0;
        int expected = 1;
        int value = 0;
        TestRendFunc functor;

        DAF::Semaphore blocker(0);

        RendezvousTest_t *rend = new RendezvousTest_t(threadCount+2, functor);

        TestRendezvous *tester = new TestRendezvous(blocker, 1, *rend);

        DAF::Runnable_ref runner(tester);

        {
            DAF::TaskExecutor executor;

            executor.execute(runner);
            for( int i =1; i < threadCount; ++i )
            {
                executor.execute(new TestRendezvous(blocker, i+1, *rend));
            }

            for( int i =0; i < threadCount; ++i )
            {
                blocker.acquire();
            }


            // Deliberately Destroy the Rendezvous
            // to make sure the destruction process doesn't deadlock.
            delete rend;
            DAF_OS::thr_yield();
        }

        value = tester->illegal + tester->broken;
        result = (value == expected);

        std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

     /**
     * TEST
     *
     * Destruction when a Long processing Trigger has been executed. The problems that can occur here are:
     * - Other Rendezvous Entrants are stuck on the entryGate
     * - The Trigger is modifying the rendezvous vector.
     * - destruction will have two sets of problems.
     * -- The Rendezvous Entrants
     * -- How to kill the Trigger ?
     *
     * Testing this situation with a Delay in the Rendezvous Trigger
     * TODO - also perform waitReset Function test.
     */
    int test_RendezvousDestructionLongTrigger(int threadCount)
    {
        std::cout << __FUNCTION__ ;

        int result = 0;
        int expected = 1;
        int value = expected;
        const ACE_Time_Value triggerDelay(1,0);
        TestRendFunc functor(triggerDelay);

        DAF::Semaphore blocker(0);

        RendezvousTest_t *rend = new RendezvousTest_t(threadCount, functor);

        {
            DAF::TaskExecutor executor;

            // Add more than the trigger amount to the rendezvous.
            // This ensures that we have a full set of waiters on the trigger.
            // And a set of waiters on the entryGate.
            for( int i =1; i < threadCount*2; ++i )
            {
                executor.execute(new TestRendezvous(blocker, i+1, *rend));
            }

            // Wait for the executor to catch up.
            for( int i = 1; i < threadCount*2; ++i)
            {
                blocker.acquire();
            }

            // Deliberately Destroy the Rendezvous
            // to make sure the destruction process doesn't deadlock.
            delete rend;
            DAF_OS::thr_yield();
        }

        // TODO Test Condition? - If it got here it didn't deadlock ?
        // Just always pass

        result = (value == expected);

        std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

     /**
     * TEST
     *
     * Rendezvous constructed with zero parties. what happens ?
     * What should happen?
     * There could be a number of ways of handling this.
     * 1. Throw DAF::InitializationException
     * 2. Trigger every time, ie same as parties == 1.
     * 3. Throw an exception on rendezvous entry ie IllegalStateException?
     *
     * Following "fail early"  philosophy and expecting InitializationException
     * Changed this behaviour to adopt the 0 -> 1 (option 2)
     */
    int test_RendezvousCtorZero(int threadCount)
    {
        ACE_UNUSED_ARG(threadCount);
        std::cout << __FUNCTION__ ;

        int result = 0;
        int expected = 1;
        int value = 0;

        TestRendFunc functor;

        try {
            RendezvousTest_t rendez(0, functor);
            DAF::TaskExecutor executor;
            rendez.rendezvous(1,100);
        } catch (const DAF::InitializationException & ) {
            value = 1;
        } catch (const DAF::TimeoutException & ) {
            value = 2;
        }

        result = (value == expected);

        std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

     /**
     * TEST
     *
     * Testing thread killing stability
     *
     *
     */
    int test_RendezvousThreadKill(int threadCount)
    {
        ACE_UNUSED_ARG(threadCount);
        int result = 1;
        int expected = 1;
        int value = 0;

        DAF::Semaphore counter(0);
        TestRendFunc functor;

        RendezvousTest_t rend(threadCount+2, functor);
        TestRendezvous *tester = new TestRendezvous(counter, 1, rend);

        DAF::Runnable_ref runner(tester);

        {
            DAF::TaskExecutor executor;
            DAF::TaskExecutor *kill_executor = new DAF::TaskExecutor;

            executor.execute(runner);
            for( int i =1; i < threadCount; ++i )
            {
                executor.execute(new TestRendezvous(counter, i+1, rend));
            }

            // Wait for the executor to catch up.
            for( int i = 0; i < threadCount; ++i)
            {
                counter.acquire();
            }

            kill_executor->execute(new TestRendezvous(counter, threadCount+1, rend));
            counter.acquire();

            if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T - Killing Executor\n"));

            delete kill_executor;

            DAF_OS::thr_yield();
            //result &= rend.broken();


        }

        value = tester->broken;

        result &= (value == expected) && !functor.ran;

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
    int result = 1, threadCount = 3;

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

    result &= test::test_RendezvousBasicWorking(threadCount);
    result &= test::test_RendezvousTimeout(threadCount);
    result &= test::test_RendezvousBroken(threadCount);
    result &= test::test_RendezvousRotator(threadCount);
    result &= test::test_RendezvousWaitResetClean(threadCount);
    result &= test::test_RendezvousWaitResetTimeoutClean(threadCount);
    result &= test::test_RendezvousWaitResetTimeout(threadCount);
    result &= test::test_RendezvousWaitResetHard(threadCount);
    result &= test::test_RendezvousDestruction(threadCount);
    result &= test::test_RendezvousDestructionLongTrigger(threadCount);

    result &= test::test_RendezvousCtorZero(threadCount);
#ifndef ACE_WIN32
    result &= test::test_RendezvousThreadKill(threadCount);
#endif

    return !result;
}
