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
#define DAF_DAF_CPP                                                                                                                                                                      E

#include "DAF.h"
#include "ARGV.h"
#include "DateTime.h"
#include "PropertyManager.h"

#include <sstream>
#include <vector>

#if !defined(FORMATTING_SYNTAX_WARNING_TEXT)
# define  FORMATTING_SYNTAX_WARNING_TEXT ACE_TEXT("WARNING: Failed to expand; invalid formatting syntax [%s].\n")
#endif

namespace DAF
{
    size_t
    hex_dump_width(void)
    {
        return ace_range(size_t(1), size_t(1024), DAF::get_numeric_property<size_t>(DAF_HEXDUMPWIDTH, size_t(16), true));
    }

    std::string
    hex_dump_data(const void *buf, size_t len, size_t width)
    {
        static const char h_char[] = {
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
        };
        static const char c_char[] = {
        //   0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
            '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', // 00
            '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', '.', // 10
            ' ', '!', '\"','#', '$', '%', '&', '\'','(', ')', '*', '+', ',', '-', '.', '/', // 20
            '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ':', ';', '<', '=', '>', '?', // 30
            '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', // 40
            'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\',']', '^', '_', // 50
            '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', // 60
            'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~', '.'  // 70
        };

        std::stringstream ss, sc;

        const char *p = reinterpret_cast<const char*>(buf);

        const int w = int(width);

