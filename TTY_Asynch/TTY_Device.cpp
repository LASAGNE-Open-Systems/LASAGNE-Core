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
#define DEV_TTY_DEVICE_CPP

#include "TTY_Device.h"

#if defined(ACE_HAS_WIN32_OVERLAPPED_IO) || defined(ACE_HAS_AIO_CALLS)

#include "TTY_Handler.h"

#include "ace/ARGV.h"
#include "ace/Min_Max.h"
#include "ace/Get_Opt.h"

#define TTY_MIN_TIMEOUT_MSEC    int(1)
#define TTY_MAX_TIMEOUT_MSEC    int(TTY_MIN_TIMEOUT_MSEC * 500)

#if defined(ACE_WIN32)

#define MODEM_CTS_ON   MS_CTS_ON
#define MODEM_DSR_ON   MS_DSR_ON
#define MODEM_RING_ON  MS_RING_ON
#define MODEM_RLSD_ON  MS_RLSD_ON

#elif defined(ACE_HAS_TERMIOS)

#define MODEM_CTS_ON   TIOCM_CTS
#define MODEM_DSR_ON   TIOCM_DSR
#define MODEM_RING_ON  TIOCM_RNG
#define MODEM_RLSD_ON  TIOCM_CAR

#else

#warning "TTY ModemStatus is not supported on this platform"

#define MODEM_CTS_ON   0
#define MODEM_DSR_ON   0
#define MODEM_RING_ON  0
#define MODEM_RLSD_ON  0

#endif

namespace {
#if defined(ACE_WIN32)
    const char DEFAULT_DEVICE_CONFIG[]   = "COM1:baud=9600,parity=N,data=8,stop=1";
#else
    const char DEFAULT_DEVICE_CONFIG[]   = "/dev/ttyUSB0:baud=9600,parity=N,data=8,stop=1";
#endif

    const double RD_TIMEOUT_SPACING = 5.0;
    const double WT_TIMEOUT_SPACING = 1.5;

    int     TTY_IO_TIMEOUT(int baudrate, double spacing)
    {
        return ace_range(TTY_MIN_TIMEOUT_MSEC, TTY_MAX_TIMEOUT_MSEC, int((spacing * 10000.0) / baudrate));
    }
}

namespace DEV
{
    TTY_Device::ModemStatus::ModemStatus(unsigned int status) : modem_status(status)
    {
    }

    unsigned int
    TTY_Device::ModemStatus::getStatus(void) const
    {
        return this->modem_status;
    }

    bool
    TTY_Device::ModemStatus::CTS(void) const
    {
        return (this->modem_status & MODEM_CTS_ON);
    }

    bool
    TTY_Device::ModemStatus::DSR(void) const
    {
        return (this->modem_status & MODEM_DSR_ON);
    }

    bool
    TTY_Device::ModemStatus::RING(void) const
    {
        return (this->modem_status & MODEM_RING_ON);
    }

    bool
    TTY_Device::ModemStatus::RLSD(void) const
    {
        return (this->modem_status & MODEM_RLSD_ON);
    }

    TTY_Asynch_Export std::ostream &
    operator << (std::ostream &os, const TTY_Device::ModemStatus &ms)
    {
        os  << std::endl
            << "{CTS="  << ms.CTS()  << '}'
            << "{DSR="  << ms.DSR()  << '}'
            << "{RING=" << ms.RING() << '}'
            << "{RLSD=" << ms.RLSD() << '}'
            << std::endl;
        return os;
    }

#if defined(ACE_WIN32)
#endif
    enum TTY_PARITY {
        TTY_PARITY_NONE, TTY_PARITY_ODD, TTY_PARITY_EVEN  // POSIX supports "none", "odd" and "even" parity
#if defined(ACE_WIN32)
        , TTY_PARITY_MARK, TTY_PARITY_SPACE  // Additionally Win32 supports "mark" and "space" parity modes
#endif
    };

