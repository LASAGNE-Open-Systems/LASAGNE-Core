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

#include <daf/Barrier.h>
#include <daf/TaskExecutor.h>

#include <ace/Thread.h>
#include <ace/Get_Opt.h>
#include <ace/Min_Max.h>

#include <iostream>

namespace { // Anonymous

    const int WILD_INITIAL_VALUE = 26857;

    bool debug = false;
    const char *TEST_NAME = "BarrierTest";

    struct TestBarrier : public DAF::Runnable {
        DAF::Barrier   &barrier_;
        int             broken;
        int             interrupted;
        int             illegal;
        int             unknown;
        int             timeout;
        int             returned;
        int             result;
        time_t          msecs_;
        time_t          delay_msec_;
        DAF::Semaphore &sema_;

        TestBarrier(DAF::Semaphore &sema_in, DAF::Barrier &barrier, time_t msecs = 0, time_t delay_msec = 0) : DAF::Runnable()
            , barrier_(barrier)
            , broken(0)
            , interrupted(0)
            , illegal(0)
            , unknown(0)
            , timeout(0)
            , returned(0)
            , result(0)
            , msecs_(ace_max(time_t(0), msecs))
            , delay_msec_(ace_max(time_t(0), delay_msec))
            , sema_(sema_in)
        {
        }

