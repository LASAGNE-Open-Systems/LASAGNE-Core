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
#include "daf/SemaphoreControlledQueue_T.h"
#include "daf/TaskExecutor.h"
#include "ace/Thread.h"
#include "ace/Get_Opt.h"
#include <iostream>
#include <vector>
#include <algorithm>

#include "daf/Exception.h"

//
// Things to test
// - Thread Kill doesn't upset the order of things on teh take and put side
// - Timeout works on put and take side
// - capacity works
// - size is valid
//
namespace test
{
    bool debug = false;
    const char *TEST_NAME = "SemaphoreControlledChannelQueue";

    typedef DAF::SemaphoreControlledQueue<int> ChannelInt_t;
    typedef DAF::SynchronousChannel<int>       SyncChannelInt_t;

    time_t wait_time = 1000;
    bool throw_exception = false;

    struct LongAss
    {
        int value;
        LongAss(int val_in = 0) : value(val_in)
        {

        }

        LongAss& operator=(const LongAss& that)
        {
            if (this == &that)
                return *this;

            //
            if (debug)
            {
                ACE_DEBUG((LM_INFO, "(%P|%t) %T LongAss Assignment Waiting %d msecs\n", wait_time));
            }

            DAF_OS::sleep(ACE_Time_Value(wait_time/1000,(wait_time%1000)*1000));
            return *this;
        }
    };

    struct ThrowAss
    {
        int value;

        ThrowAss(int val_in = 0) : value(val_in)
        {

        }

        ThrowAss &operator=(const ThrowAss& that)
        {
            if ( this == &that)
                return *this;

            if ( debug )
            {
                ACE_DEBUG((LM_INFO, "(%P|%t) %T ThrowAss Throwing %d\n", throw_exception));
            }

            if ( throw_exception )
            {
                throw DAF::InternalException();
            }

            this->value = that.value;

            return *this;
        }
    };

    typedef DAF::SynchronousChannel<LongAss> SyncChannelLongAss_t;
    typedef DAF::SynchronousChannel<ThrowAss> SyncChannelThrowAss_t;

    template<typename T>
    struct TestTakerGen : DAF::Runnable
    {
        T value;
        int internal;
        int timeout;
        int notfound;
        int unknown;
        int result;
        DAF::Semaphore &sema;
        time_t timevalue;
        DAF::Channel<T> &channel;

        TestTakerGen(DAF::Semaphore &sema_in, DAF::Channel<T> &channel_in, time_t timeout_in = 0)
            : DAF::Runnable()
            , value(0)
            , internal(0)
            , timeout(0)
            , notfound(0)
            , unknown(0)
            , sema(sema_in)
            , timevalue(timeout_in)
            , channel(channel_in)
        {

        }

        virtual int run(void)
        {
            if ( debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T Taking from Channel 0x%08X\n", &this->channel));
            try
            {
                this->sema.release();
                if (this->timevalue)
                {
                    this->value = this->channel.poll(this->timevalue);
                }
                else
                {
                    this->value = this->channel.take();
                }

            } catch ( const DAF::TimeoutException &) {
                this->timeout++;
                if (debug) ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X Timeout\n"), this));
            } catch ( const DAF::InternalException & e) {
                this->internal++;
                if (debug) ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X Internal %s\n"), this,  e.what()));
            } DAF_CATCH_ALL {
                this->unknown++;
                if (debug) ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X Unknown \n"), this));
            }

            return 0;
        }
    };

    typedef TestTakerGen<int> TestTaker;

    template<typename T>
    struct TestPutterGen : TestTakerGen<T>
    {

        TestPutterGen(DAF::Semaphore &sema_in, DAF::Channel<T> &channel_in, time_t timeout_in = 0, T value_in = T())
            : TestTakerGen<T>(sema_in, channel_in, timeout_in)
        {
            this->value = value_in;
        }

        virtual int run(void)
        {
            if ( debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T Putter from Channel 0x%08X %d\n", &this->channel, this->value));
            try
            {
                this->sema.release();
                DAF_OS::thr_yield();

                if ( this->timevalue )
                {
                    this->result = this->channel.offer(this->value, this->timevalue);
                }
                else
                {
                    this->result = this->channel.put(this->value);
                }

                if ( this->result == -1 && errno == ETIME)
                {
                    this->timeout++;
                    if (debug) ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X Timeout\n"), this ));
                }

            } catch (const DAF::NotFoundException & e) {
                this->notfound++;
                if (debug) ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X NotFound %s\n"), this,  e.what()));
            } catch (const DAF::InternalException & e) {
                this->internal++;
                if (debug) ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X Internal %s\n"), this,  e.what()));
            } catch (const DAF::TimeoutException & e) {
                this->timeout++;
                if (debug) ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X Timeout %s\n"), this, e.what()));
            } DAF_CATCH_ALL {
                this->unknown++;
                if (debug) ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %T - 0x%08X Unknown \n"), this));
            }

            return 0;
        }
    };


