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
#include "daf/Exception.h"

#include "ace/Get_Opt.h"
#include "ace/Refcounted_Auto_Ptr.h"
#include <iostream>

//
// Things to Test
// - setKeepAliveTime. Remaining threads hanging around.
// - size() . Is Accurate
// - Decay Time Threads
// - SVC thread vs Worker Thread conflicts
// -- being able to spawn worker threads prior to the svc thread
// -- Testing User Throwing
// - Singleton Testing. Testing the Interface ?
// - isActive. size > 0
// - isAvailable close
// - Constructor Testing
// -- Specific Thread Manager
// - Destruction,
// - Checks on close and dtor
//
namespace test
{
    int debug = DAF::debug();
    const char *TEST_NAME = "TaskExecutorTest";
    const time_t ATTEMPT_TIMEOUT = 1000;

    struct TestPause : DAF::Runnable
    {
        ACE_Time_Value delay;
        DAF::Semaphore &sema_counter;
        TestPause(const ACE_Time_Value & delay_in, DAF::Semaphore &sema_in) : DAF::Runnable()
        , delay(delay_in)
        , sema_counter(sema_in)
        {
        }

        int run(void)
        {
            sema_counter.release();
            if ( debug ) ACE_DEBUG((LM_INFO, "%t - %T Sleeping for Time %d ms\n", delay.msec()));
            DAF_OS::sleep(delay);
            if ( debug ) ACE_DEBUG((LM_INFO, "%t - %T Exit Sleep\n"));
            int count = sema_counter.permits();
            if ( debug ) ACE_DEBUG((LM_INFO, "%t - %T %d Exit Sleep\n", count));
            return 0;
        }
    };
    struct TestBasic : DAF::Runnable
    {
        int ran;
        TestBasic(): ran(0)
        {}
        virtual int run(void)
        {
            ++ran;
            return 0;
        }
    };
    struct TestBlocker : DAF::Runnable
    {
        DAF::Semaphore &sema;
        TestBlocker(DAF::Semaphore &sema_in) : sema(sema_in)
        {
        }
        virtual int run(void)
        {
            if ( debug) ACE_DEBUG((LM_DEBUG, "%t - Release   0x%08X\n", &sema));
            sema.release();
            return 0;
        }
    };

    struct TestAcquire : DAF::Runnable
    {
        DAF::Semaphore &sema;
        DAF::Semaphore &sema_count;

        TestAcquire(DAF::Semaphore &sema_count_in, DAF::Semaphore &sema_in) : sema(sema_in), sema_count(sema_count_in)
        {
        }
        virtual int run(void)
        {
            if ( debug ) ACE_DEBUG((LM_DEBUG, "%t - Acquiring 0x%08X\n", &sema));
            sema_count.release();
            sema.acquire();
            return 0;
        }
    };

    struct TestThrower : DAF::Runnable
    {
        DAF::Semaphore &sema_count;

        TestThrower(DAF::Semaphore &sema_in) : sema_count(sema_in)
        {
        }

        virtual int run(void)
        {
            if ( debug ) ACE_DEBUG((LM_DEBUG, "%t - %T Throwing\n"));
            sema_count.release();
            throw DAF::InternalException("Tester");

            return 0;
        }
    };

    struct TestExecutor : DAF::TaskExecutor
    {
        int svc_run;
        int argc;
        DAF::Semaphore &sema_block;
        DAF::Semaphore &sema_count;

        TestExecutor(DAF::Semaphore &sema_count_in, DAF::Semaphore &sema_block_in) : sema_block(sema_block_in), sema_count(sema_count_in)
        {
        }

        int init(int argc, ACE_TCHAR* argv[])
        {
            ACE_UNUSED_ARG(argv);
            this->argc = argc;
            return this->execute(this->argc);
        }

        int svc(void)
        {
            if(debug) ACE_DEBUG((LM_DEBUG, "%t - svcs \n"));
            ++svc_run;
            sema_count.release();
            sema_block.acquire();
            return 0;
        }
    };

    struct TestACETask : ACE_Task_Base
    {
        int long_or_short;
        DAF::Semaphore sema;
        DAF::Semaphore sema_post;

        TestACETask(): long_or_short(0), sema(0), sema_post(0)
        {
        }

        ~TestACETask(void)
        {
            if ( debug ) { ACE_DEBUG((LM_DEBUG, "dtor\n")); }
            this->module_closed();
        }

