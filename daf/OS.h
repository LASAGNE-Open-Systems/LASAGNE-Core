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
#ifndef DAF_OS_H
#define DAF_OS_H

/**
* @file     OS.h
* @author   Derek Dominish
* @author   $LastChangedBy$
* @date     1st September 2011
* @version  $Revision$
* @ingroup
* @namespace DAF_OS
*/

#include "DAF_export.h"

#include <ace/OS.h>
#include <ace/Thread.h>
#include <ace/Time_Value.h>
#include <ace/Basic_Types.h>
#include <ace/Default_Constants.h>
#include <ace/Numeric_Limits.h>
#include <ace/Min_Max.h>

#include <stdlib.h>
#include <stdexcept>
#include <math.h>
#include <time.h>

#include "Config.h"

typedef class ACE_Time_Value    DAF_Time_Value;

namespace DAF_OS
{
    using namespace ACE_OS;

    /** Remap some windows error messages as appropriate */
    DAF_Export int                  last_error(void);
    inline int                      last_error(int error)
    {
        ACE_OS::last_error(error); return error;
    }

    inline ACE_Time_Value           gettimeofday(void)
    {
        return ACE_OS::gettimeofday();
    }

    DAF_Export ACE_Time_Value       gettimeofday(time_t msec);

    DAF_Export const std::string &  gethostname(void);

    inline pid_t                    process_ID(void)
    {
        return ACE_OS::getpid();
    }

    inline ACE_thread_t             thread_ID(void)
    {
        return ACE_Thread::self();
    }

    DAF_Export ACE_hthread_t        thread_HANDLE(void);

    DAF_Export long                 thread_PRIORITY(ACE_hthread_t ht_id = thread_HANDLE());

    DAF_Export int                  thread_0_SIGSET_T(void);

   /*
    * The abs() function is required to ensure that the
    * same result in precision is returned across
    * different OS implementations, otherwise you will need
    * to call the appropriate system function depending on type
    *
    * ie. windows provides a macro -  double abs(double x);
    * The equivalent in linux is   -  double fabs(double x);
    *
    * This template works with the type that is passed regardless
    * of the input precision type
    */

    template <typename T> inline
    const T abs(const T &t)
    {
        return T((t > T(0.0)) ? (t) : -(t));
    }

    template <typename T> inline
    const T rand(const T &min_val, const T &max_val)
    {
        return T(((double(ACE_OS::rand()) / (1.0 + RAND_MAX)) * (max_val - min_val)) + min_val);
    }

    /*
    * A potential implementation problem is that different implementations
    * of the trig functions do not approximate zero very well.  To aleviate
    * this problem all trig function outputs are evaluated to approximate
    * EPS which is a very small number with respect to machine precision.
    */
    template <typename T> inline
    const T eps_zero(const T &t)
    {
        return ((DAF_OS::abs(t) > T(DAF_M_EPS)) ? t : T(0.0));
    }

    /*
    * The mod() function appears to be implemented differently
    * in different language implementations. A mod(x,y) function
    * is defined as the remainder on dividing x by y and always
    * lies in the range (0 <= mod < y).
    * (eg. mod(2.3,2)=0.3 and mod(-2.3,2)=1.7)
    * The following implementation ensures this consistancy
    */
    template <typename T> inline
    const T mod(const T &x, const T &y)
    {
        return DAF_OS::eps_zero(x-(y*::floor(x/y)));
    }
}  // namespace DAF_OS

#endif  // DAF_OS_H