        ~TestBarrier(void)
        {
            if (debug) {
                ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) 0x%@ broken=%d,interrupted=%d,timeout=%d,illegal=%d,unknown=%d,returned=%d\n"), this
                    , this->broken
                    , this->interrupted
                    , this->timeout
                    , this->illegal
                    , this->unknown
                    , this->returned
                    ));
            }
        }

        DAF_DEFINE_REFCOUNTABLE(TestBarrier);

        virtual int run(void)
        {
            int rtnval = 0, error = 0;

            if (this->delay_msec_) {
                DAF_OS::sleep(ACE_Time_Value(0, suseconds_t(this->delay_msec_ * 1000)));
            }

            if (debug) {
                ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t)\t%T 0x%@ Entering Barrier\n"), this));
            }

            try {

                this->sema_.release();

                if (this->msecs_ == 0) {
                    result = rtnval = this->barrier_.barrier();
                } else {
                    result = rtnval = this->barrier_.barrier(this->msecs_);
                }

                ++returned; return 0;

            } catch (const DAF::BrokenBarrierException &e) {
                ++this->broken;
                if (debug) { ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X Broken on Barrier %s\n"), this, e.what())); }
            } catch (const DAF::TimeoutException &e) {
                ++this->timeout;
                if (debug) { ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X Timeout on Barrier %s\n"), this, e.what())); }
            } catch (const DAF::InterruptedException &e) {
                ++this->interrupted;
                if (debug) { ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X Interrupted on Barrier %s\n"), this, e.what())); }
            } catch (const std::exception &e) {
                ++this->illegal;
                if (debug) { ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X Interrupted on Barrier %s\n"), this, e.what())); }
            } catch (...) {
                ++this->unknown;
                if (debug) { ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - Unknown Exception on Barrier\n"))); }
                throw;
            }

            return 0;
        }
    };

    DAF_DECLARE_REFCOUNTABLE(TestBarrier);

    struct TestTrigger : DAF::Runnable
    {
        int count;
        bool ran;
        const ACE_Time_Value delay_;

        TestTrigger(const ACE_Time_Value & delay = ACE_Time_Value(1)) : DAF::Runnable()
            , count(0), ran(false), delay_(delay)
        {
        }

        DAF_DEFINE_REFCOUNTABLE(TestTrigger);

        virtual int run(void)
        {
            this->ran = true; ++this->count;
            if (debug) {
                ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) %T - 0x%@ Trigger Call Count %d\n"), this, this->count));
            }
            DAF_OS::sleep(this->delay_); return 0;
        }
    };

    DAF_DECLARE_REFCOUNTABLE(TestTrigger);

} // Anonymous

/**
 * TEST
 *
 * No Barrier Command, will the barrier still function with when a
 * BarrierCommand has not been set.
 */
int test_BarrierNoCommand(int threadCount )
{
    int expected = 0, value = WILD_INITIAL_VALUE;

    DAF::Semaphore  blocker(0);

    {
        DAF::Barrier    barrier(threadCount);
        {
            DAF::TaskExecutor executor;

            for (int i = 1; i < threadCount; ++i) {
                executor.execute(new TestBarrier(blocker, barrier)); blocker.acquire(); ++expected;
            }

            value = barrier.barrier();
        }
    }

    int result = (expected == value);

    std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

    return result;
}

/**
 * TEST
 * With Barrier Command does the Barrier Command get called ?
 */
int test_BarrierCommand(int threadCount)
{
    int expected = 0, value = WILD_INITIAL_VALUE;

    DAF::Semaphore  blocker(0);
    TestTrigger_ref trigger(new TestTrigger());

    {
        DAF::Barrier    barrier(threadCount);
        {
            DAF::TaskExecutor executor;

            barrier.setBarrierCommand(trigger);

            for (int i = 1; i < threadCount; ++i) {
                executor.execute(new TestBarrier(blocker, barrier)); blocker.acquire(); ++expected;
            }

            int returned = barrier.barrier();

            if (trigger->count) {
                value = returned;
            }
        }
    }

    int result = (value == expected);

    std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

    return result;
}

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
int test_BarrierWaitResetTimeoutClean(int threadCount)
{
    ACE_UNUSED_ARG(threadCount);

    int expected = 0, value = WILD_INITIAL_VALUE;

    DAF::Semaphore  blocker(0);
    TestTrigger_ref trigger(new TestTrigger());

    {
        DAF::Barrier        barrier(2, trigger);
        TestBarrier_ref     tester(new TestBarrier(blocker, barrier));
        TestBarrier_ref     player(new TestBarrier(blocker, barrier, 0, 100)); // Test actor

        {
            DAF::TaskExecutor executor;

            executor.execute(tester);   blocker.acquire(); ++expected;
            executor.execute(player);   blocker.acquire(); ++expected;

            if (barrier.waitReset(100) || barrier.broken()) {
                ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: %C should have observed a clean competion, but did not!\n"), __FUNCTION__), 0);
            } else if (trigger->ran ? false : true) {
                ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: %C Trigger should have run, but did not!\n"), __FUNCTION__), 0);
            }

            DAF_OS::thr_yield();
        }

        value = tester->returned + player->returned;
    }

    int result = (value == expected);

    std::cout << __FUNCTION__ << " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED") << std::endl;

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
int test_BarrierWaitResetTimeout(int threadCount)
{
    int expected = 0, value = WILD_INITIAL_VALUE;

    DAF::Semaphore  blocker(0);
    TestTrigger_ref trigger(new TestTrigger());

    {  // Scope rend for destructor

        DAF::Barrier        barrier(threadCount, trigger);
        TestBarrier_ref     tester(new TestBarrier(blocker, barrier));

        {
            DAF::TaskExecutor executor;

            for (int i = 1; i < threadCount; ++i) {
                executor.execute(tester); blocker.acquire(); ++expected;
            }

            // Want this one to work after a period of time.
            if (barrier.waitReset(100) && DAF_OS::last_error() == ETIME) {
                if (trigger->ran ? true : false) {
                    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: %C Functor should not have run, but did!\n"), __FUNCTION__), 0);
                }
                value = barrier.broken() ? WILD_INITIAL_VALUE : 0; // Ensure we are no longer broken after reset with timeout
            }

            DAF_OS::thr_yield();
        }

        value += barrier.broken() ? tester->unknown : tester->broken; // Should now be broken after clobber with an unknown (abi)
    }

    int result = (value == expected);

    std::cout << __FUNCTION__ << " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED") << std::endl;

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
int test_BarrierWaitResetHard(int threadCount)
{
    int expected = 0, value = WILD_INITIAL_VALUE;

    DAF::Semaphore  blocker(0);
    TestTrigger_ref trigger(new TestTrigger());

    { // Scope rend

        DAF::Barrier        barrier(threadCount, trigger);
        TestBarrier_ref     tester(new TestBarrier(blocker, barrier));

        {
            DAF::TaskExecutor executor;

            for (int i = 1; i < threadCount; ++i) {
                executor.execute(tester); blocker.acquire(); ++expected;
            }

            if (barrier.waitReset(100) && DAF_OS::last_error() == ETIME) {
                barrier.interrupt(); value = 0;
            }

            DAF_OS::thr_yield();
        }

        if (trigger->ran ? true : false) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: %C Functor should not have run, but did!\n"), __FUNCTION__), 0);
        }

        value += tester->interrupted + tester->broken; // Either of these would happen dependant on thread scheduling (probably broken)
    }

    int result = (value == expected);

    std::cout << __FUNCTION__ << " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED") << std::endl;

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
int test_BarrierWaitResetClean(int threadCount)
{
    int expected = 0, value = WILD_INITIAL_VALUE;

    DAF::Semaphore  blocker(0);
    TestTrigger_ref trigger(new TestTrigger());

    { // scope rend

        DAF::Barrier        barrier((threadCount - 1), trigger);
        TestBarrier_ref     tester(new TestBarrier(blocker, barrier));

        {
            DAF::TaskExecutor executor;

            for (int i = 1; i < threadCount; ++i) {
                executor.execute(tester); blocker.acquire(); ++expected;
            }

            if (barrier.waitReset(100) || barrier.broken()) {
                ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: %C should have observed a clean competion, but did not!\n"), __FUNCTION__), 0);
            } else if (trigger->ran ? false : true) {
                ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: %C Functor should have run, but did not!\n"), __FUNCTION__), 0);
            }

            DAF_OS::thr_yield();
        }

        value = tester->returned;
    }

    int result = (value == expected);

    std::cout << __FUNCTION__ << " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED") << std::endl;

    return result;
}