        int run(void)
        {
            this->activate(THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED, 1, 1);
            return 0;
        }

        int svc(void)
        {
            int grp_id = -1;
            if ( this->thr_mgr() )
            {
                ACE_thread_t id = this->thr_mgr()->thr_self();

               this->thr_mgr()->get_grp(id, grp_id);
               ACE_DEBUG((LM_DEBUG, "%t - GrpId %d\n", grp_id));
            }
            ACE_DEBUG((LM_DEBUG, "%t - Task GrpId %d\n", this->grp_id()));

            if ( grp_id != this->grp_id() )
            {
                ACE_DEBUG((LM_INFO, "ERROR Inconsistency in Thread GroupId %d %d\n", grp_id, this->grp_id()));
            }

            sema.release();
            if (long_or_short )
            {
                DAF_OS::sleep(ACE_Time_Value(1,0));
            }
            else
            {
                DAF_OS::sleep(ACE_Time_Value(0,100));
            }

            sema_post.release();

            return 0;
        }
    };

    struct TestTaskExecutorLongShort : DAF::TaskExecutor
    {
        int long_or_short;
        DAF::Semaphore sema;
        DAF::Semaphore sema_post;
        int thr_grp_id;

        TestTaskExecutorLongShort(): long_or_short(0), sema(0), sema_post(0)
        {
        }

        ~TestTaskExecutorLongShort(void)
        {
            if ( debug ) {
                ACE_DEBUG((LM_DEBUG, "dtor\n"));
            }
            this->module_closed();
        }

        int run(int threads = 1)
        {
            return this->execute(threads);
        }

        int svc(void)
        {
            if ( this->thr_mgr() )
            {
                ACE_thread_t id = this->thr_mgr()->thr_self();

                this->thr_mgr()->get_grp(id, thr_grp_id);
                if (debug) ACE_DEBUG((LM_DEBUG, "%t - GrpId %d\n", thr_grp_id));
            }

            if (debug) ACE_DEBUG((LM_DEBUG, "%t - Task GrpId %d\n", this->grp_id()));

            sema.release();
            if (long_or_short )
            {
                DAF_OS::sleep(ACE_Time_Value(1,0));
            }
            else
            {
                DAF_OS::sleep(ACE_Time_Value(0,100));
            }

            sema_post.release();

            return 0;
        }
    };