    static const char *TTY_PARITY_STR[] = {
        "none","odd","even"  // POSIX supports "none", "odd" and "even" parity.
#if defined(ACE_WIN32)
        ,"mark","space"  // Additionally Win32 supports "mark" and "space" parity modes.
#endif
    };

    TTY_Device::TTY_Device(TTY_Handler *handler, bool delete_handler) : ACE_TTY_IO()
        , tty_handler       (handler)
        , delete_handler_   (delete_handler)
        , device_active_    (false)
        , device_debug_     (0)
        , tty_device_config (DEFAULT_DEVICE_CONFIG)
        , modem_status_     (0)
    {
#if defined(ACE_HAS_TERMIOS)
        this->tty_params.readmincharacters  = int(RX_DEVICE_BUFFER / 4);
#endif
        this->tty_params.dtrdisable = false;
    }

    TTY_Device::~TTY_Device(void)
    {
        this->module_closed();

        if (this->delete_handler_) {
            delete this->tty_handler;
        }

        ACE_TTY_IO::close();
    }

    void
    TTY_Device::set_handle(ACE_HANDLE handle)
    {
        ACE_TTY_IO::set_handle(handle);
    }

    ACE_HANDLE
    TTY_Device::get_handle(void) const
    {
        return ACE_TTY_IO::get_handle();
    }

    const TTY_Device::ModemStatus&
    TTY_Device::get_status() const
    {
#if defined(ACE_WIN32)
        DWORD m_status = 0; if (::GetCommModemStatus(this->get_handle(), &m_status) == 0) {
#elif defined(ACE_HAS_TERMIOS)
        unsigned int m_status = 0; if (ACE_IO_SAP::control(TIOCMGET, &m_status)) {
#else
        unsigned int m_status = 0;
        {
            ACE_ERROR((LM_ERROR, ACE_TEXT("TTY_Device::get_status() is not supported on this platform\n")));
            // throw NotImplemented ?
#endif
            DAF::print_last_error();
        }

        return this->modem_status_ = TTY_Device::ModemStatus(m_status);
    }

    int
    TTY_Device::parse_config(const std::string &config_arg)
    {
        // "COMx[:][baud=b][,parity=p][,data=d][,stop=s]\n\t\t[,to={1|0}][,xon={1|0}][,odsr={1|0}][,octs={1|0}]\n\t\t[,dtr={1|0|hs}][,rts={1|0|hs|tg}][,idsr={1|0}]\n"

        const std::string &tty_config(this->tty_device_config.assign(DAF::trim_string(config_arg, '"')));

        int pos = int(tty_config.find_first_of(':'));

        if (0 > pos) {   // Only Device specified
            if (tty_config.length() > 0) {
#if defined(ACE_WIN32)
                this->tty_device_name = std::string("\\\\.\\").append(DAF::trim_string(tty_config,'\\')); return 0;
#else
                this->tty_device_name = std::string(DAF::trim_string(tty_config,'\\')); return 0;
#endif
            }
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: Invalid TTY Device specified -\n\t\"%s\"")
                , tty_config.c_str()),-1);
        }

        // Set Up the device address.
#if defined(ACE_WIN32)
        this->tty_device_name.assign(std::string("\\\\.\\").append(DAF::trim_string(tty_config.substr(0,pos++),'\\')));
#else
        this->tty_device_name.assign(DAF::trim_string(tty_config.substr(0,pos++),'\\'));
#endif

        // Process the device Parameters

