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
#include "ace/Get_Opt.h"
#include "ace/Event_Handler.h"
#include "daf/Exception.h"
#include "daf/WFMOSignalReactor.h"
#include "daf/SignalHandler.h"
#include "daf/Event_Handler.h"
#include <iostream>
#include <sstream>


#define TIMER_COUNT  1
#define SIGNAL_COUNT 1
#define MAX_TIMER_COUNT 1024

/**
 * This Test is here to ensure the schedule_timer is working
 * It is a simple test that checks for multiple invocations
 */

class TimerHandlerTester : public DAF::SignalHandler
{
private:
  ACE_Time_Value interval_;
  ACE_Time_Value expected_;
  ACE_High_Res_Timer  hr1;
  ACE_High_Res_Timer hr2;
  ACE_Mutex mutex_;

public:
  long id;
  int invocation_count;
  ACE_Time_Value difference_;
  int load_;



  TimerHandlerTester(ACE_Reactor *reactor, ACE_Time_Value &delay, ACE_Time_Value &interval, int load = false) : DAF::SignalHandler(reactor)
  , interval_(interval)
  ,  id(-1)
  , invocation_count(0)
  , load_(load)
  {
    if ( !reactor ) {
        throw DAF::InitializationException("Failed to register Time with Reactor");
    }

    expected_ = expected_.now();

    this->id = reactor->schedule_timer(this, 0, delay, this->interval_);
    if (this->id == -1 ) {
        throw DAF::InitializationException("Invalid Timer Id Returned");
    }

    this->hr1.reset();
    this->hr2.reset();

    expected_ += delay;
  }

  ~TimerHandlerTester(void)
  {
    this->reactor()->cancel_timer(this->id);
  }




  virtual int handle_timeout(const ACE_Time_Value &current_time, const void *)
  {
    {
        ACE_Guard<ACE_Mutex> guard(this->mutex_);



        ACE_hrtime_t nanodiff;

        if( this->invocation_count % 2 ) {

            this->hr1.start();
            this->hr2.stop();
            this->hr2.elapsed_time(nanodiff);
        } else {
            this->hr2.start();
            this->hr1.stop();
            this->hr1.elapsed_time(nanodiff);
        }

        ACE_Time_Value expect_last = this->expected_;
        ACE_Time_Value diff(0,0);
        if ( current_time > this->expected_ ) {
            diff += current_time - this->expected_;
        } else {
            diff += this->expected_ - current_time;
        }

        this->difference_ += diff;

        this->expected_ = current_time + interval_;


        ++this->invocation_count;

        std::stringstream stream;
        stream << this->id <<  ", " << current_time << ", " << expect_last << ", " << nanodiff << ", " << diff  << std::endl;
        ACE_DEBUG((LM_INFO, "%t, %s", stream.str().c_str()));

        if ( this->load_) this->signal();


    }

    return 0;
  }

  virtual int handle_event(int , siginfo_t *, ucontext_t * )
  {
      //ACE_DEBUG((LM_INFO, "Handle Event TimerHandlerTester\n"));
      // put some load on the reactor thread
      ACE_Time_Value time;
      time = time.now();

      double sum = 0.0;
      for ( int i = 0 ;i < this->load_; ++i )
      //for ( int i = 0 ;i < 2000; ++i )
      {
        sum += DAF_OS::rand(10, 200);
      }
      ACE_Time_Value netime = time.now();

      std::stringstream stream;
      stream << netime - time ;

      ACE_DEBUG((LM_INFO, "%t, %s\n", stream.str().c_str()));

    return 0;
  }

};


void print_usage(const ACE_Get_Opt &cli_opt)
{
    ACE_UNUSED_ARG(cli_opt);  // TODO : Find a automated way of dumping the arguments from cli_opt ?


    std::cout << " WFMOTimer Test\n"
              << "-h --help            : Print this Message\n"
              << "-n --timers   <int>  : Number of Timers to use\n"
              << "-i --interval <int>  : Time Interval(ms) between scheduling\n"
              << "-t --threads  <int>  : Number of Threads\n"
              << "-l --load     <int>  : Load with <x> iterations\n"
              << "-d --duraction <int> : Test Duration (sec)\n"
              << std::endl;

}

int main(int argc, char *argv[])
{
  int num_of_timers = TIMER_COUNT;
  int interval_time = 10000;
  int num_of_threads = 5;
  int with_load = 0;
  int testDuration = 5;

  ACE_Get_Opt cli_opt(argc, argv, "hn:i:t:l:d:");
  cli_opt.long_option("help",'h', ACE_Get_Opt::NO_ARG);
  cli_opt.long_option("timers",'n', ACE_Get_Opt::ARG_REQUIRED);
  cli_opt.long_option("interval",'i', ACE_Get_Opt::ARG_REQUIRED);
  cli_opt.long_option("threads", 't', ACE_Get_Opt::ARG_REQUIRED);
  cli_opt.long_option("load", 'l', ACE_Get_Opt::ARG_REQUIRED);
  cli_opt.long_option("duration", 'd', ACE_Get_Opt::ARG_REQUIRED);


  for( int i = 0; i < argc; ++i )  {
    switch(cli_opt()) {
        case -1 : break;
        case 'h': print_usage(cli_opt); return 0;
        case 'n': num_of_timers = DAF_OS::atoi(cli_opt.opt_arg()); break;
        case 'i': interval_time = DAF_OS::atoi(cli_opt.opt_arg()) * 1000; break;
        case 't': num_of_threads = DAF_OS::atoi(cli_opt.opt_arg()); break;
        case 'l': with_load = DAF_OS::atoi(cli_opt.opt_arg()); break;
        case 'd': testDuration = DAF_OS::atoi(cli_opt.opt_arg()); break;
        default:  //???
            break;
    }
  }



  //ACE_DEBUG((LM_INFO, "Test Duration %d\n", testDuration));

  DAF::WFMOSignalReactor reactor; reactor.run(num_of_threads);

  ACE_Time_Value testDur(testDuration, 1000);

  ACE_Time_Value delay(1,0);
  ACE_Time_Value interval(0, interval_time);

  num_of_timers = (num_of_timers > MAX_TIMER_COUNT? MAX_TIMER_COUNT: num_of_timers);

  DAF::SignalHandler_ref timers[MAX_TIMER_COUNT];

  for ( int i = 0; i < num_of_timers; ++i ) {

    ACE_Time_Value start_jitter_i(0, DAF_OS::rand(10000,40000));
    ACE_Time_Value delay_i = delay + start_jitter_i;
    timers[i] = new TimerHandlerTester(&reactor, delay_i, interval,with_load);
  }


  reactor.run_reactor_event_loop(testDur);


  return 0;
}
