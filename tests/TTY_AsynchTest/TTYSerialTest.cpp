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
//#include "ace/Asynch_IO.h"

#include "daf/ShutdownHandler.h"
#include "daf/Event_Handler.h"

// This only works on Win32 platforms and on Unix platforms supporting POSIX aio calls.
#if defined (ACE_HAS_WIN32_OVERLAPPED_IO) || defined (ACE_HAS_AIO_CALLS)

#include "TTY_Handler.h" // daf/TTY_Asynch/TTY_Handler.h
#include "TTY_Device.h"  // daf/TTY_Asynch/TTY_Device.h

#include <ace/Get_Opt.h>

#include <iostream>

/**
 * This test file was used a "loopback" serial device
 * It tests that a write and read are coupled.
 *
 * Some NOTES for safe keeping:
 * - Linux aio_read and aio_write can't be on the same file descriptor if you are
 *   wanting to execute them concurrently. See TTY_WriteStream for more info.
 * - DEV::TTY_Handler svc runs the proactor. This causes confusion if trying to run the
 *   Singleton Proactor from main. Using Reactor instead.
 */

#define BUFFER_SIZE 256

bool TEST_DEBUG = false;

namespace TEST
{
namespace //anonymous
{
#if defined(ACE_WIN32)
    const char DEFAULT_SERIAL_DEVICE_CONFIG[] = "COM1:baud=4800,parity=N,data=8,stop=1";
#else
    const char DEFAULT_SERIAL_DEVICE_CONFIG[] = "/dev/ttyS0:baud=4800,parity=N,data=8,stop=1";
#endif
}//namespace anonymous

/**
 *  TestSerialHandler  Application level Tester for
 *
 */
class TestSerialHandler : public DEV::TTY_Handler
{
    std::string current_;
    std::string match_;
    int count_;

public:
    TestSerialHandler(const std::string &match) : TTY_Handler()
    , current_()
    , match_(match)
    , count_(0)
    {
    }

    virtual ~TestSerialHandler(void)
    {
    }

    virtual int get_count(void) const
    {
        return this->count_;
    }

    virtual int process_input(ACE_HANDLE handle )
    {
        //ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) %d process_input\n"), handle));
        if ( this->is_active() ) {
            char buffer[BUFFER_SIZE];
            int wstream_len = int(this->write_stream.length());
            int rstream_len = int(this->read_stream.length());

            int num = this->read_stream.get(buffer, BUFFER_SIZE);

            if ( num > 0 ) {
                if ( TEST_DEBUG ) {
                    ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) %d process_input WR %d RD %d %T data(%d) %s\n")
                        , handle, wstream_len, rstream_len, num, DAF::hex_dump(buffer, num).c_str()));
                }

                current_ += std::string(buffer, num);
                size_t index = current_.find(this->match_);
                if ( index != std::string::npos ) {
                    this->count_++;
                    current_ = current_.substr(index + this->match_.length());
                    if ( TEST_DEBUG ) {
                        std::cout << "**" <<  this->count_ << " Current : '" << current_ << std::endl;
                    }
                }

                // push the read pointer up.
                this->read_stream.get_bump(num);
            }
        }
        return 0;
    }

    virtual int process_output(ACE_HANDLE handle )
    {
        if (TEST_DEBUG) {
            ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) %d process_output %T\n"), handle));
        }

        return 0;
    }
};

/**
 * TestSerialDevice
 */
class TestSerialDevice : public DEV::TTY_Device
{
public:
    TestSerialDevice(const std::string &testStr);

    virtual ~TestSerialDevice(void);

    virtual int open(void *args = 0 )
    {
        return TTY_Device::open(args) ? -1 : this->execute(1);
        return -1;
    }

protected:

private:
    virtual int svc(void)
    {
        return 0;
    }
};

TestSerialDevice::TestSerialDevice(const std::string &testStr) : DEV::TTY_Device(new TestSerialHandler(testStr), true)
{
    this->tty_params.baudrate   = 4800;
    this->tty_params.paritymode = "none";
    this->tty_params.databits   = 8;
    this->tty_params.stopbits   = 1;
    this->tty_params.modem      = true;
    this->tty_params.ctsenb     = false;
    this->tty_params.rtsenb     = true;
    this->tty_params.rcvenb     = true;
    this->tty_params.dsrenb     = true;
    this->tty_params.readmincharacters = 1;
    this->tty_params.readtimeoutmsec = 10;

    this->tty_device_config.assign(DEFAULT_SERIAL_DEVICE_CONFIG);
}

TestSerialDevice::~TestSerialDevice(void)
{
    this->module_closed();
}