        if (len) {
            for (int j = 0, l = int(len); j < w; j++) {
                if ((j % 4) == 0) {
                    ss << ' ';
                }
                if (l-- > 0) {
                    if (j && (j % 8) == 0) {
                        sc << ' ';
                    }
                    const char sx = *p++;
                    sc << ((sx & 0x80) ? '.' : c_char[sx & 0x7F]);
                    ss << h_char[(sx >> 4) & 0x0F] << h_char[sx & 0x0F];
                } else {
                    ss << "  ";
                }
            }

            ss << " \"" << sc.str() << '"';
        }
        return ss.str();
    }

    std::string
    hex_dump(const void *buf, size_t len, size_t width)
    {
        std::stringstream ss;

        const char *p = reinterpret_cast<const char*>(buf);

        char s[128]; DAF_OS::sprintf(s, ACE_TEXT("\nDAF_DUMP: Address = 0x%p, Length = %d"), p, int(len)); ss << s;

        if (p) {

            for (int i = 0; len > 0; i++) {
                DAF_OS::sprintf(s, ACE_TEXT("\n0x%04X -"), unsigned(i * width));
                size_t l_len = ace_min(width, len);
                ss << s << hex_dump_data(p, l_len, width);
                len -= l_len; p += l_len;
            }

        }

        std::ends(ss << std::endl); return ss.str();
    }

    std::string
    bit_dump(const void *buf, size_t bits)
    {
        static const unsigned char _bit_mask[8] =
        {
            0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01
        };

        std::stringstream ss;

        const char *p = reinterpret_cast<const char*>(buf);

        char s[128]; DAF_OS::sprintf(s, ACE_TEXT("\nDAF_BitDump: Address = 0x%p, Length = %d bits"), p, int(bits)); ss << s;

        if (p && int(bits) > 0) {

            size_t width = ace_max(size_t(8), size_t((DAF::hex_dump_width() / 4) * 8)); // Round down but min = 8

            for (size_t i = 0; i < bits; i++) {

                do {
                    if (i) {
                        if (i % width) {
                            if ((i % 8) == 0) {
                                ss << ' ';
                            }
                            continue;
                        }
                    }

                    DAF_OS::sprintf(s, ACE_TEXT("\n0x%04X - "), unsigned(i / 8)); ss << s;

                } while (false);

                ss << ((p[i / 8] & _bit_mask[i % 8]) ? '1' : '0');
            }
        }

        std::ends(ss << std::endl); return ss.str();
    }

    const char * get_errno_text(int error)
    {
        switch (error) {
        case 0:              return 0;
        case EPERM:          return "EPERM";
        case ENOENT:         return "ENOENT";
        case ESRCH:          return "ESRCH";
        case EINTR:          return "EINTR";
        case EIO:            return "EIO";
        case ENXIO:          return "ENXIO";
        case E2BIG:          return "E2BIG";
        case ENOEXEC:        return "ENOEXEC";
        case EBADF:          return "EBADF";
        case ECHILD:         return "ECHILD";
        case EAGAIN:         return "EAGAIN";
        case ENOMEM:         return "ENOMEM";
        case EACCES:         return "EACCES";
        case EFAULT:         return "EFAULT";
        case EBUSY:          return "EBUSY";
        case EEXIST:         return "EEXIST";
        case EXDEV:          return "EXDEV";
        case ENODEV:         return "ENODEV";
        case ENOTDIR:        return "ENOTDIR";
        case EISDIR:         return "EISDIR";
        case ENFILE:         return "ENFILE";
        case EMFILE:         return "EMFILE";
        case ENOTTY:         return "ENOTTY";
        case EFBIG:          return "EFBIG";
        case ENOSPC:         return "ENOSPC";
        case ESPIPE:         return "ESPIPE";
        case EROFS:          return "EROFS";
        case EMLINK:         return "EMLINK";
        case EPIPE:          return "EPIPE";
        case EDOM:           return "EDOM";
        case EDEADLK:        return "EDEADLK";
        case ENAMETOOLONG:   return "ENAMETOOLONG";
        case ENOLCK:         return "ENOLCK";
        case ENOSYS:         return "ENOSYS";
        case ENOTEMPTY:      return "ENOTEMPTY";
        case EINVAL:         return "EINVAL";
        case ERANGE:         return "ERANGE";
        case EILSEQ:         return "EILSEQ";
        case EADDRINUSE:     return "EADDRINUSE";
        case EADDRNOTAVAIL:  return "EADDRNOTAVAIL";
        case EAFNOSUPPORT:   return "EAFNOSUPPORT";
        case EALREADY:       return "EALREADY";
        case EBADMSG:        return "EBADMSG";
        case ECANCELED:      return "ECANCELED";
        case ECONNABORTED:   return "ECONNABORTED";
        case ECONNREFUSED:   return "ECONNREFUSED";
        case ECONNRESET:     return "ECONNRESET";
        case EDESTADDRREQ:   return "EDESTADDRREQ";
        case EHOSTUNREACH:   return "EHOSTUNREACH";
        case EIDRM:          return "EIDRM";
        case EINPROGRESS:    return "EINPROGRESS";
        case EISCONN:        return "EISCONN";
        case ELOOP:          return "ELOOP";
        case EMSGSIZE:       return "EMSGSIZE";
        case ENETDOWN:       return "ENETDOWN";
        case ENETRESET:      return "ENETRESET";
        case ENETUNREACH:    return "ENETUNREACH";
        case ENOBUFS:        return "ENOBUFS";
        case ENODATA:        return "ENODATA";
        case ENOLINK:        return "ENOLINK";
        case ENOMSG:         return "ENOMSG";
        case ENOPROTOOPT:    return "ENOPROTOOPT";
        case ENOSR:          return "ENOSR";
        case ENOSTR:         return "ENOSTR";
        case ENOTCONN:       return "ENOTCONN";
        case ENOTRECOVERABLE:return "ENOTRECOVERABLE";
        case ENOTSOCK:       return "ENOTSOCK";
        case ENOTSUP:        return "ENOTSUP";
        case EOVERFLOW:      return "EOVERFLOW";
        case EOWNERDEAD:     return "EOWNERDEAD";
        case EPROTO:         return "EPROTO";
        case EPROTONOSUPPORT:return "EPROTONOSUPPORT";
        case EPROTOTYPE:     return "EPROTOTYPE";
        case ETIME:          return "ETIME";
        case ETIMEDOUT:      return "ETIMEDOUT";
        case ETXTBSY:        return "ETXTBSY";
        }

        return "Unknown";
    }

    std::string
    last_error_text(int error)
    {
        char s[64] = { 0 };

        if (error) for (const char * errno_text = DAF::get_errno_text(error); errno_text;) {
            DAF_OS::sprintf(s, ACE_TEXT("[Error=%d (%s)]"), error, errno_text); break;
        }

        return s;
    }

    int     print_last_error(int error)
    {
        if (error) {
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("\n(%P|%t) ERROR: Failure with last error %s\n")
                , DAF::last_error_text(error).c_str()));
        }
        return error;
    }