/**
* TEST
*
* Testing the rendezvous(value, timeout) functionality
* and making sure the Rendezvous condition is set before timeout
*
* NOTE: We have 1 too little for the rendezvous
*/
int test_BarrierTimeoutBroken(int threadCount)
{
    int expected = 0, value = WILD_INITIAL_VALUE;

    DAF::Semaphore  blocker(0);
    TestTrigger_ref trigger(new TestTrigger());

    {
        DAF::Barrier        barrier(threadCount, trigger);
        TestBarrier_ref     tester(new TestBarrier(blocker, barrier, 100));

        {
            DAF::TaskExecutor executor;

            for (int i = 1; i < threadCount; ++i) {
                executor.execute(tester); blocker.acquire(); ++expected;
            }

            DAF_OS::sleep(1);

            value = tester->timeout + tester->broken;

            DAF_OS::thr_yield();
        }
    }

    int result = (value == expected);

    std::cout << __FUNCTION__ << " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED") << std::endl;

    return result;
}

/**
* TEST
*
* Testing the rendezvous(value, timeout) functionality
* and making sure the Rendezvous condition is set before timeout
*
* NOTE: We have 1 too many for the rendezvous.
*/
int test_BarrierTimeoutIllegal(int threadCount)
{
    int expected = 0, value = WILD_INITIAL_VALUE;

    DAF::Semaphore  blocker(0);
    TestTrigger_ref trigger(new TestTrigger());

    {
        DAF::Barrier        barrier(threadCount - 1, trigger);
        TestBarrier_ref     tester(new TestBarrier(blocker, barrier, 100));

        {
            DAF::TaskExecutor executor;

            for (int i = 0; i < threadCount; ++i) {
                executor.execute(tester); blocker.acquire(); ++expected;
            }

            DAF_OS::sleep(1); // Need to wait for threads to do their thing

            if (tester->returned == (expected - 1)) {
                value = tester->returned + tester->illegal;
            }

            DAF_OS::thr_yield();
        }
    }

    int result = (value == expected);

    std::cout << __FUNCTION__ << " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED") << std::endl;

    return result;
}

/**
* TEST
*
* Testing the Broken exception is thrown when another thread
* exits the rendezvous.
*/
int test_BarrierBroken(int threadCount)
{
    int expected = 0, value = WILD_INITIAL_VALUE;

    DAF::Semaphore  blocker(0);
    TestTrigger_ref trigger(new TestTrigger());

    {
        DAF::Barrier        barrier(threadCount, trigger);
        TestBarrier_ref     tester(new TestBarrier(blocker, barrier, 500));

        {
            DAF::TaskExecutor executor;

            for (int i = 2; i < threadCount; ++i) {
                executor.execute(tester); blocker.acquire(); ++expected;
            }

            try {
                barrier.barrier();
            } catch (const DAF::BrokenBarrierException &) {
                value = expected;
            }

            DAF_OS::thr_yield();
        }
    }

    int result = (value == expected);

    std::cout << __FUNCTION__ << " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED") << std::endl;

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
int test_BarrierBasicWorking(int threadCount)
{
    int expected = 1, value = WILD_INITIAL_VALUE;

    DAF::Semaphore  blocker(0);
    TestTrigger_ref trigger(new TestTrigger());

    {
        DAF::Barrier    barrier(threadCount, trigger);
        {
            DAF::TaskExecutor executor;

            for (int i = 1; i < threadCount; ++i) {
                executor.execute(new TestBarrier(blocker, barrier)); blocker.acquire();
            }

            barrier.barrier();

            DAF_OS::thr_yield();
        }

        value = trigger->count;
    }

    int result = (value == expected);

    std::cout << __FUNCTION__ << " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED") << std::endl;

    return result;
}