    typedef TestPutterGen<int> TestPutter;

     /**
     * TEST
     *
     * Testing for size validity
     */
    int test_Channel_Size(int threadCount)
    {
        int capacity = (threadCount > 10 ? threadCount : 10);
        int result = 1;
        int expected = 4;
        int value = 0;

        ChannelInt_t channel(capacity);

        {
            for ( int i =0; i < expected; ++i)
            {
                channel.put(i);
            }

        }

        value = int(channel.size());

        result &= (value == expected);

        std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }


     /**
     * TEST
     *
     * Testing for Timeout
     */
    int test_Channel_TimeoutTaker(int threadCount)
    {
        int capacity = (threadCount > 10 ? threadCount+5 : 10);
        int result = 1;
        int expected = 1;
        int value = 0;


        DAF::Semaphore counter(0);
        ChannelInt_t channel(capacity);

        TestTaker *tester = new TestTaker(counter, channel, 500);
        DAF::Runnable_ref runner(tester);

        {
            DAF::TaskExecutor executor;

            executor.execute(runner);

            counter.acquire();

            DAF_OS::sleep(1);
        }

        value = tester->timeout;

        result &= (value == expected);

        std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }


     /**
     * TEST
     *
     * Testing for Timeout on putter side
     * Should consider changing this to a throw of TimeoutException to be consistent.
     */
    int test_Channel_TimeoutPutter(int )
    {
        int capacity = 1;
        int result = 1;
        int expected = 1;
        int value = 0;


        DAF::Semaphore counter(0);
        ChannelInt_t channel(capacity);

        channel.put(100);

        TestTaker *tester = new TestPutter(counter, channel, 500);
        DAF::Runnable_ref runner(tester);

        {
            DAF::TaskExecutor executor;

            executor.execute(runner);

            counter.acquire();

            DAF_OS::sleep(1);
        }

        value = tester->timeout;

        result &= (value == expected);

        std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

    /**
     * TEST
     *
     * Testing for Thread being Killed
     */
    int test_Channel_ThreadKill_Taker(int threadCount)
    {
        int capacity = (threadCount > 10 ? threadCount+5 : 10);
        int result = 1;
        int expected = 12345;
        int value = 0;


        DAF::Semaphore counter(0);
        ChannelInt_t channel(capacity);

        TestTaker *tester = new TestTaker(counter, channel);
        DAF::Runnable_ref runner(tester);

        {
            DAF::TaskExecutor executor;

            DAF::TaskExecutor *kill_executor = new DAF::TaskExecutor;

            // push our killer to exec first
            kill_executor->execute(new TestTaker(counter, channel));

            counter.acquire();

            executor.execute(runner);

            counter.acquire();

            // At this point my killer is sitting on the guard
            delete kill_executor;

            // does our channel still function
            channel.put(expected);

            DAF_OS::sleep(1);
        }

        value = tester->value;

        result &= (value == expected);

        std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }


     /**
     * TEST
     *
     * Testing for Thread being Killed - This test is a condition that
     * occurs when a thread is killed on the taker side. the unclaimedTakers_
     * state is not restored and a synchronous-nature is lost.
     */
    int test_SyncChannel_ThreadKill_LostSample(int )
    {
        int result = 1;
        int expected = 12345;
        int value = 0;


        DAF::Semaphore counter(0);
        SyncChannelInt_t channel;

        TestTaker *tester = new TestTaker(counter, channel);
        DAF::Runnable_ref runner(tester);

        {
            DAF::TaskExecutor executor;

            DAF::TaskExecutor *kill_executor = new DAF::TaskExecutor;

            // push our killer to exec first
            kill_executor->execute(new TestTaker(counter, channel));

            counter.acquire();

            DAF_OS::sleep(1);

            // At this point my killer is sitting on the guard
            // and we may have lost a value on the unclaimedTakers_ count
            delete kill_executor;

            executor.execute(new TestPutter(counter, channel, 0, expected));

            counter.acquire();

            executor.execute(new TestPutter(counter, channel, 500, 456));

            counter.acquire();

            // Run our tester
            executor.execute(runner);

            counter.acquire();

            DAF_OS::sleep(1);
        }

        value = tester->value;

        result &= (value == expected);

        std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

     /**
     * TEST
     *
     * Testing for Thread being Killed
     */
    int test_SyncChannel_ThreadKill_LostSample_Poll(int )
    {
        int result = 1;
        int expected = 12345;
        int value = 0;


        DAF::Semaphore counter(0);
        SyncChannelInt_t channel;

        TestTaker *tester = new TestTaker(counter, channel);
        DAF::Runnable_ref runner(tester);

        {
            DAF::TaskExecutor executor;

            DAF::TaskExecutor *kill_executor = new DAF::TaskExecutor;

            // push our killer to exec first
            kill_executor->execute(new TestTaker(counter, channel, DAF::TaskExecutor::THREAD_EVICT_TIMEOUT * 2));

            counter.acquire();

            // At this point my killer is sitting on the guard
            // and we may have lost a value on the unclaimedTakers_ count
            delete kill_executor;

            executor.execute(new TestPutter(counter, channel, 0, expected));

            counter.acquire();

            executor.execute(new TestPutter(counter, channel, 1500, 456));

            counter.acquire();

            // Run our tester
            executor.execute(runner);

            counter.acquire();

            DAF_OS::sleep(1);
        }

        value = tester->value;

        result &= (value == expected);

        std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

     /**
     * TEST
     *
     * Testing for Timeout
     *
     * This test means we must have the errno guard in place
     */
    int test_SyncChannel_Timeout_Poll(int )
    {
        int result = 1;
        int expected = 1;
        int value = 0;


        DAF::Semaphore counter(0);
        SyncChannelInt_t channel;

        TestTaker *tester = new TestTaker(counter, channel, 500);
        DAF::Runnable_ref runner(tester);

        {
            DAF::TaskExecutor executor;

            executor.execute(runner);

            counter.acquire();

            DAF_OS::sleep(1);
        }

        value = tester->timeout;

        result &= (value == expected);

        std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }

    /**
     * TEST
     *
     * Testing for SyncChannel incorrect error state on a user throws
     *
     */
    int test_SyncChannel_Putter_User_throw(int )
    {
        int result = 1;
        int expected = -1;
        int value = 0;


        DAF::Semaphore counter(0);

        // Long Assignment Channel
        SyncChannelThrowAss_t channel;

        ThrowAss put_value(123456);


        TestTakerGen<ThrowAss> *tester = new TestTakerGen<ThrowAss>(counter, channel);
        DAF::Runnable_ref runner(tester);

        DAF::Runnable_ref putter(new TestPutterGen<ThrowAss>(counter,channel, 0, put_value));

        {
            throw_exception = 1;
            DAF::TaskExecutor executor;

            executor.execute(runner);

            counter.acquire();



            executor.execute(putter);

            counter.acquire();


            DAF_OS::sleep(2);
        }

        value = tester->result;

        result &= (value == expected);

        std::cout << __FUNCTION__ <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

        return result;
    }


    /**
     * TEST
     *
     * Testing for SyncChannel incorrect error state on Timeout
     * To test this we are really using a LONG putter.
     * out Test harness will use a slow assginment on the type.
     * This should allow us to kill the putter thread in the middle
     * of the embrace and check the state
     * The major resolution here is the threads don't dead-lock. Particularly
     * on the Taker Side
     */
    int test_SyncChannel_ThreadKill_Putter_Taker_take(int )
    {
        int result = 1;
        int expected = 0;
        int value = 0;


        DAF::Semaphore counter(0);

        // Long Assignment Channel
        SyncChannelLongAss_t channel;

        LongAss put_value(123456);

        wait_time = DAF::TaskExecutor::THREAD_EVICT_TIMEOUT * 2;

        TestTakerGen<LongAss> *tester = new TestTakerGen<LongAss>(counter, channel);
        DAF::Runnable_ref runner(tester);

        {
            DAF::TaskExecutor executor;

            executor.execute(runner);

            counter.acquire();


            DAF::TaskExecutor *kill_executor = new DAF::TaskExecutor;

            // push our killer to exec first
            kill_executor->execute(new TestPutterGen<LongAss>(counter, channel, 0, put_value ));

            counter.acquire();

            // At this point the embrace should have started and we are sitting
            // on the assignment
            if ( debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T Killing Putter Thread\n"));
            delete kill_executor;
            if ( debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T Killing Putter Thread\n"));



            DAF_OS::sleep(2);
        }

        value = tester->value.value;

        result &= (value == expected);

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

    result &= test::test_Channel_Size(threadCount);
    result &= test::test_Channel_TimeoutTaker(threadCount);
    result &= test::test_Channel_TimeoutPutter(threadCount);

    result &= test::test_SyncChannel_Timeout_Poll(threadCount);
    result &= test::test_SyncChannel_Putter_User_throw(threadCount);
#ifndef ACE_WIN32
    result &= test::test_Channel_ThreadKill_Taker(threadCount);
    result &= test::test_SyncChannel_ThreadKill_LostSample(threadCount);
    result &= test::test_SyncChannel_ThreadKill_LostSample_Poll(threadCount);
    result &= test::test_SyncChannel_ThreadKill_Putter_Taker_take(threadCount);
#endif

    return !result;
}