/************* String Manimpulation *********************************************/

    std::string
    trim_string(const std::string &param, const char c)
    {
        std::string arg(param);
        for (size_t pos = arg.length(); pos--;) {
            if (::isspace(int(arg[pos])) || ::iscntrl(int(arg[pos]))) {
                arg.erase(pos, 1); continue;
            } else if (::isspace(int(arg[0])) || ::iscntrl(int(arg[0]))) {
                arg.erase(0, 1); continue;
            } else if (arg[0] == c && arg[pos] == c) {
                if (pos) { arg.erase(pos--, 1); } arg.erase(0, 1); continue; // Balance erase from both ends
            } else do {
                if (::iscntrl(int(arg[pos]))) { arg[pos] = ' '; }
            } while (pos--); break;
        }
        return arg;
    }

    std::string
    parse_esc(const std::string &arg, size_t offset)
    {
        std::string param(arg);
        for (int pos = int(offset); (pos = int(param.find_first_of('\\',pos))) >= 0; param.erase(pos++,1))
            ;
        return param;
    }

    std::string
    parse_argv(const ACE_ARGV &argv, bool quote_args)
    {
        std::string params;

        for (int arg_c = 0; arg_c < argv.argc(); arg_c++) {
            std::string arg(DAF::trim_string(const_cast<ACE_ARGV &>(argv).operator [] (arg_c)));
            if (arg.length()) {
                if (params.length()) {
                    params.append(1,' '); // Add Space Seperator
                }
                if (quote_args && int(arg.find_first_of(' ')) > 0) { // Embedded spaces
                    arg.insert(0,1,'\'').append(1,'\'');
                }
                params.append(arg);
            }
        }

        return params;
    }

    std::string
    parse_args(int argc, ACE_TCHAR *argv[], bool substitute_env_args, bool quote_args)
    {
        return DAF::parse_argv(ACE_ARGV(argc, argv, substitute_env_args, quote_args));
    }

    std::string
    parse_args(const std::string &arg, bool substitute_env_args, bool quote_args)
    {
        return DAF::parse_argv(ACE_ARGV(arg.c_str(), substitute_env_args), quote_args);
    }

    void
    print_argv(const ACE_ARGV &args)
    {
        for (int arg_c = 0, end_c = args.argc(); arg_c < end_c; arg_c++) {
            ACE_DEBUG((LM_INFO, ACE_TEXT("argv[%02d] = '%s'\n"), arg_c, const_cast<ACE_ARGV &>(args).operator [] (arg_c)));
        }
    }

    void
    print_args(int argc, ACE_TCHAR *argv[], bool substitute_env_args)
    {
        print_argv(ACE_ARGV(argc, argv, substitute_env_args, true));
    }

    void
    print_args(const std::string &arg, bool substitute_env_args)
    {
        print_argv(ACE_ARGV(arg.c_str(), substitute_env_args));
    }

    std::string
    format_args(const std::string &args, bool substitute_env_args, bool quote_args)
    {
        ACE_ARGV args_argv(args.c_str(), substitute_env_args); DAF_ARGV params_argv(substitute_env_args);

        for (int i = 0; i < args_argv.argc(); i++) {

            std::string arg(args_argv[i]); // Grab our modifyable parameter argument

            for (int pos = 0; (pos = int(arg.find_first_of('%', pos))) >= 0;) {

                std::string formattingWarnMsg;

                if (int(arg.length()) > pos++) {

                    std::stringstream mod_arg;

                    switch (arg[pos++]) {
                        /* Treat '%' as character */
                    case '%': arg.erase(--pos, 1); continue;

                        /* Host Name */
                    case 'H': mod_arg << DAF_OS::gethostname(); break;

                        /* print thread id */
                    case 't': mod_arg << static_cast<unsigned>(DAF_OS::thread_ID());    break;

                        /* print out the current process id */
                    case 'P': mod_arg << static_cast<unsigned>(DAF_OS::process_ID());   break;

                    case 'p':  /* FORMAT named property */
                        do {

                            formattingWarnMsg.assign("%p");

                            if (int(arg.length()) > pos) {
                                if (arg[pos] == '{') {
                                    int lpos = int(arg.find_first_of('}', pos));
                                    if (lpos-- > pos) try {
                                        int plen = lpos - pos;
                                        const std::string prop(DAF::trim_string(arg.substr(pos + 1, plen)));
                                        formattingWarnMsg.append(arg.substr(pos, plen + 2));
                                        mod_arg << DAF::trim_string(DAF::get_property(prop, true), '\''); arg.erase(pos, plen + 2); continue;
                                    }
                                    catch (const DAF::IllegalArgumentException &) {
                                    }
                                }
                            }

                            ACE_DEBUG((LM_WARNING, FORMATTING_SYNTAX_WARNING_TEXT, formattingWarnMsg.c_str()));

                        } while (false); break;

                    case 'D': /* FORMAT Date output */
                        do {

                            formattingWarnMsg.assign("%D");

                            if (int(arg.length()) > pos) switch (arg[pos]) {
                                /* print GMT timestamp 2015-10-08 13:15:35 format */
                            case 'g': mod_arg << DAF_Date_Time::GMTime().toString(false);       arg.erase(pos, 1); continue;

                                /* print GMT timestamp Thursday, 8th October 2015 1:15:35PM format */
                            case 'G': mod_arg << DAF_Date_Time::GMTime().toString(true);        arg.erase(pos, 1); continue;

                                /* print LOCAL timestamp 2015-10-08 13:15:35 format */
                            case 'l': mod_arg << DAF_Date_Time::LOCALTime().toString(false);    arg.erase(pos, 1); continue;

                                /* print LOCAL timestamp Thursday, 8th October 2015 1:15:35PM format */
                            case 'L': mod_arg << DAF_Date_Time::LOCALTime().toString(true);     arg.erase(pos, 1); continue;

                            default: formattingWarnMsg.append(1, arg[pos]); break;
                            }

                            ACE_DEBUG((LM_WARNING, FORMATTING_SYNTAX_WARNING_TEXT, formattingWarnMsg.c_str()));

                        } while (false); break;

                    case 'T': /* FORMAT Time output */
                        do {

                            formattingWarnMsg.assign("%T");

                            if (int(arg.length()) > pos) switch (arg[pos]) {
                                /* print UTC timestamp */
                            case 'u': mod_arg << DAF_Date_Time::UTCTime(); arg.erase(pos, 1); continue;

                            default: formattingWarnMsg.append(1, arg[pos]); break;
                            }

                            ACE_DEBUG((LM_WARNING, FORMATTING_SYNTAX_WARNING_TEXT, formattingWarnMsg.c_str()));

                        } while (false); break;

                    default:
                        formattingWarnMsg.assign(arg.substr(pos - 2, 2));
                        ACE_DEBUG((LM_WARNING, FORMATTING_SYNTAX_WARNING_TEXT, formattingWarnMsg.c_str()));
                        continue;
                    }

                    pos -= 2; arg.erase(pos, 2).insert(pos, mod_arg.str());
                }
            }

            if (arg.length()) { // Build the ARGV from backing list
                params_argv.add(arg.c_str(), false); // Add this argument to the backing list
            }
        }

        return DAF::parse_argv(params_argv, quote_args);
    }


    /************* High Resolution Time Measurement *********************************************/

    ACE_hrtime_t
    elapsed_hrtime(const ACE_hrtime_t &start, const ACE_hrtime_t &end)
    {
        // We have to check if the measurement counter has wrapped around.
        // ACE_High_Res_Timer has a static method called elapsed_hrtime that would do this for you if they didn't make it private...
        return (end > start ? end - start : ~start + 1 + end);
    }


    ACE_hrtime_t
    elapsed_hrtime_msecs(const ACE_hrtime_t &start, const ACE_hrtime_t &end)
    {
        return DAF::elapsed_hrtime_usecs(start, end) / 1000u;
    }


    ACE_hrtime_t
    elapsed_hrtime_usecs(const ACE_hrtime_t &start, const ACE_hrtime_t &end)
    {
        // Really annoyingly, these scales work differently if you are on Windows compared to everything else
        // Refer to ACE_High_Res_Timer::elapsed_microseconds(ACE_hrtime_t &usecs) const in $(ACE_ROOT)/ace/High_Res_Timer.inl
#if defined (ACE_WIN32)
        return DAF::elapsed_hrtime(start, end) * ACE_HR_SCALE_CONVERSION / ACE_High_Res_Timer::global_scale_factor();
#else
        return DAF::elapsed_hrtime(start, end) / ACE_High_Res_Timer::global_scale_factor();
#endif // ACE_WIN32
    }


    ACE_hrtime_t
    elapsed_hrtime_nsecs(const ACE_hrtime_t &start, const ACE_hrtime_t &end)
    {
        // Refer to ACE_High_Res_Timer::elapsed_time(ACE_hrtime_t &nanoseconds) const
        // in $(ACE_ROOT)/ace/High_Res_Timer.cpp for additional notes on this technique
#if defined (ACE_WIN32)
        return DAF::elapsed_hrtime(start, end) * ACE_HR_SCALE_CONVERSION * 1000u / ACE_High_Res_Timer::global_scale_factor();
#else
        return (DAF::elapsed_hrtime(start, end) * (1024000u / ACE_High_Res_Timer::global_scale_factor())) >> 10;
#endif // ACE_WIN32
    }

}