/**
* TEST
*
* Rendezvous Is getting shutdown and an IllegalThreadState should
* be used. Wanting to use multiple Entrants here to ensure the
* destruction correctly waits for all entrants.
*/
int test_BarrierDestruction(int threadCount)
{
    int expected = 0, value = WILD_INITIAL_VALUE;

    DAF::Semaphore  blocker(0);
    TestTrigger_ref trigger(new TestTrigger());

    {
        DAF::Barrier *      barrier(new DAF::Barrier(threadCount, trigger));
        TestBarrier_ref     tester(new TestBarrier(blocker, *barrier));

        {
            DAF::TaskExecutor executor;

            for (int i = 1; i < threadCount; ++i) {
                executor.execute(tester); blocker.acquire(); ++expected;
            }

            DAF_OS::sleep(1);

            delete barrier; barrier = 0;

            DAF_OS::thr_yield();
        }

        value = tester->interrupted;
    }

    int result = (value == expected);

    std::cout << __FUNCTION__ << " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED") << std::endl;

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
int test_BarrierCtorZero(int threadCount)
{
    ACE_UNUSED_ARG(threadCount);

    int expected = 1, value = WILD_INITIAL_VALUE;

    try {
        DAF::Barrier(0).barrier(100);
    } catch (const DAF::InitializationException &) {
        value = 1;
    } catch (const DAF::TimeoutException &) {
        value = 2;
    } catch (const std::exception &) {
        value = 3;
    }

    int result = (value == expected);

    std::cout << __FUNCTION__ << " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED") << std::endl;

    return result;
}

/**
* TEST
*
* Testing thread killing stability
*
*
*/
int test_BarrierThreadKill(int threadCount)
{
    int expected = 0, value = WILD_INITIAL_VALUE;

    DAF::Semaphore  blocker(0);
    TestTrigger_ref trigger(new TestTrigger());

    {
        DAF::Barrier        barrier(threadCount + 1, trigger);
        TestBarrier_ref     tester(new TestBarrier(blocker, barrier));

        {
            DAF::TaskExecutor executor, *kill_executor = new DAF::TaskExecutor;

            for (int i = 1; i < threadCount; ++i) {
                executor.execute(tester); blocker.acquire(); ++expected;
            }

            kill_executor->execute(tester); blocker.acquire(); ++expected;

            DAF_OS::sleep(1);

            delete kill_executor; kill_executor = 0;

            DAF_OS::thr_yield();
        }

        if (trigger->ran) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: %C should not have run, but did!\n"), __FUNCTION__), 0);
        }

        value = tester->broken + tester->unknown; // We could have recieved an abi unwind
    }

    int result = (value == expected);

    std::cout << __FUNCTION__ << " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED") << std::endl;

    return result;
}

void print_usage(const ACE_Get_Opt &cli_opt)
{
    ACE_UNUSED_ARG(cli_opt);
    std::cout << TEST_NAME
              << " -h --help              : Print this message \n"
              << " -z --debug             : Debug \n"
              << " -n --count             : Number of Threads/Test\n"
              << std::endl;
}

int main(int argc, char *argv[])
{
    ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) %T - %C\n"), TEST_NAME));

    int result = 1, threadCount = 3; // Must be at least 3 threads for tests to pass

    ACE_Get_Opt cli_opt(argc, argv, "hzn:");
    cli_opt.long_option("help",'h', ACE_Get_Opt::NO_ARG);
    cli_opt.long_option("debug",'z', ACE_Get_Opt::NO_ARG);
    cli_opt.long_option("count",'n', ACE_Get_Opt::ARG_REQUIRED);

    for( int i = 0; i < argc; ++i ) switch(cli_opt()) {
        case -1: break;
        case 'h': print_usage(cli_opt); return 0;
        case 'z': DAF::debug(true); debug=true; break;
        case 'n': threadCount = ace_max(3,DAF_OS::atoi(cli_opt.opt_arg()));
    }

    result &= test_BarrierCommand(threadCount);
    result &= test_BarrierNoCommand(threadCount);
    result &= test_BarrierBasicWorking(threadCount);
    result &= test_BarrierTimeoutBroken(threadCount);
    result &= test_BarrierTimeoutIllegal(threadCount);
    result &= test_BarrierBroken(threadCount);
    result &= test_BarrierWaitResetClean(threadCount);
    result &= test_BarrierWaitResetTimeoutClean(threadCount);
    result &= test_BarrierWaitResetTimeout(threadCount);
    result &= test_BarrierWaitResetHard(threadCount);
    result &= test_BarrierDestruction(threadCount);
    result &= test_BarrierCtorZero(threadCount);
    result &= test_BarrierThreadKill(threadCount);

    return !result;
}