    /**
     * TEST
     *
     * Looking to understand if Size is Svc Threads + Worker Threads or Worker Threads
     *
     */
    int test_TaskExecutorSize(int threads)
    {
        int value = 0;
        int expected = threads + threads; // acquire threads + svc_thread + blocker_thread
        int result = 0;

        DAF::Semaphore blocker(0);
        DAF::Semaphore svc_blocker(0);
        DAF::Semaphore counter(0);

        try {

            TestExecutor executor(counter, svc_blocker);

            executor.init(threads,0);

            for ( int i =0; i < (threads); ++i )
            {
                counter.attempt(ATTEMPT_TIMEOUT);
            }

            for ( int i = 0 ; i < threads; ++i)
            {
                executor.execute(new TestAcquire(counter, blocker));
            }

            for ( int i =0; i < (threads); ++i )
            {
                counter.attempt(ATTEMPT_TIMEOUT);
            }

            if (debug) ACE_DEBUG((LM_INFO, "S1 Current Pool Size: %d\n", executor.size()));

            value = int(executor.size());

            if (debug) ACE_DEBUG((LM_INFO, "Releasing %d \n", threads));
            svc_blocker.release(threads);

            blocker.release(threads);

        } DAF_CATCH_ALL{
            ACE_DEBUG((LM_WARNING, ACE_TEXT("Exception caughtin %s\n"),__FUNCTION__)); expected = -1; // Forced Error Result
        }

         result = (value == expected);

         std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

    /**
     * TEST
     *
     * Looking to see the number of threads is decrementing correctly
     */
    int test_TaskExecutorSize_Svc_Dec(int threads)
    {
        int value = 0;
        int expected = 1; // acquire threads + svc_thread + blocker_thread
        int diff = 0;
        int result = 0;

        DAF::Semaphore blocker(0);
        DAF::Semaphore svc_blocker(0);
        DAF::Semaphore counter(0);

        try {

            TestExecutor executor(counter, svc_blocker);

            executor.init(threads,0);

            for ( int i =0; i < (threads); ++i )
            {
                counter.attempt(ATTEMPT_TIMEOUT);
            }

            for ( int i = 0 ; i < threads; ++i)
            {
                executor.execute(new TestAcquire(counter, blocker));
            }

            for ( int i =0; i < (threads); ++i )
            {
                counter.attempt(ATTEMPT_TIMEOUT);
            }

            if (debug) ACE_DEBUG((LM_INFO, "S1 Current Pool Size: %d\n", executor.size()));

            diff = int(executor.size());

            // Checking Debug on thread count --> output should decrement
            svc_blocker.release(1);

            DAF_OS::sleep(ACE_Time_Value(0, 50000));
            // TODO work out how we can sync off this ?

            value = int(diff - executor.size());

            if ( int(executor.size()) != (diff - 1) )
            {
                std::cout << __FUNCTION__ << " ERROR on Svc Thread Exit Expected " << diff-1 << " result " << executor.size() << std::endl;
            }

            if (debug) ACE_DEBUG((LM_INFO, "S2 Current Pool Size: %d\n", executor.size()));

            svc_blocker.release(1);
            DAF_OS::sleep(ACE_Time_Value(0, 50000));

            if ( int(executor.size()) != (diff - 2) )
            {
                std::cout << __FUNCTION__ << " ERROR on Svc Thread Exit Expected " << diff - 2 << " result " << executor.size() << std::endl;
            }

            if (debug) ACE_DEBUG((LM_INFO, "S3 Current Pool Size: %d\n", executor.size()));

            svc_blocker.release(threads-2);

            blocker.release(threads);

        } DAF_CATCH_ALL {
            ACE_DEBUG((LM_WARNING, ACE_TEXT("Exception caughtin %s\n"),__FUNCTION__)); expected = -1; // Forced Error Result
        }

         result = (value == expected);

         std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

    /**
     * TEST
     *
     * Looking to see there is any sensitivity to workers vs svc
     */
    int test_TaskExecutorSize_Wrk_Svc(int threads)
    {
        int value = 0;
        int expected = threads + threads;
        int result = 0;

        DAF::Semaphore blocker(0);
        DAF::Semaphore svc_blocker(0);
        DAF::Semaphore counter(0);

        try {

            TestExecutor executor(counter, svc_blocker);

            for ( int i = 0 ; i < threads; ++i)
            {
                executor.execute(new TestAcquire(counter, blocker));
            }

            for ( int i =0; i < (threads); ++i )
            {
                counter.attempt(1000);
            }

            // There is a problem here is using the ACE_Task_Base::activate without force on.
            // as the worker thread has already created the thread group. It will not activate the svc thread.
            // Your options are:
            // 1. use the TaskExecutor::execute(threads ) method which turns on active by default
            // 2. use the ACE_Task_Base::activate(X, X, force=1)
            //executor.init(threads,0);
            executor.execute(threads);

            for ( int i =0; i < (threads); ++i )
            {
                counter.attempt(ATTEMPT_TIMEOUT);
            }

            if (debug) ACE_DEBUG((LM_INFO, "S1 Current Pool Size: %d\n", executor.size()));

            value = int(executor.size());

            if (debug) ACE_DEBUG((LM_INFO, "S3 Current Pool Size: %d\n", executor.size()));

            svc_blocker.release(threads);

            blocker.release(threads);

        } DAF_CATCH_ALL {
            ACE_DEBUG((LM_WARNING, ACE_TEXT("Exception caughtin %s\n"),__FUNCTION__)); expected = -1; // Forced Error Result
        }

         result = (value == expected);

         std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

    /**
     * TEST
     *
     * Looking to see the number of threads is decrementing correctly
     */
    int test_TaskExecutorSize_WorkerDecay(int threads)
    {
        int value = 0;
        int expected = 1; // acquire threads + svc_thread + blocker_thread
        int diff = 2;
        int result = 0;

        DAF::Semaphore blocker(0);
        DAF::Semaphore svc_blocker(0);
        DAF::Semaphore counter(0);

        try {
            TestExecutor executor(counter, svc_blocker);

            executor.init(threads,0);

            for ( int i =0; i < (threads); ++i )
            {
                counter.attempt(ATTEMPT_TIMEOUT);
            }

            for ( int i = 0 ; i < threads; ++i)
            {
                executor.execute(new TestAcquire(counter, blocker));
            }

            for ( int i =0; i < (threads); ++i )
            {
                counter.attempt(ATTEMPT_TIMEOUT);
            }

            if (debug) ACE_DEBUG((LM_INFO, "S1 Current Pool Size: %d\n", executor.size()));

            diff = int(executor.size());

            svc_blocker.release(threads);

            blocker.release(threads);

            // Checking Debug on thread count --> output should decrement

            if (debug) ACE_DEBUG((LM_INFO, "Going for a LONG sleep... %d\n", DAF::TaskExecutor::THREAD_DECAY_TIMEOUT / 1000));

            while ( executor.size() > 0 )
            {
                if ( debug ) ACE_DEBUG((LM_DEBUG, "%T Current Pool Size %d\n", executor.size()));
                DAF_OS::sleep(ACE_Time_Value(DAF::TaskExecutor::THREAD_DECAY_TIMEOUT / 10000, 1000));
            }

            // DAF_OS::sleep(ACE_Time_Value(DAF::TaskExecutor::THREAD_DECAY_LIMIT/1000, 10000));
            // TODO work out how we can sync off this ?

            value = int(diff - executor.size());

            if ( int(executor.size()) != (diff - 1) )
            {
                std::cout << __FUNCTION__ << " ERROR on Svc Thread Exit Expected " << diff-1 << " result " << executor.size() << std::endl;
            }

            if (debug) ACE_DEBUG((LM_INFO, "S2 Current Pool Size: %d\n", executor.size()));

        } DAF_CATCH_ALL{
            ACE_DEBUG((LM_WARNING, ACE_TEXT("Exception caughtin %s\n"),__FUNCTION__)); expected = -1; // Forced Error Result
        }

         result = (value == expected);

         std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

     /**
     * TEST
     *
     * Checking for order of worker vs svc threads
     *
     */
    int test_TaskExecutor_WorkerThrow(int threads)
    {
        ACE_UNUSED_ARG(threads);
        int value = 0;
        int expected = 0;
        int result = 0;

        DAF::Semaphore counter(0);
        DAF::Semaphore svc_blocker(0);

        try {

            TestExecutor executor(counter, svc_blocker);

            executor.execute(new TestThrower(counter));

            counter.attempt(ATTEMPT_TIMEOUT);
            if (debug) ACE_DEBUG((LM_DEBUG, "%t - Did something Amazing Happen?\n"));

        } DAF_CATCH_ALL{
            ACE_DEBUG((LM_WARNING, ACE_TEXT("Exception caughtin %s\n"),__FUNCTION__)); expected = -1; // Forced Error Result
        }

        result = (value == expected);
        // Dummy Output
        std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

    /**
     * TEST
     *
     * Looking to understand whether the Keep Alive feature is working.
     * NOTE in the code it "ranges" the input from 1000 to one hour
     * So although I'm using 500 it will become 1000 internally.
     * Need to make note of this in the documentation. Also possibly suggest a change
     * to use THREAD_SAMPLE_LIMIT or RETRY_HANDOFF_LIMIT or a new one THREAD_DECAY_LOWER
     * to make it clearer.
     *
     */
    int test_TaskExecutor_KeepAlive(int threads)
    {
        int value = 1;
        int expected = 0;
        int result = 0;

        const int max_attempts = 10;

        const time_t TestKeepAliveTime_1 = 1000;
        const time_t TestKeepAliveTime_2 = 2000;

        DAF::Semaphore blocker(0);
        DAF::Semaphore svc_blocker(0);
        DAF::Semaphore counter(0);

        try {
            TestExecutor executor(counter, svc_blocker);
            executor.setDecayTimeout(TestKeepAliveTime_1);

            for ( int i = 0 ; i < threads; ++i)
            {
                executor.execute(new TestAcquire(counter, blocker));
            }

            for ( int i =0; i < (threads); ++i )
            {
                counter.attempt(ATTEMPT_TIMEOUT);
            }

            if (debug) ACE_DEBUG((LM_INFO, "S1 Current Pool Size: %d\n", executor.size()));

            blocker.release(threads);

            int attempts = 0;
            for ( attempts = 0; executor.size() > 0 && attempts < max_attempts; ++attempts)
            {
                if ( debug ) ACE_DEBUG((LM_DEBUG, "%T Current Pool Size %d\n", executor.size()));
                DAF_OS::sleep(ACE_Time_Value(0, TestKeepAliveTime_1 * 1000/(max_attempts-1)));
            }
            //value = (attempts < max_attempts);
            //DAF_OS::sleep(ACE_Time_Value(0, (TestKeepAliveTime_1 + 1) * 1000));

            if (debug) ACE_DEBUG((LM_INFO, "S2 Current Pool Size: %d\n", executor.size()));

            executor.setDecayTimeout(TestKeepAliveTime_2);

            for ( int i =0 ;i < threads; ++i )
            {
                executor.execute(new TestAcquire(counter,blocker));
            }

            for ( int i =0; i < threads; ++i )
            {
                counter.attempt(ATTEMPT_TIMEOUT);
            }

            blocker.release(threads);

            for ( attempts = 0; executor.size() > 0 && attempts < max_attempts; ++attempts)
            {
                if ( debug ) ACE_DEBUG((LM_DEBUG, "%T  2 Current Pool Size %d\n", executor.size()));
                DAF_OS::sleep(ACE_Time_Value(0, TestKeepAliveTime_2 * 1000/(max_attempts-1)));
            }

            if (debug) ACE_DEBUG((LM_INFO, "S3 Current Pool Size: %d\n", executor.size()));

            value = int(executor.size());

        } DAF_CATCH_ALL{
            ACE_DEBUG((LM_WARNING, ACE_TEXT("Exception caughtin %s\n"),__FUNCTION__)); expected = -1; // Forced Error Result
        }

        result = (value == expected);

        std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

    /**
     * TEST
     *
     * Looking to understand what happens when ACE_Task_Base::svc is running twice in
     * different time scales
     */
    int test_ACE_Task_Base(int threads )
    {
        int result = 0;
        ACE_UNUSED_ARG(threads);

        try {

            ACE_Refcounted_Auto_Ptr<TestACETask, ACE_SYNCH_MUTEX> task(new TestACETask);

            task->long_or_short = 0;
            if (debug)
            {
                ACE_DEBUG((LM_INFO, "%t - Task GrpId %d ThrCount %d\n", task->grp_id(), task->thr_count() ));
            }

            task->run();
            if (debug)
            {
                ACE_DEBUG((LM_INFO, "%t - Task GrpId %d ThrCount %d\n", task->grp_id(), task->thr_count()));
            }
            task->sema.acquire();
            task->sema_post.acquire();
            if (debug)
            {
                ACE_DEBUG((LM_INFO, "%t - Task GrpId %d ThrCount %d\n", task->grp_id(), task->thr_count()));
            }

            task->long_or_short = 1;

            task->run();
            DAF_OS::thr_yield();
            if (debug)
            {
                ACE_DEBUG((LM_INFO, "%t - Task GrpId %d ThrCount %d\n", task->grp_id(), task->thr_count()));
            }
            task->sema.acquire();
            if (debug)
            {
                ACE_DEBUG((LM_INFO, "%t - Task GrpId %d ThrCount %d\n", task->grp_id(), task->thr_count()));
            }

        } DAF_CATCH_ALL{
            ACE_DEBUG((LM_WARNING, ACE_TEXT("Exception caughtin %s\n"),__FUNCTION__)); result = -1;
        }

        return result;
    }

    /**
     * TEST
     *
     * Looking to understand if the TaskExecutor is the same
     */
    int test_TaskExecutor_GroupId(int threads)
    {
        ACE_UNUSED_ARG(threads);
        int value  = 0;
        int expected = -1;
        int result = 0;

        try {

            ACE_Refcounted_Auto_Ptr<TestTaskExecutorLongShort, ACE_SYNCH_MUTEX> task(new TestTaskExecutorLongShort);

            task->long_or_short = 0;
            if (debug)
            {
                ACE_DEBUG((LM_INFO, "%t - Task GrpId %d ThrCount %d\n", task->grp_id(), task->thr_count() ));
            }

            task->run();
            if (debug)
            {
                ACE_DEBUG((LM_INFO, "%t - Task GrpId %d ThrCount %d\n", task->grp_id(), task->thr_count()));
            }
            task->sema.acquire();
            task->sema_post.acquire();

            // Crappy test..
            while ( task->thr_count() > 0 )
            {
                DAF_OS::sleep(ACE_Time_Value(0, 100));
            }

            if (debug)
            {
                ACE_DEBUG((LM_INFO, "%t - Task GrpId %d ThrCount %d\n", task->grp_id(), task->thr_count()));
            }

            task->long_or_short = 1;

            task->run();
            DAF_OS::thr_yield();
            if (debug)
            {
                ACE_DEBUG((LM_INFO, "%t - Task GrpId %d ThrCount %d\n", task->grp_id(), task->thr_count()));
            }
            task->sema.acquire();
            expected = task->thr_grp_id;
            value = task->grp_id();
            if (debug)
            {
                ACE_DEBUG((LM_INFO, "%t - Task GrpId %d ThrCount %d\n", task->grp_id(), task->thr_count()));
            }

        } DAF_CATCH_ALL{
            ACE_DEBUG((LM_WARNING, ACE_TEXT("Exception caughtin %s\n"),__FUNCTION__)); expected = -1; // Forced Error Result
        }

         result = (value == expected);

         std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

     /**
     * TEST
     *
     * Looking to understand if the TaskExecutor is the same
     */
    int test_TaskExecutor_Dtor_Time(int threads)
    {
        ACE_UNUSED_ARG(threads);
        int value  = 1;
        int expected = 0;
        int result = 0;
        int grp_id = -1;

        const ACE_Time_Value delay((DAF::TaskExecutor::THREAD_EVICT_TIMEOUT * 3) / 1000);

        DAF::Semaphore counter(0);
        ACE_Thread_Manager *thr_man = 0;

        //DAF_Debug dbg(10);

        try {

            ACE_Refcounted_Auto_Ptr<TestTaskExecutorLongShort, ACE_SYNCH_MUTEX> task(new TestTaskExecutorLongShort());

            grp_id = task->grp_id();
            thr_man = task->thr_mgr();

            task->long_or_short = 1;

            task->run(threads);

            for ( int i =0; i < threads; ++i )
            {
                task->sema.acquire();
            }

            if (debug)
            {
                ACE_DEBUG((LM_INFO, "%t - Task GrpId %d ThrCount %d\n", task->grp_id(), task->thr_count() ));
            }

            for ( int i =0; i < threads; ++i )
            {
                task->execute(new TestPause(delay, counter));
            }

            for ( int i =0; i < threads; ++i )
            {
                counter.attempt(ATTEMPT_TIMEOUT);
            }

            // At this point all threads are up and "sleeping" (svc_threads + worker_threads)
            // Going to destroy the TaskExecutor.

        } DAF_CATCH_ALL{
            ACE_DEBUG((LM_WARNING, ACE_TEXT("Exception caughtin %s\n"),__FUNCTION__)); expected = -1; // Forced Error Result
        }

        DAF_OS::sleep(1);
        if ( thr_man && grp_id != -1)
        {
            const int THREAD_LIST_SIZE = 10;
            ACE_thread_t thread_list[THREAD_LIST_SIZE];

            value = int(thr_man->thread_grp_list(grp_id, thread_list, THREAD_LIST_SIZE));
            if (debug) ACE_DEBUG((LM_INFO, "%t - Main Checking GroupId %d thread Count %d\n", grp_id, value));
        }

         result = (value == expected);

         std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

     /**
     * TEST
     *
     * Looking to understand if the TaskExecutor is the same
     */
    int test_TaskExecutor_Dtor_Block(int threads)
    {
        ACE_UNUSED_ARG(threads);
        int value  = 1;
        int expected = 0;
        int result = 0;
        int grp_id = -1;

        const ACE_Time_Value delay(DAF::TaskExecutor::THREAD_EVICT_TIMEOUT / 1000 + 1);

        DAF::Semaphore counter(0);
        DAF::Semaphore blocker(0);

        ACE_Thread_Manager *thr_man = 0;

        try {

            ACE_Refcounted_Auto_Ptr<TestExecutor, ACE_SYNCH_MUTEX> executor(new TestExecutor(counter, blocker));

            grp_id = executor->grp_id();
            thr_man = executor->thr_mgr();

            executor->init(threads, 0);

            for ( int i =0; i < threads; ++i )
            {
                counter.attempt(ATTEMPT_TIMEOUT);
            }

            for ( int i =0; i < threads; ++i )
            {
                executor->execute(new TestAcquire(counter, blocker));
            }

            for ( int i =0; i < threads; ++i )
            {
                counter.attempt(ATTEMPT_TIMEOUT);
            }

            // At this point all threads are up and "sleeping" (svc_threads + worker_threads)
            // Going to destroy the TaskExecutor.
        } DAF_CATCH_ALL{
            ACE_DEBUG((LM_WARNING, ACE_TEXT("Exception caughtin %s\n"),__FUNCTION__)); expected = -1;
        }

        DAF_OS::sleep(1);
        if ( thr_man && grp_id != -1)
        {
            const int THREAD_LIST_SIZE = 10;
            ACE_thread_t thread_list[THREAD_LIST_SIZE];

            value = int(thr_man->thread_grp_list(grp_id, thread_list, THREAD_LIST_SIZE));
            if (debug) ACE_DEBUG((LM_INFO, "%t - Main Checking GroupId %d thread Count %d\n", grp_id, value));
        }

         result = (value == expected);

         std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

    void terminatefunction()
    {
        ACE_DEBUG((LM_ERROR, ACE_TEXT("termfunction was called by terminate.\n"))); exit(-1);
    }

    void unexpectedfunction()
    {
        ACE_DEBUG((LM_ERROR, ACE_TEXT("unexpectedfunction was called by unexpected.\n"))); exit(-1);
    }

    int test_TaskExecutor_Sync_Block(int threads)
    {
        //terminate_handler   oldterminatefunction    = set_terminate(terminatefunction);
        //unexpected_handler  oldunexpectedfunction   = set_unexpected(unexpectedfunction);

        do try {
            class SynchBlockTest : public DAF::TaskExecutor
            {
                DAF::Semaphore semaphore;

            public:
                SynchBlockTest(int threads) : semaphore(0)
                {
                    this->execute(threads);
                }

                ~SynchBlockTest(void)
                {
                    this->module_closed();
                }

                virtual int svc(void) {

                    try {
                        return this->semaphore.acquire();
                    } DAF_CATCH_ALL{}

                    return -1;
                }

            } synchExecutor_(threads); DAF_OS::sleep(10);

        } catch (const std::exception &ex) {
            ACE_ERROR_RETURN((LM_INFO, ACE_TEXT("(%P | %t) Exception %s\n"), ex.what()), -3);
        } DAF_CATCH_ALL {
            ACE_ERROR_RETURN((LM_INFO, ACE_TEXT("(%P | %t) Exception - Unknown\n")), -2);

        } while (false);

        //set_terminate(oldterminatefunction);
        //set_unexpected(oldunexpectedfunction);

        return 0;
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
    int result = 1;
    int threadCount = 2;
    ACE_Get_Opt cli_opt(argc, argv, "hzn:");
    cli_opt.long_option("help",'h', ACE_Get_Opt::NO_ARG);
    cli_opt.long_option("debug",'z', ACE_Get_Opt::NO_ARG);
    cli_opt.long_option("count",'n', ACE_Get_Opt::ARG_REQUIRED);

    for( int i = 0; i < argc; ++i ) switch(cli_opt()) {
        case -1: break;
        case 'h': print_usage(cli_opt); return 0;
        case 'z': DAF::debug(10); test::debug = DAF::debug(); break;
        case 'n': threadCount = DAF_OS::atoi(cli_opt.opt_arg());
    }

    ACE::debug(test::debug ? 1 : 0);

//    std::cout << test::TEST_NAME << std::endl;

    result &= test::test_TaskExecutorSize(threadCount);
    result &= test::test_TaskExecutorSize_Svc_Dec(threadCount);
    //result &= test::test_TaskExecutorSize_WorkerDecay(threadCount); // TOO LONG!
    result &= test::test_TaskExecutor_WorkerThrow(threadCount);
    result &= test::test_TaskExecutor_KeepAlive(threadCount);
    result &= test::test_TaskExecutor_GroupId(threadCount);
    result &= test::test_TaskExecutorSize_Wrk_Svc(threadCount);

    result &= test::test_TaskExecutor_Dtor_Time(threadCount);

#ifndef ACE_WIN32
    result &= test::test_TaskExecutor_Dtor_Block(threadCount);
    //result &= test::test_TaskExecutor_Sync_Block(threadCount);
#endif
    //result &= test::test_ACE_Task_Base(threadCount );

    return !result;
}