        do {
            int e_pos = int(tty_config.find_first_of(',', pos));

            std::string param_arg, param_val, param(DAF::trim_string(tty_config.substr(pos, e_pos -= pos)));

            int eq_pos = int(param.find_first_of('=')); pos += e_pos;

            if (0 >= eq_pos) continue;

            param_arg = DAF::trim_string(param.substr(0, eq_pos++));
            param_val = DAF::trim_string(param.substr(eq_pos));

            if (param_arg.length() == 0 || param_val.length() == 0) {
                continue;
            } else if (DAF_OS::strcasecmp(param_arg.c_str(), "baud") == 0) {  // Defaults to 9600
                int baudrate = DAF_OS::atoi(param_val.c_str()); if (baudrate > 0) {
                    this->tty_params.baudrate = ace_range(50, 256000, DAF_OS::atoi(param_val.c_str()));
                } else this->tty_params.baudrate = 9600;
            } else if (DAF_OS::strcasecmp(param_arg.c_str(), "parity") == 0) {  // Defaults to ACE_TTY_IO_NONE
                if        (param_val[0] == '1' || param_val == "O" || DAF_OS::strcasecmp(param_val.c_str(), TTY_PARITY_STR[TTY_PARITY_ODD])     == 0) {
                    this->tty_params.paritymode = TTY_PARITY_STR[TTY_PARITY_ODD];
                } else if (param_val[0] == '2' || param_val == "E" || DAF_OS::strcasecmp(param_val.c_str(), TTY_PARITY_STR[TTY_PARITY_EVEN])    == 0) {
                    this->tty_params.paritymode = TTY_PARITY_STR[TTY_PARITY_EVEN];
#if defined(ACE_WIN32)
                } else if (param_val[0] == '3' || param_val == "M" || DAF_OS::strcasecmp(param_val.c_str(), TTY_PARITY_STR[TTY_PARITY_MARK])    == 0) {
                    this->tty_params.paritymode = TTY_PARITY_STR[TTY_PARITY_MARK];
                } else if (param_val[0] == '4' || param_val == "S" || DAF_OS::strcasecmp(param_val.c_str(), TTY_PARITY_STR[TTY_PARITY_SPACE])   == 0) {
                    this->tty_params.paritymode = TTY_PARITY_STR[TTY_PARITY_SPACE];
#endif
                } else this->tty_params.paritymode = TTY_PARITY_STR[TTY_PARITY_NONE];
            } else if (DAF_OS::strcasecmp(param_arg.c_str(), "data") == 0) {  // Defaults to 8
                int databits = DAF_OS::atoi(param_val.c_str()); switch (databits) {
                default: databits = 8; // Otherwise Use 8
                case 4 :
                case 5 :
                case 6 :
                case 7 :
                case 8 : this->tty_params.databits = databits;
                }
            } else if (DAF_OS::strcasecmp(param_arg.c_str(), "stop") == 0) {  // Defaults to 1
                int stopbits = DAF_OS::atoi(param_val.c_str()); switch (stopbits) {
                default: stopbits = 1; // Otherwise Use 1
                case 1 :
                case 2 : this->tty_params.stopbits = stopbits;
                }
            } else if (DAF_OS::strcasecmp(param_arg.c_str(), "to") == 0) {
                int readtimeoutmsec = DAF_OS::atoi(param_val.c_str()); if (readtimeoutmsec > 0) {
                    this->tty_params.readtimeoutmsec = readtimeoutmsec;
                } else this->tty_params.readtimeoutmsec = 10000;
            } else if (DAF_OS::strcasecmp(param_arg.c_str(), "xon") == 0) {
                if (param_val[0] == '1' || DAF_OS::strcasecmp(param_val.c_str(), "on") == 0) {
                    this->tty_params.xinenb = this->tty_params.xoutenb = true;
                } else this->tty_params.xinenb = this->tty_params.xoutenb = false;
            } else if (DAF_OS::strcasecmp(param_arg.c_str(), "xin") == 0) {
                if (param_val[0] == '1' || DAF_OS::strcasecmp(param_val.c_str(), "on") == 0) {
                    this->tty_params.xinenb = true;
                } else this->tty_params.xinenb = false;
            } else if (DAF_OS::strcasecmp(param_arg.c_str(), "xout") == 0) {
                if (param_val[0] == '1' || DAF_OS::strcasecmp(param_val.c_str(), "on") == 0) {
                    this->tty_params.xoutenb = true;
                } else this->tty_params.xoutenb = false;
            } else if (DAF_OS::strcasecmp(param_arg.c_str(), "xonlim") == 0)    {
                int xonlim = DAF_OS::atoi(param_val.c_str()); if (xonlim > 0)   {
                    this->tty_params.xonlim = xonlim;
                } else this->tty_params.xonlim = 0;
            } else if (DAF_OS::strcasecmp(param_arg.c_str(), "xofflim") == 0)   {
                int xofflim = DAF_OS::atoi(param_val.c_str()); if (xofflim > 0) {
                    this->tty_params.xofflim = xofflim;
                } else this->tty_params.xofflim = 0;
            } else if (DAF_OS::strcasecmp(param_arg.c_str(), "odsr") == 0) { // Enable/disable DSR protocol.
                if (param_val[0] == '1' || DAF_OS::strcasecmp(param_val.c_str(), "on") == 0) {
                    this->tty_params.dsrenb = true;
                } this->tty_params.dsrenb = false;
            } else if (DAF_OS::strcasecmp(param_arg.c_str(), "octs") == 0) { // Enable/disable CTS protocol.
                if (param_val[0] == '1' || DAF_OS::strcasecmp(param_val.c_str(), "on") == 0) {
                    this->tty_params.ctsenb = true;
                } this->tty_params.ctsenb = false;
            } else if (DAF_OS::strcasecmp(param_arg.c_str(), "dtr") == 0)  { // Disable/enable DTR protocol
                if (param_val[0] == '1' || DAF_OS::strcasecmp(param_val.c_str(), "on") == 0) {
                    this->tty_params.dtrdisable = true;
                } this->tty_params.dtrdisable = false;
            } else if (DAF_OS::strcasecmp(param_arg.c_str(), "rts") == 0)  {
                if (param_val[0] == '1' || DAF_OS::strcasecmp(param_val.c_str(), "on") == 0) {
                    this->tty_params.rtsenb = 1;
                } else if (param_val[0] == '2' || DAF_OS::strcasecmp(param_val.c_str(), "hs") == 0) {
                    this->tty_params.rtsenb = 2;
                } else if (param_val[0] == '3' || DAF_OS::strcasecmp(param_val.c_str(), "tg") == 0) {
                    this->tty_params.rtsenb = 3;
                } else this->tty_params.rtsenb = 0;
            } else if (DAF_OS::strcasecmp(param_arg.c_str(), "idsr") == 0) {
                if (param_val[0] == '1' || DAF_OS::strcasecmp(param_val.c_str(), "on") == 0) {
                    this->tty_params.dsrenb = true;
                } this->tty_params.dsrenb = false;
            } else if (DAF_OS::strcasecmp(param_arg.c_str(), "modem") == 0) {
                if (param_val[0] == '1' || DAF_OS::strcasecmp(param_val.c_str(), "on") == 0) {
                    this->tty_params.modem = true;
                } this->tty_params.modem = false;
            } else if (1) /* Maybe Debug? */    {
                ACE_DEBUG((LM_WARNING, ACE_TEXT("WARNING: Parameter '%s' is unknown.\n")
                    , tty_config.substr(pos, e_pos - pos).c_str()));
            }
        } while (pos++ > 0);

