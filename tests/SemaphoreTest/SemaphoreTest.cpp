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
#include "daf/Semaphore.h"
#include "daf/Runnable.h"
#include "daf/TaskExecutor.h"
#include "ace/Get_Opt.h"

#include <iostream>

//
// Tests for Semaphore.
// - Blocking on acquire
// - Timeout on acquire - return code -1
// +ve releases
// -ve releases
//

namespace test
{
    bool debug = false;
    const char *TEST_NAME = "SemaphoreTest";

    struct TestSemaphore : DAF::Runnable
    {
        DAF::Semaphore &sema;
        int permit_entry;
        int permit_exit;
        int result;

        TestSemaphore( DAF::Semaphore& sema_in) : sema(sema_in)
        {
        }

        virtual int run(void)
        {
            permit_entry = sema.permits();
            if ( debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T 0x%08X Acquire Semaphore %d\n", this, permit_entry ));
            result = sema.acquire();
            permit_exit = sema.permits();
            if ( debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T 0x%08X Acquire Semaphore %d\n", this, permit_exit));

            return 0;
        }
    };

    struct TestSemaphoreAttempt : DAF::Runnable
    {
        DAF::Semaphore &sema;
        int permit_entry;
        int permit_exit;
        int result;
        time_t timeout;

        TestSemaphoreAttempt( DAF::Semaphore& sema_in, time_t time) : sema(sema_in), timeout(time)
        {
        }

        virtual int run(void)
        {
            permit_entry = sema.permits();
            if ( debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T 0x%08X Acquire Semaphore %d %d\n", this, permit_entry, sema.waiters() ));
            result = sema.attempt(this->timeout);
            permit_exit = sema.permits();
          //  if ( debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T 0x%08X Acquire Semaphore %d %d %d\n", this, permit_exit, sema.waiters(), result));

            return 0;
        }
    };

    //
    // TEST
    //
    // Make sure Default Constructor is valid on the input permits
    //
    int test_SemaphoreCtorDefault()
    {
        int result = 1;
        int expected = 1;
        int value = -1;

        DAF::Semaphore sema;

        value = sema.permits();

        result &= (value == expected);

        std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

    //
    // TEST
    //
    // Make sure Constructor is valid on the input permits
    //
    int test_SemaphoreCtor()
    {
        int result = 1;
        int expected = 123;
        int value = 0;

        DAF::Semaphore sema(expected);

        value = sema.permits();

        result &= (value == expected);

        std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

    //
    // TEST
    //
    // Make sure the functionality of the Semaphore allows releasing negative values
    //
    int test_SemaphoreNegativeRelease()
    {
        int result = 1;
        int expected = -10;
        int value = 0;

        DAF::Semaphore sema(0);

        sema.release(expected);

        value = sema.permits();

        result &= (value == expected);

        std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

    //
    // TEST
    //
    // Make sure the functionality of the Semaphore allows releasing positive values
    //
    int test_SemaphorePositiveRelease()
    {
        int result = 1;
        int expected = 10;
        int value = 0;

        DAF::Semaphore sema(0);

        sema.release(expected);

        value = sema.permits();

        result &= (value == expected);

        std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

    //
    // TEST
    //
    // Make sure the functionality of the Semaphore allows timeout on a zero value
    //
    int test_SemaphoreAttemptTimeoutZero()
    {
        int result = 1;
        int expected = -1;
        int value = 0;

        DAF::Semaphore sema(0);

        const time_t timeout = 0;

        expected = sema.permits();
        result &= sema.attempt(timeout) == -1;
        value = sema.permits();

        result &= (value == expected);

        std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

    //
    // TEST
    //
    // Make sure the functionality of the Semaphore allows timeout on a valid timeout
    //
    int test_SemaphoreAttemptTimeout()
    {
        int result = 1;
        int expected = -1;
        int value = 0;

        DAF::Semaphore sema(0);

        const time_t timeout = 1000;

        expected = sema.permits();
        result &= sema.attempt(timeout) == -1;
        value = sema.permits();

        result &= (value == expected);

        std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

    //
    // TEST
    //
    // Make sure the functionality of the Semaphore allows
    //
    int test_SemaphoreAcquire()
    {
        int result = 1;
        int expected = 4;
        int value = 0;

        DAF::Semaphore sema(expected+1);

        result &= sema.acquire() == 0;
        value = sema.permits();
        result &= (value == expected);

        std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

    //
    // TEST
    //
    // Make sure the functionality of the Semaphore blocks on < 0 permits
    //
    int test_SemaphoreAcquireBlock()
    {
        int result = 1;
        int expected = 1;
        int value = 0;

        DAF::Semaphore sema(0);

        TestSemaphore *tester = new TestSemaphore(sema);

        DAF::Runnable_ref runner(tester);

        {
            DAF::TaskExecutor executor;

            executor.execute(runner);

            while(executor.size() < 1) {
                DAF_OS::sleep(ACE_Time_Value(0,300));
            }
            DAF_OS::sleep(ACE_Time_Value(1,300));

            sema.release();

            DAF_OS::sleep(ACE_Time_Value(1, 5000));
        }

        value = tester->permit_exit == tester->permit_entry;

        result &= (value == expected);

        std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

     //
    // TEST
    //
    // Make sure the functionality of the Semaphore blocks on < 0 permits
    //
    int test_SemaphoreThreadKill()
    {
        int result = 1;
        int expected = 1;
        int value = 0;

        DAF::Semaphore sema(0);

        TestSemaphore *tester = new TestSemaphore(sema);

        DAF::Runnable_ref runner(tester);

        {
            DAF::TaskExecutor executor;
            DAF::TaskExecutor *kill_executor = new DAF::TaskExecutor;

            executor.execute(runner);

            kill_executor->execute(new TestSemaphore(sema));

            while(executor.size() < 1 && kill_executor->size() < 1) {
                DAF_OS::sleep(ACE_Time_Value(0,300));
            }
            DAF_OS::sleep(ACE_Time_Value(1,300));

            // Kill the Executor
            if ( debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T - Killing Executor\n"));
            delete kill_executor;

            // At this point the semaphore should still function.
            // There should be only 1 waiter not 2.

            sema.release();

            DAF_OS::sleep(ACE_Time_Value(1, 5000));
        }

        value = tester->permit_exit == tester->permit_entry;

        result &= (value == expected);

        std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

    //
   // TEST
   //
   // This test examines the behaviour of the semaphore when it comes to
   // waiters. There have been errors in the past that will leave
   // waiters blocked.
   //
   int test_SemaphoreWaiterPrefer_NoBlock(const int threadCount)
   {
       int result = 1;
       int expected = threadCount - 2;
       int value = 0;

       DAF::Semaphore sema(0);
       if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T - BEGIN semaphore permits %d waiters %d\n", sema.permits(), sema.waiters()));

       TestSemaphore *tester = new TestSemaphore(sema);

       DAF::Runnable_ref runner(tester);

       {
           DAF::TaskExecutor executor;

           //executor.execute(runner);

           for(int i = 0 ;i < threadCount; ++i)
           {
             executor.execute(new TestSemaphore(sema));
           }

           // At this point we expect the Waiters to all be blocked on
           // acquiring the semaphore. N-threads should be the count in the
           // executor. if we release the semaphore we expect one of the
           // threads to be released. - There is a BUG that we are testing
           // on this condition where permits > waiters is used off the
           // wait. This should be permits > 0

           if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T - ST Executor Size %d semaphore permits %d waiters %d\n", executor.size(), sema.permits(), sema.waiters()));

           sema.release();

           if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T - R1 Executor Size %d semaphore permits %d waiters %d\n", executor.size(), sema.permits(), sema.waiters()));

           sema.release();

           if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T - R2 Executor Size %d semaphore permits %d waiters %d\n", executor.size(), sema.permits(), sema.waiters()));

           DAF_OS::sleep(ACE_Time_Value(0,5000));
           value = sema.waiters();

           // Clear out the rest - test is over.
           for (int i = 0; i< threadCount; ++i)
           {
             sema.release();
           }
       }

       result &= (value == expected);

       std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

       return result;
   }

   //
  // TEST
  //
  // This test examines the behaviour of the semaphore when it comes to
  // waiters. There have been errors in the past that will leave
  // waiters blocked. Same test as before but looking at the attempt interface.
  //
  int test_SemaphoreWaiterPrefer_NoBlockAttempt(const int threadCount)
  {
      int result = 1;
      int expected = threadCount - 2;
      int value = 0;

      DAF::Semaphore sema(0);
      time_t very_long= 10000;

      if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T - BEGIN semaphore permits %d waiters %d\n", sema.permits(), sema.waiters()));


      TestSemaphoreAttempt *tester = new TestSemaphoreAttempt(sema,very_long );

      DAF::Runnable_ref runner(tester);

      {
          DAF::TaskExecutor executor;

          //executor.execute(runner);

          for(int i = 0 ;i < threadCount; ++i)
          {
            executor.execute(new TestSemaphoreAttempt(sema,very_long));
          }

          // At this point we expect the Waiters to all be blocked on
          // acquiring the semaphore. N-threads should be the count in the
          // executor. if we release the semaphore we expect one of the
          // threads to be released. - There is a BUG that we are testing
          // on this condition where permits > waiters is used off the
          // wait. This should be permits > 0

          if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T - ST Executor Size %d semaphore permits %d waiters %d\n", executor.size(), sema.permits(), sema.waiters()));
          if (sema.permits() < 0 )
          {
            // force Failure
            ACE_DEBUG((LM_INFO, "(%P|%t) %T - Test FAILURE NEGATIVE permits %d waiters %d\n", sema.permits(), sema.waiters()));
            expected = -1;
          }


          sema.release();

          if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T - R1 Executor Size %d semaphore permits %d waiters %d\n", executor.size(), sema.permits(), sema.waiters()));

          sema.release();

          if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T - R2 Executor Size %d semaphore permits %d waiters %d\n", executor.size(), sema.permits(), sema.waiters()));
          DAF_OS::sleep(ACE_Time_Value(0,5000));
          value = sema.waiters();

          // Clear out the rest - test is over.
          for (int i = 0; i< threadCount; ++i)
          {
            sema.release();
          }
      }

      result &= (value == expected);

      std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

      return result;
  }



} //namespace test

void print_usage(const ACE_Get_Opt &cli_opt)
{
    ACE_UNUSED_ARG(cli_opt);
    std::cout << test::TEST_NAME
              << " -h --help              : Print this message \n"
              << " -z --debug             : Debug \n"
              << " -n --count             : Number of Threads/Test\n"
              << std::endl;
}

int main(int argc, char* argv[])
{
     int result = 1;
    size_t threadCount = 3;

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

    ACE_UNUSED_ARG(threadCount);

    std::cout << test::TEST_NAME << std::endl;

    result &= test::test_SemaphoreCtorDefault();
    result &= test::test_SemaphoreCtor();
    result &= test::test_SemaphorePositiveRelease();
    //result &= test::test_SemaphoreNegativeRelease();
    result &= test::test_SemaphoreAttemptTimeoutZero();
    result &= test::test_SemaphoreAttemptTimeout();
    result &= test::test_SemaphoreAcquire();
    result &= test::test_SemaphoreAcquireBlock();

    result &= test::test_SemaphoreWaiterPrefer_NoBlock(int(threadCount));
    result &= test::test_SemaphoreWaiterPrefer_NoBlockAttempt(int(threadCount));
    result &= test::test_SemaphoreThreadKill();
    return !result;
}
