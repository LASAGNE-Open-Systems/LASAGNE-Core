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

#include <daf/Rendezvous_T.h>
#include <daf/TaskExecutor.h>

#include <ace/Thread.h>
#include <ace/Get_Opt.h>
#include <ace/Min_Max.h>

#include <iostream>

/**
 * At the moment the Rendezvous Template takes a copy of the Functor object,
 * prohibiting storing of state in the Functor and forcing
 * copyable properties. TODO revisit this.
 *
 * It looks like the use case is to return alternative slots to a thread.
 *
 */

namespace
{ // Anonymous

    const int WILD_INITIAL_VALUE = 26857;

    bool debug = false;
    const char *TEST_NAME = "RendezvousTest";

    struct TestRendFunc : DAF::RendezvousCommand<int> {
        int value;
        bool ran;
        ACE_Time_Value delay_;
        TestRendFunc(const ACE_Time_Value & delay = ACE_Time_Value(1)) : value(0), ran(false), delay_(delay) {}

        struct InnerFunctor {
            int &valueInner;
            InnerFunctor(int &va) : valueInner(va) {}

            void operator()(int i)
            {
                valueInner += i;
            }
        };

        void operator()(std::vector<int> & v)
        {
            ran = true;
            for (size_t i = 0; i < v.size(); i++) {
                if (debug) {
                    std::cout << i << ":" << v[i] << std::endl;
                }

                value += v[i];
            }

            if (debug) {
                std::cout << "sum " << value << std::endl;
            }

            DAF_OS::sleep(this->delay_);

            //InnerFunctor inner(this->value);
            //std::for_each(v.begin(), v.end(), inner);
        }

        void operator()(int i)
        {
            value += i;
        }
    };

    typedef DAF::Rendezvous<int, TestRendFunc> RendezvousTest_t;

    struct TestRendezvous : public DAF::Runnable {
        RendezvousTest_t &rend;
        int id;
        int broken;
        int interrupted;
        int illegal;
        int unknown;
        int timeout;
        int returned;
        time_t msec;
        int result;
        time_t delay_msec;
        DAF::Semaphore &sema;

        TestRendezvous(DAF::Semaphore &sema_in, int idi, RendezvousTest_t &theRend, time_t mseci = 0, time_t delay_mseci = 0) : DAF::Runnable()
            , rend(theRend)
            , id(idi)
            , broken(0)
            , interrupted(0)
            , illegal(0)
            , unknown(0)
            , timeout(0)
            , returned(0)
            , msec(mseci)
            , result(0)
            , delay_msec(delay_mseci)
            , sema(sema_in)
        {
        }

        ~TestRendezvous(void)
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

        DAF_DEFINE_REFCOUNTABLE(TestRendezvous);

        virtual int run(void)
        {
            int rtnval = 0;

            if (delay_msec) {
                DAF_OS::sleep(ACE_Time_Value(0, suseconds_t(delay_msec * 1000)));
            }

            if (debug) {
                ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t)\t%T 0x%@ Entering Rendezvous %d\n"), this, this->id));
            }

