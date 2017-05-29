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
#include "daf/Monitor.h"
#include "daf/TaskExecutor.h"
#include "daf/CountDownSemaphore.h"
#include "daf/SemaphoreControlledQueue_T.h"
#include "ace/Get_Opt.h"
#include <iostream>

namespace test
{
  bool debug = false;
  const char *TEST_NAME = "MonitorTest";


  struct TestMonitorWait : DAF::Runnable
  {
    DAF::CountDownSemaphore &start_gate_;
    DAF::Monitor &monitor_;
    int illegal_;

    TestMonitorWait(DAF::CountDownSemaphore& start, DAF::Monitor &mon): start_gate_(start), monitor_(mon), illegal_(0)
    {

    }

    virtual int run(void)
    {
      int result = 0;
      try
      {
        this->start_gate_.release();
        //this->start_gate_.acquire();

        result = this->monitor_.wait();
      }
      catch (const DAF::InterruptedException & )
      {
        if (debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - Exception Thrown\n"));

        this->illegal_++;
      }
      if (debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - Wait Result %d\n", result));

      return 0;
    }


  };

  typedef DAF::SemaphoreControlledQueue<int> TestChannel_t;

  struct TestChannelTake : DAF::Runnable
  {
    DAF::CountDownSemaphore &start_gate_;

    TestChannel_t &channel_;
    int illegal_;


    TestChannelTake(DAF::CountDownSemaphore &start, TestChannel_t &channel) : start_gate_(start), channel_(channel), illegal_(0)
    {
    }

    virtual int run(void)
    {
      int result = 0;
      try
      {
        this->start_gate_.release();
        //this->start_gate_.acquire();

        result = this->channel_.take();
      }
      catch (const DAF::InterruptedException & )
      {
        if (debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - Exception Thrown\n"));

        this->illegal_++;
      }
      if (debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - Wait Result %d\n", result));

      return 0;

    }
  };

  /**
   * TEST
   *
   */
  int test_MonitorThrowDestruction(int threadCount)
  {


    ACE_UNUSED_ARG(threadCount);
    int result = 1;
    const int expected = 1;
    int value = 0;

    DAF::CountDownSemaphore counter(2);
    DAF::Monitor *monitor = new DAF::Monitor;
    TestMonitorWait *tester = new TestMonitorWait(counter, *monitor);
    DAF::Runnable_ref run_ref(tester);



    try
    {
      DAF::TaskExecutor executor;

      executor.execute(run_ref);

      counter.release();
      counter.acquire();




      // Kill the TaskExecutor - This should lead to a hard thread kill
      // Can the Barrier recover?
      if (debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - Killing the Monitor\n"));
      delete monitor;

      DAF_OS::sleep(ACE_Time_Value(1,0));



    }
    catch ( const DAF::IllegalThreadStateException& )
    {
      // Force a fail
      if (debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - Destruction Failed Monitor\n"));

      result = 0;
    }

    value = tester->illegal_  ;

    result &= (value == expected);

    std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

    return result;
  }

  /**
   * TEST
   *
   */
  int test_ChannelTake(int threadCount)
  {


    ACE_UNUSED_ARG(threadCount);
    int result = 1;
    const int expected = 1;
    int value = 0;

    DAF::CountDownSemaphore counter(2);
    TestChannel_t *channel = new TestChannel_t(10);
    TestChannelTake *tester = new TestChannelTake(counter, *channel);
    DAF::Runnable_ref run_ref(tester);

    try
    {
      DAF::TaskExecutor executor;

      executor.execute(run_ref);

      counter.release();
      counter.acquire();




      // Kill the TaskExecutor - This should lead to a hard thread kill
      // Can the Barrier recover?
      if (debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - Killing the Channel\n"));
      delete channel;

      DAF_OS::sleep(ACE_Time_Value(1,0));



    }
    catch ( const DAF::IllegalThreadStateException& )
    {
      // Force a fail
      if (debug) ACE_DEBUG((LM_DEBUG, "(%P|%t) %T - Destruction Failed Channel\n"));

      result = 0;
    }

    value = tester->illegal_  ;

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

  result &= test::test_MonitorThrowDestruction(threadCount);
  result &= test::test_ChannelTake(threadCount);



  return !result;
}
