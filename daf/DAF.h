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
#ifndef DAF_DAF_H
#define DAF_DAF_H

#include <ace/ACE.h>
#include <ace/ARGV.h>
#include <ace/Synch.h>
#include <ace/Synch_Traits.h>
#include <ace/Guard_T.h>
#include <ace/Log_Msg.h>
#include <ace/Auto_Ptr.h>
#include <ace/OS_NS_errno.h>
#include <ace/Copy_Disabled.h>
#include <ace/High_Res_Timer.h>
#include <ace/Service_Config.h>

#include <string>
#include <sstream>
#include <iostream>

#include "DAF_export.h"

#if defined(_MSC_VER)
# pragma hdrstop
#endif

#include "OS.h"
#include "DAFDebug.h"
#include "Exception.h"

#include <ace/OS_main.h>

/**
  \namespace DAF
  \brief Concurrency and ACE extensions

  The DAF Namespace contains a number of fixes, additions and extensions of the
  ACE library. Its focus is on:
  - Concurrency primitives
  - High level Concurrency patterns that are used in multi-threaded development
  - Threading engines to allow easy management of thread pools and execution
*/
namespace DAF
{
    DAF_Export size_t       hex_dump_width(void);

    DAF_Export std::string  hex_dump(const void*, size_t len, size_t width = hex_dump_width());
    DAF_Export std::string  hex_dump_data(const void *s, size_t len, size_t width = hex_dump_width());

    DAF_Export std::string  bit_dump(const void*, size_t bits);

    DAF_Export const char * get_errno_text(int error = int(errno)); // NOTE: Could return 0

    DAF_Export std::string  last_error_text(int error = DAF_OS::last_error());

    DAF_Export std::string  trim_string(const std::string &, const char c = ' ');

    DAF_Export std::string  parse_esc(const std::string &, size_t offset = 0);

    DAF_Export std::string  parse_argv(const ACE_ARGV &, bool quote_args = true);
    DAF_Export std::string  parse_args(const std::string &args, bool substitute_env_args = true, bool quote_args = true);
    DAF_Export std::string  parse_args(int argc, ACE_TCHAR *argv[], bool substitute_env_args = true, bool quote_args = true);

    DAF_Export std::string  format_args(const std::string &args, bool substitute_env_args = true, bool quote_args = true);

    DAF_Export void         print_argv(const ACE_ARGV &);
    DAF_Export void         print_args(const std::string &args, bool substitute_env_args = false);
    DAF_Export void         print_args(int argc, ACE_TCHAR *argv[], bool substitute_env_args = false);

    DAF_Export int          print_last_error(int error = DAF_OS::last_error());

    DAF_Export void         print_gestalt(const ACE_Service_Gestalt * sg = ACE_Service_Config::current());

    DAF_Export std::string  locateServiceIdent(const ACE_Service_Object * svc_obj, const ACE_Service_Gestalt * sg = ACE_Service_Config::current());


    /**
    * \fn ACE_hrtime_t elapsed_hrtime(const ACE_hrtime_t &start, const ACE_hrtime_t &end)
    * \brief Helper method to calculate the difference between a start and end time and account for counter rollover
    * \param start The start time
    * \param end The end time
    * \return The elapsed time difference in high resolution timer ticks
    */
    ACE_hrtime_t            elapsed_hrtime(const ACE_hrtime_t &start, const ACE_hrtime_t &end);

    /**
     * \fn ACE_hrtime_t elapsed_hrtime_msecs(const ACE_hrtime_t &start, const ACE_hrtime_t &end = DAF_OS::gethrtime(DAF_OS::ACE_HRTIMER_GETTIME))
     * \brief Calculate elapsed time between a start and end time in milliseconds (ms)
     * \param start The start time
     * \param end The end time
     * \return The elapsed time in milliseconds
     */
    DAF_Export ACE_hrtime_t elapsed_hrtime_msecs(const ACE_hrtime_t &start, const ACE_hrtime_t &end = DAF_OS::gethrtime(DAF_OS::ACE_HRTIMER_GETTIME));

    /**
    * \fn ACE_hrtime_t elapsed_hrtime_usecs(const ACE_hrtime_t &start, const ACE_hrtime_t &end = DAF_OS::gethrtime(DAF_OS::ACE_HRTIMER_GETTIME))
    * \brief Calculate elapsed time between a start and end time in microseconds (us)
    * \param start The start time
    * \param end The end time
    * \return The elapsed time in microseconds
    */
    DAF_Export ACE_hrtime_t elapsed_hrtime_usecs(const ACE_hrtime_t &start, const ACE_hrtime_t &end = DAF_OS::gethrtime(DAF_OS::ACE_HRTIMER_GETTIME));

    /**
    * \fn ACE_hrtime_t elapsed_hrtime_nsecs(const ACE_hrtime_t &start, const ACE_hrtime_t &end = DAF_OS::gethrtime(DAF_OS::ACE_HRTIMER_GETTIME))
    * \brief Calculate elapsed time between a start and end time in nanoseconds (ns)
    * \param start The start time
    * \param end The end time
    * \return The elapsed time in nanoseconds
    */
    DAF_Export ACE_hrtime_t elapsed_hrtime_nsecs(const ACE_hrtime_t &start, const ACE_hrtime_t &end = DAF_OS::gethrtime(DAF_OS::ACE_HRTIMER_GETTIME));


    /** @class HighResTimer
    * @brief Brief \todo{fill this in}
    *
    * Detailed description \todo{fill this in}
    */
    class DAF_Export HighResTimer : ACE_High_Res_Timer
    {
    public:

        /** \todo{Fill this in}   */
        HighResTimer(const char *msg = 0) : msg_(msg ? msg : "")
        {
            this->start();
        }

        /** \todo{Fill this in}   */
        ~HighResTimer(void)
        {
            this->stop(); this->print_ave(this->msg(), 1);
        }

        /** \todo{Fill this in}   */
        const char* msg(void) const
        {
            return this->msg_.c_str();
        }

    private:

        std::string msg_;
    };

    template <typename T> inline
    bool is_nil(const T &t)
    {
        return bool(t == 0);
    }
}

#endif