/**
 * TestTimerWrite
 *
 * A simple writer to the device based on Reactor timeouts
 */
class TestTimerWrite : public DAF::Event_Handler
{
    const std::string testStr_;

public:

    int count;

    TestTimerWrite(const std::string &testStr) : testStr_(testStr)
    , count(0)
    {
    }

    virtual int handle_timeout(const ACE_Time_Value &current, const void *device_ptr )
    {
        ACE_UNUSED_ARG(current);

        TEST::TestSerialDevice *device = const_cast<TEST::TestSerialDevice*>(reinterpret_cast<const TEST::TestSerialDevice *>(device_ptr));
        if ( device ) {
            if ( TEST_DEBUG ) {
                ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) Writing Test %T '%s'\n"), testStr_.c_str()));
            }

            device->write_data(testStr_.c_str(), int(testStr_.length() + 1));

            this->count++;
        }

        return 0;
    }
};
}//namespace TEST

void print_usage(const ACE_Get_Opt &cli_opt)
{
    ACE_UNUSED_ARG(cli_opt);
    std::cout << " TTYSerailTest\n"
              << " -h --help              : Print this message \n"
              << " -a --device <dev>      : change the Device String\n"
              << " -z --debug             : Debug (NOT USED?)\n"
              << " -m --message <string>  : Message to Send Test\n"
              << " -d --duration <int>    : Test Duration \n"
              << " -i --interval <int>    : Time Interval(ms) between Sends\n"
              << std::endl;
}

int main ( int argc, char *argv[] )
{
    int testDuration = 3;
    std::string testString = "ping pong toast";
    int interval_time = 100000;

    ACE_Get_Opt cli_opt(argc, argv, "ha:zm:d:");
    cli_opt.long_option("help",'h', ACE_Get_Opt::NO_ARG);
    cli_opt.long_option("device",'a', ACE_Get_Opt::ARG_REQUIRED);
    cli_opt.long_option("debug",'z', ACE_Get_Opt::ARG_REQUIRED);
    cli_opt.long_option("interval",'i', ACE_Get_Opt::ARG_REQUIRED);
    cli_opt.long_option("message", 'm', ACE_Get_Opt::ARG_REQUIRED);
    cli_opt.long_option("duration", 'd', ACE_Get_Opt::ARG_REQUIRED);

    for ( int i = 0; i < argc; ++i )  switch(cli_opt()) {
        case -1: break;
        case 'h': print_usage(cli_opt); return 0;
        case 'a': break; // handled by device;
        case 'z': DAF::debug(true); TEST_DEBUG = true; break;
        case 'i': interval_time = DAF_OS::atoi(cli_opt.opt_arg()) * 1000; break;
        case 'd': testDuration = DAF_OS::atoi(cli_opt.opt_arg()); break;
        case 'm': testString = cli_opt.opt_arg(); break;
    }

    ACE_Time_Value testTime(testDuration, interval_time + 100);
    ACE_Time_Value testInterval(0 , interval_time );
    TEST::TestSerialDevice device(testString);
    TEST::TestTimerWrite writer(testString);

    device.init(argc, argv);

    device.open(&device);

    long id = ACE_Reactor::instance()->schedule_timer(&writer, &device, testInterval, testInterval);

    ACE_Reactor::instance()->run_reactor_event_loop(testTime);
    ACE_Reactor::instance()->cancel_timer(id);

    // going to do a short sleep to ensure the
    // aio is cleared out and we don't miss the
    // last trigger.
    DAF_OS::sleep(testInterval);

    const int expected = writer.count;
    const int result = reinterpret_cast<TEST::TestSerialHandler*>(device.get_device_handler())->get_count();

    std::cout << " Expected " << expected << " Result " << result
              << " Test " << (result == expected ? "OK" : "FAILED") << std::endl;

    // Be friendly, coz I'm going to forget this...
    if ( result == 0 ) {
        std::cout << " Looks like you got nothing back from the TTYSerial. Did you connect a Serial 'Loopback' to the COM port?\n Maybe turn on debugging to Identity the problem. See --help for more info" << std::endl;
    }

    // Basic exercising of Status method.
    const DEV::TTY_Device::ModemStatus &status = device.get_status();
    std::cout << status;

    return !(result == expected);
}

#else
# include "ace/Log_Msg.h"

int main(int argc, char* argv[])
{
    ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("This only works on Win32 platforms and on Unix platforms supporting POSIX aio calls.\n")), -1);
}

#endif // defined (ACE_HAS_WIN32_OVERLAPPED_IO) || defined (ACE_HAS_AIO_CALLS)
