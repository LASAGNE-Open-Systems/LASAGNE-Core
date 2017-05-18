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
#include "daf/WFMOSignalReactor.h"
#include "daf/SignalHandler.h"
#include <iostream>

// Must be in DAF to negate template specialization error
// on the RefCount mismatch between ACE_Event_Handler
// valgrind is still reporting lost bytes on the SignalHandler
// which needs further investigation
namespace DAF
{
class TestSignalHandler;

template <> inline void
ObjectRefTraits<TestSignalHandler>::release(TestSignalHandler *&);

template <> inline TestSignalHandler *
ObjectRefTraits<TestSignalHandler>::duplicate(const TestSignalHandler *);

class TestSignalHandler : public DAF::SignalHandler
{
public:
  int invocation_count;
  ACE_High_Res_Timer timer;
  ACE_hrtime_t nanosum;

  DAF_DEFINE_REFCOUNTABLE(TestSignalHandler);

  TestSignalHandler() : DAF::SignalHandler()
  , invocation_count(0)
  , nanosum(0)
  {
  }

  TestSignalHandler(ACE_Reactor *reactor) : DAF::SignalHandler(reactor)
  , invocation_count(0)
  , nanosum(0)
  {
  }

  virtual int handle_event(int signum, siginfo_t *, ucontext_t *)
  {
    this->timer.stop();
    this->timer.elapsed_time(nanosum);
    ACE_UNUSED_ARG(signum);
    ++this->invocation_count;

    return 0;
  }

  double  average_calltime() const
  {
     return (double)this->nanosum/this->invocation_count;
  }
};

// Need to define these to help out with the cross-linkage between
// the reference counting in ACE_Event_Handler and
// DAF::RefCount
template <> inline void
ObjectRefTraits<TestSignalHandler>::release(TestSignalHandler *&p)
{
    if(p) {
      p->remove_reference();
      p = ObjectRefTraits<TestSignalHandler>::nil();
    }
}

template <> inline TestSignalHandler *
ObjectRefTraits<TestSignalHandler>::duplicate(const TestSignalHandler *p )
{
    if(p) {
      const_cast<TestSignalHandler*>(p)->add_reference();
    }

    return const_cast<TestSignalHandler*>(p);
}

DAF_DECLARE_REFCOUNTABLE(TestSignalHandler);

class TestGenerator : public ACE_Event_Handler
{
public:

  int invocation_count;
  TestSignalHandler_ref &signalUnderTest;

  TestGenerator(TestSignalHandler_ref &handler) : ACE_Event_Handler()
  , invocation_count(0)
  , signalUnderTest(handler)
  {
  }

  int handle_timeout(const ACE_Time_Value &current_time, const void *)
  {
    ACE_UNUSED_ARG(current_time);

    ++this->invocation_count;
    this->signalUnderTest->timer.start();
    this->signalUnderTest->signal();

    return 0;
  }
};
} //namespace DAF

using namespace DAF;

int main(int argc, char *argv[])
{
  ACE_UNUSED_ARG(argv);

  ACE_Time_Value testDur(1);
  ACE_Time_Value interval(0,20000);
  bool result = true;

  if ( argc > 1 )
  {
    // Explicit Reactor Construction
      DAF::WFMOSignalReactor reactor; reactor.run(1);
    TestSignalHandler_ref testHandler = new TestSignalHandler(&reactor);
    TestGenerator testGen(testHandler);

    long timer_id = reactor.schedule_timer(&testGen, 0, interval, interval);

    reactor.run_reactor_event_loop(testDur);
    reactor.cancel_timer(timer_id);

    int expected = testGen.invocation_count;
    int value = testHandler->invocation_count;

    result &= (expected != 0 ) && ( expected == value) ;

    std::cout << "WFMOSignalReactor SignalHandler Test:\n "
            << " Expected " << expected << " Got " << value
            << ( result ? "\tOK\n" : "\tFAILED\n") << std::endl;

    std::cout << "Average Signal Time: " << testHandler->average_calltime() << "ns" << std::endl;
  }
  else
  {
    // This one is here to test the WFMOReactor Singleton construction
    TestSignalHandler_ref testHandlerSingleton = new TestSignalHandler();
    TestGenerator testGen(testHandlerSingleton);

    long timer_id = SingletonWFMOSignalReactor::instance()->schedule_timer(&testGen, 0, interval, interval);

    SingletonWFMOSignalReactor::instance()->run_reactor_event_loop(testDur);
    SingletonWFMOSignalReactor::instance()->cancel_timer(timer_id);

    int expected = testGen.invocation_count;
    int value = testHandlerSingleton->invocation_count;

    result &= (expected != 0 )  && (expected == value);

    std::cout << "WFMOSignalReactor Singleton SignalHandler Test:\n "
            << " Expected " << expected << " Got " << value
            << ( result ? "\tOK\n" : "\tFAILED\n") << std::endl;
  }

  return !result;
}
