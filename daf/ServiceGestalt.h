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
#ifndef DAF_SERVICEGESTALT_H
#define DAF_SERVICEGESTALT_H

/**
* @file     ServiceGestalt.h
* @author   Derek Dominish
* @author   $LastChangedBy$
* @date     1st September 2011
* @version  $Revision$
* @ingroup
* @namespace DAF
*/

#include "DAF.h"
#include "TaskExecutor.h"

#include <ace/Service_Config.h>
#include <ace/Service_Gestalt.h>
#include <ace/Min_Max.h>

#include <list>

namespace DAF
{
    /** @class ServiceGestalt
    *@brief Brief \todo{Fill this in}
    *
    * Details \todo{Detailed description}
    */
    class DAF_Export ServiceGestalt : protected ACE_Service_Gestalt
    {
        DAF::TaskExecutor svcExecutor_;

    public:

        /** \todo{Fill this in} */
        ServiceGestalt(const ACE_TCHAR *program_name = 0);
        /** \todo{Fill this in} */
        virtual ~ServiceGestalt(void);

        /** \todo{Fill this in} */
        enum {
            DEFAULT_MAXIMUM_TIMEOUT = time_t(60000),    // milliseconds
            DEFAULT_DLOAD_TIMEOUT   = time_t(15000),    // milliseconds
            DEFAULT_SLOAD_TIMEOUT   = time_t(10000),     // milliseconds
            DEFAULT_REMOVE_TIMEOUT  = time_t(6000),     // milliseconds
            DEFAULT_RESUME_TIMEOUT  = time_t(5000),     // milliseconds
            DEFAULT_SUSPEND_TIMEOUT = time_t(4000),     // milliseconds
            DEFAULT_MINIMUM_TIMEOUT = time_t(1000)      // milliseconds
        };

    public:

        /** \todo{Fill this in} */
        static std::string  static_directive    (const ACE_TCHAR *ident, const ACE_TCHAR *parameters = "");
        /** \todo{Fill this in} */
        static std::string  dynamic_directive   (const ACE_TCHAR *ident, const ACE_TCHAR *libpathname, const ACE_TCHAR *objectclass, const ACE_TCHAR *parameters = "");
        /** \todo{Fill this in} */
        static std::string  suspend_directive   (const ACE_TCHAR *ident);
        /** \todo{Fill this in} */
        static std::string  resume_directive    (const ACE_TCHAR *ident);
        /** \todo{Fill this in} */
        static std::string  remove_directive    (const ACE_TCHAR *ident);

    protected:

        /** \todo{Fill this in} */
        virtual int process_command(const std::string &command, time_t timeout);
    };

#if !defined(ACE_HAS_SERVICE_REPOSITORY_LOCK_ACCESS)
    DAF_Export ACE_Recursive_Thread_Mutex & getRepositoryLock(const ACE_Service_Repository *repo);
#endif

} // namespace DAF

  /** @struct DAF_Service_Config_Guard
  *@brief Brief \todo{Fill this in}
  *
  * Details \todo{Detailed description}
  */
struct DAF_Service_Config_Guard : ACE_Service_Config_Guard {
    DAF_Service_Config_Guard(ACE_Service_Gestalt *gestalt = ACE_Service_Config::global())
        : ACE_Service_Config_Guard(gestalt) {}
};

#endif // DAF_SERVICEGESTALT