        return 0;
    }

    int
    TTY_Device::parse_args(int &argc, ACE_TCHAR *argv[])
    {
        ACE_Get_Opt get_opts(argc, argv, ACE_TEXT("a:z::?"),0);
        get_opts.long_option("address", 'a', ACE_Get_Opt::ARG_REQUIRED);
        get_opts.long_option("debug", 'z', ACE_Get_Opt::ARG_OPTIONAL);
        get_opts.long_option("help", '?', ACE_Get_Opt::NO_ARG);  // Help

        for (;;) switch (get_opts()) {
        case -1 :
            if (this->parse_config(this->tty_device_config) == 0) {
                return 0;
            }
            errno = EINVAL; return -1;

//"COMx[:][baud=b][,parity=p][,data=d][,stop=s]\n\t\t[,to={1|0}][,xon={1|0}][,odsr={1|0}][,octs={1|0}]\n\t\t[,dtr={1|0|hs}][,rts={1|0|hs|tg}][,idsr={1|0}]\n"

        case 'a':   this->tty_device_config.assign(get_opts.opt_arg()); break;

        case 'z':   this->device_debug_ = 1;
            for (const ACE_TCHAR *debug_lvl = get_opts.opt_arg(); debug_lvl;) {
                if (::isdigit(int(*debug_lvl))) {
                    this->device_debug_ = ace_range(1, 10, DAF_OS::atoi(debug_lvl));
                } break; // Turn on Debug optionally at a level
            } break;

        default:    if (DAF::debug()) {
        case '?':
            ACE_DEBUG((LM_INFO,
                "usage:  %s\n"
                "-a Address[:config]\n"
                "-z Debug ON[Level]\n"
                "\n", "TTY_Device"
                ));
            } break;
        }

        return 0;
    }

    int
    TTY_Device::init(int argc, ACE_TCHAR *argv[])
    {
        if (this->parse_args(argc, argv)) {
            ACE_ERROR_RETURN((LM_ERROR,
                ACE_TEXT("ERROR: Unable to Initialize Device\n\t-params='-a %s'.\n"),
                    this->tty_device_config.c_str()),-1);
        }
        return 0;
    }

    int
    TTY_Device::open(void *args)
    {
        ACE_UNUSED_ARG(args);

        this->set_handle(DAF_OS::open(this->tty_device_name.c_str(), O_RDWR | FILE_FLAG_OVERLAPPED , 0));
        //this->enable(ACE_SIGIO);
        //this->enable(ACE_NONBLOCK);

#if defined(_MSC_VER)  && (_MSC_VER > 1800)
# pragma warning (push)
# pragma warning (disable : 4838)
#endif
        ACE_HANDLE handle = this->get_handle(); if (handle != ACE_INVALID_HANDLE) {

#if defined (ACE_WIN32)
            ::SetupComm(handle, RX_DEVICE_BUFFER, TX_DEVICE_BUFFER);
#elif defined(ACE_HAS_TERMIOS)
            this->tty_params.readtimeoutmsec = TTY_IO_TIMEOUT(this->tty_params.baudrate, RD_TIMEOUT_SPACING);
#endif

            if (this->control(ACE_TTY_IO::SETPARAMS, &this->tty_params) == 0) {
#if defined (ACE_WIN32)
                COMMTIMEOUTS commtimeouts = {
                    TTY_IO_TIMEOUT(this->tty_params.baudrate, RD_TIMEOUT_SPACING), // ReadIntervalTimeout - Maximum time between read chars.
                    0, // ReadTotalTimeoutMultiplier - Multiplier of characters.
                    0, // ReadTotalTimeoutConstant - Constant in milliseconds.
                    TTY_IO_TIMEOUT(this->tty_params.baudrate, WT_TIMEOUT_SPACING), // WriteTotalTimeoutMultiplier - Multiplier of characters.
                    0  // WriteTotalTimeoutConstant - Constant in milliseconds.
                }; ::SetCommTimeouts(handle, &commtimeouts);
#endif
                this->device_active_ = true;  // Say Device is now Active

                return this->get_device_handler()->open(this);
            }
        }

#if defined(_MSC_VER)  && (_MSC_VER > 1800)
# pragma warning (pop)
#endif

        ACE_ERROR_RETURN((LM_ERROR,
            ACE_TEXT("ERROR: Unable to Open Device '%s'\n\t-params='%s'.\n"),
                this->tty_device_name.c_str(), this->tty_device_config.c_str()),-1);
    }

    int
    TTY_Device::close(u_long flags)
    {
        if (flags) {
            this->device_active_ = false;
            this->get_device_handler()->module_closed();
        }
        return DAF::TaskExecutor::close(flags);
    }

    ssize_t
    TTY_Device::write_data(const char *ptr, int len, int timeout)
    {
        return this->get_device_handler()->write_data(ptr, len, timeout);
    }
} /* namespace DEV */

#endif /* ACE_HAS_WIN32_OVERLAPPED_IO || ACE_HAS_AIO_CALLS */