            try {

                sema.release();

                if (this->msec == 0) {
                    result = rtnval = this->rend.rendezvous(this->id);
                } else {
                    result = rtnval = this->rend.rendezvous(this->id, this->msec);
                }

                ++returned; return 0;
            } catch (const DAF::BrokenBarrierException &e) {
                ++this->broken;
                if (debug) { ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X Broken on Rend %s\n"), this, e.what())); }
            } catch (const DAF::TimeoutException &e) {
                ++this->timeout;
                if (debug) { ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X Timeout on Rend %s\n"), this, e.what())); }
            } catch (const DAF::InterruptedException &e) {
                ++this->interrupted;
                if (debug) { ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X Interrupted on Rend %s\n"), this, e.what())); }
            } catch (const std::exception &e) {
                ++this->illegal;
                if (debug) { ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X Interrupted on Rend %s\n"), this, e.what())); }
            } catch (...) {
                ++this->unknown;
                if (debug) { ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - Unknown Exception on Rend\n"))); }
                throw;
            }

            return 0;
        }
    };

    DAF_DECLARE_REFCOUNTABLE(TestRendezvous);

    struct TestRotator : DAF::Runnable {
        DAF::Rendezvous<int> &rend;
        int id;
        int result;

        TestRotator(DAF::Rendezvous<int>& ren, int idin)
            : rend(ren)
            , id(idin)
            , result(0)
        {
        }

        DAF_DEFINE_REFCOUNTABLE(TestRotator);

        virtual int run(void)
        {
            if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T 0x%08X Entering Rendezvous %d\n", this, this->id));

            try {
                result = this->rend.rendezvous(this->id);
            } catch (const DAF::BrokenBarrierException &e) {
                if (debug) ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X Broken on Rend %s\n"), this, e.what()));
            } catch (const DAF::TimeoutException &te) {
                if (debug) ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X Timeout on Rend %s\n"), this, te.what()));
            } catch (const DAF::IllegalThreadStateException &te) {
                if (debug) ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X IllegalThreadState on Rend %s\n"), this, te.what()));
            } DAF_CATCH_ALL{
                if (debug) ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - Unknown Exception on Rend\n")));
            }

            return 0;
        }
    };

    DAF_DECLARE_REFCOUNTABLE(TestRotator);

} // Anonymous

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

    int expected = 0, value = WILD_INITIAL_VALUE;

    DAF::Semaphore  blocker(0);
    TestRendFunc    functor;

    {
        RendezvousTest_t    rend(2, functor);
        TestRendezvous_ref  tester(new TestRendezvous(blocker, 1, rend));
        TestRendezvous_ref  player(new TestRendezvous(blocker, 2, rend, 0, 100)); // Test actor

        {
            DAF::TaskExecutor executor;

            executor.execute(tester);   blocker.acquire(); ++expected;
            executor.execute(player);   blocker.acquire(); ++expected;

            if (rend.waitReset(100) || rend.broken()) {
                ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: %C should have observed a clean competion, but did not!\n"), __FUNCTION__), 0);
            }
            else if (functor.ran ? false : true) {
                ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: %C Functor should have run, but did not!\n"), __FUNCTION__), 0);
            }

            DAF_OS::thr_yield();
        }

        value = tester->returned + player->returned;
    }

    int result = (value == expected);

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
    int expected = 0, value = WILD_INITIAL_VALUE;

    DAF::Semaphore  blocker(0);
    TestRendFunc    functor;

    {  // Scope rend for destructor

        RendezvousTest_t    rend(threadCount, functor);
        TestRendezvous_ref  tester(new TestRendezvous(blocker, 1, rend));

        {
            DAF::TaskExecutor executor;

            for (int i = 1; i < threadCount; ++i) {
                executor.execute(tester); blocker.acquire(); ++expected;
            }

            // Want this one to work after a period of time.
            if (rend.waitReset(100) && DAF_OS::last_error() == ETIME) {
                if (functor.ran ? true : false) {
                    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: %C Functor should not have run, but did!\n"), __FUNCTION__),0);
                }
                value = rend.broken() ? WILD_INITIAL_VALUE : 0; // Ensure we are not broken on timeout
            }

            DAF_OS::thr_yield();
        }

        value += rend.broken() ? tester->unknown : tester->broken; // Should now be broken after clobber with an unknown (abi)
    }

    int result = (value == expected);

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
    int expected = 0, value = WILD_INITIAL_VALUE;

    DAF::Semaphore  blocker(0);
    TestRendFunc    functor;

    { // Scope rend

        RendezvousTest_t    rend(threadCount, functor);
        TestRendezvous_ref  tester(new TestRendezvous(blocker, 1, rend));

        {
            DAF::TaskExecutor executor;

            for (int i = 1; i < threadCount; ++i) {
                executor.execute(tester); blocker.acquire(); ++expected;
            }

            if (rend.waitReset(100) && DAF_OS::last_error() == ETIME) {
                rend.interrupt(); value = 0;
            }

            DAF_OS::thr_yield();
        }

        if (functor.ran ? true : false) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: %C Functor should not have run, but did!\n"), __FUNCTION__), 0);
        }

        value += tester->interrupted + tester->broken; // Either of these would happen dependant on thread scheduling (probably broken)
    }

    int result = (value == expected);

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
    int expected = 0, value = WILD_INITIAL_VALUE;

    DAF::Semaphore  blocker(0);
    TestRendFunc    functor;

    { // scope rend

        RendezvousTest_t    rend((threadCount - 1), functor);
        TestRendezvous_ref  tester(new TestRendezvous(blocker, 1, rend));

        {
            DAF::TaskExecutor executor;

            for (int i = 1; i < threadCount; ++i) {
                executor.execute(tester); blocker.acquire(); ++expected;
            }

            if (rend.waitReset(100) || rend.broken()) {
                ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: %C should have observed a clean competion, but did not!\n"), __FUNCTION__), 0);
            }
            else if (functor.ran ? false : true) {
                ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: %C Functor should have run, but did not!\n"), __FUNCTION__), 0);
            }

            DAF_OS::thr_yield();
        }

        value = tester->returned;
    }

    int result = (value == expected);

    std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

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
int test_RendezvousTimeoutBroken(int threadCount)
{
    int expected = 0, value = WILD_INITIAL_VALUE;

    DAF::Semaphore  blocker(0);
    TestRendFunc    functor;

    {
        RendezvousTest_t    rend(threadCount, functor);
        TestRendezvous_ref  tester(new TestRendezvous(blocker, 6, rend, 100));

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

    std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

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
int test_RendezvousTimeoutIllegal(int threadCount)
{
    int expected = 0, value = WILD_INITIAL_VALUE;

    DAF::Semaphore  blocker(0);
    TestRendFunc    functor;

    {
        RendezvousTest_t    rend(threadCount - 1, functor);
        TestRendezvous_ref  tester(new TestRendezvous(blocker, 6, rend, 100));

        {
            DAF::TaskExecutor executor;

            for (int i = 0; i < threadCount; ++i) {
                executor.execute(tester); blocker.acquire(); ++expected;
            }

            DAF_OS::sleep(1);

            if (tester->returned == (threadCount - 1)) {
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
int test_RendezvousBroken(int threadCount)
{
    int expected = 0, value = WILD_INITIAL_VALUE;

    DAF::Semaphore  blocker(0);
    TestRendFunc    functor;

    {
        RendezvousTest_t    rend(3, functor);
        TestRendezvous_ref  tester(new TestRendezvous(blocker, 2, rend, 500));

        {
            DAF::TaskExecutor executor;

            for (int i = 2; i < threadCount; ++i) {
                executor.execute(tester); blocker.acquire(); ++expected;
            }

            try {
                rend.rendezvous(value);
            }
            catch (const DAF::BrokenBarrierException &) {
                value = expected;
            }

            DAF_OS::thr_yield();
        }
    }

    int result = (value == expected);

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
int test_RendezvousCommand(int threadCount)
{
    ACE_UNUSED_ARG(threadCount);

    int expected = 10, value = WILD_INITIAL_VALUE;

    const int rotateValue = 20;

    DAF::RendezvousCommand<int> command;

    {

        DAF::Rendezvous<int> rend(2, command);
        TestRotator_ref tester(new TestRotator(rend, expected));

        {
            DAF::TaskExecutor executor;

            executor.execute(tester);

            if (rend.rendezvous(rotateValue) == expected) {
                if (tester->result == rotateValue) {
                    value = expected; // We pass
                }
            }

            DAF_OS::thr_yield();
        }

    }

    int result = (value == expected);

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
    int expected = 1, value = WILD_INITIAL_VALUE;

    DAF::Semaphore  blocker(0);
    TestRendFunc    functor;

    {
        RendezvousTest_t rend(threadCount, functor);
        {
            DAF::TaskExecutor executor;

            for (int i = 1; i < threadCount; ++i) {
                executor.execute(new TestRendezvous(blocker, i + 1, rend));
                expected += i + 1;
            }

            rend.rendezvous(1);

            DAF_OS::thr_yield();
        }

        value = functor.value;
    }

    int result = (value == expected);

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
    int expected = 0, value = WILD_INITIAL_VALUE;

    DAF::Semaphore  blocker(0);
    TestRendFunc    functor;

    {
        RendezvousTest_t *  rend(new RendezvousTest_t(threadCount, functor));
        TestRendezvous_ref  tester(new TestRendezvous(blocker, 1, *rend));

        {
            DAF::TaskExecutor executor;

            executor.execute(tester); blocker.acquire(); ++expected;

            for (int i = 2; i < threadCount; ++i) {
                executor.execute(new TestRendezvous(blocker, i, *rend)); blocker.acquire();
            }

            DAF_OS::sleep(1);

            delete rend; rend = 0;

            DAF_OS::thr_yield();
        }

        value = tester->interrupted;
    }

    int result = (value == expected);

    std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

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

    int expected = 1, value = WILD_INITIAL_VALUE;

    TestRendFunc functor;

    try {
        RendezvousTest_t(0, functor).rendezvous(1,100);
    }
    catch (const DAF::InitializationException &) {
        value = 1;
    }
    catch (const DAF::TimeoutException &) {
        value = 2;
    }
    catch (const std::exception &) {
        value = 3;
    }

    int result = (value == expected);

    std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

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
    int expected = 0, value = WILD_INITIAL_VALUE;

    DAF::Semaphore  blocker(0);
    TestRendFunc    functor;

    {
        RendezvousTest_t    rend(threadCount, functor);
        TestRendezvous_ref  tester(new TestRendezvous(blocker, 1, rend));
        {
            DAF::TaskExecutor executor, *kill_executor = new DAF::TaskExecutor;

            for (int i = 2; i < threadCount; ++i) {
                executor.execute(new TestRendezvous(blocker, i, rend)); blocker.acquire();
            }

            kill_executor->execute(tester); blocker.acquire(); ++expected;

            DAF_OS::sleep(1);

            delete kill_executor; kill_executor = 0;

            DAF_OS::thr_yield();
        }

        if (functor.ran ? true : false) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) ERROR: %C should not have run, but did!\n"), __FUNCTION__), 0);
        }

        value = tester->unknown; // We should have recieved an abi unwind
    }

    int result = (value == expected);

    std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

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

    result &= test_RendezvousBasicWorking(threadCount);
    result &= test_RendezvousTimeoutBroken(threadCount);
    result &= test_RendezvousTimeoutIllegal(threadCount);
    result &= test_RendezvousBroken(threadCount);
    result &= test_RendezvousCommand(threadCount);
    result &= test_RendezvousWaitResetClean(threadCount);
    result &= test_RendezvousWaitResetTimeoutClean(threadCount);
    result &= test_RendezvousWaitResetTimeout(threadCount);
    result &= test_RendezvousWaitResetHard(threadCount);
    result &= test_RendezvousDestruction(threadCount);
    result &= test_RendezvousCtorZero(threadCount);
    result &= test_RendezvousThreadKill(threadCount);

    return !result;
}
