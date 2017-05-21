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
#ifndef DAF_CONSTANTS_H
#define DAF_CONSTANTS_H

#include <ace/config.h>

/**
* @file    Constants.h
* @author
* @author   $LastChangedBy$
* @date
* @version  $Revision$
* @ingroup  \todo{what group?}
*/

/*************** ACE Conditional Support Checks ***************/

#include <ace/Version.h>

#if !defined(ACE_GTEQ_VERSION)
# define ACE_GTEQ_VERSION(MAJOR,MINOR,BETA) \
    ((ACE_MAJOR_VERSION >  MAJOR) || \
    ((ACE_MAJOR_VERSION == MAJOR) && (ACE_MINOR_VERSION >  MINOR)) || \
    ((ACE_MAJOR_VERSION == MAJOR) && (ACE_MINOR_VERSION == MINOR)  && (ACE_BETA_VERSION >= BETA)))
#endif

// Used by DAF::ServiceGestalt for iteration lock access to ACE_Service_Repository
#if !defined(ACE_HAS_SERVICE_REPOSITORY_LOCK_ACCESS) && ACE_GTEQ_VERSION(6, 3, 4)
# define ACE_HAS_SERVICE_REPOSITORY_LOCK_ACCESS 1
#endif

/******************* ACE CONSTANTS ****************************/

#define ACE_DEBUGGING               ACE_TEXT("ACEDebug")
// ACE Reactor Threads
#define ACE_BASETHREADS             ACE_TEXT("ACEBaseThreads")

#if !defined(ACE_READ_GUARD_ACTION)
# define ACE_READ_GUARD_ACTION(MUTEX, OBJ, LOCK, ACTION, REACTION) \
   ACE_Read_Guard< MUTEX >  OBJ(LOCK); if (OBJ.locked()) { ACTION; } else { REACTION; }
#endif /* !ACE_READ_GUARD_ACTION */
#if !defined(ACE_READ_GUARD_REACTION)
# define ACE_READ_GUARD_REACTION(MUTEX, OBJ, LOCK, REACTION) \
   ACE_READ_GUARD_ACTION(MUTEX,OBJ,LOCK,,REACTION)
#endif

#if !defined(ACE_WRITE_GUARD_ACTION)
# define ACE_WRITE_GUARD_ACTION(MUTEX, OBJ, LOCK, ACTION, REACTION) \
   ACE_Write_Guard< MUTEX > OBJ(LOCK); if (OBJ.locked()) { ACTION; } else { REACTION; }
#endif /* !ACE_WRITE_GUARD_ACTION */
#if !defined(ACE_WRITE_GUARD_REACTION)
# define ACE_WRITE_GUARD_REACTION(MUTEX, OBJ, LOCK, REACTION) \
   ACE_WRITE_GUARD_ACTION(MUTEX,OBJ,LOCK,,REACTION)
#endif

#if !defined(DAF_DEPRECATED)
# if !defined(DAF_DEPRECATED_MSG)
#  if defined(ACE_WIN32)
#   define DAF_DEPRECATED_MSG(MSG)  __declspec(deprecated(MSG))
#  else
#   define DAF_DEPRECATED_MSG(MSG)  __attribute__((deprecated(MSG)))
#  endif
# endif
# define DAF_DEPRECATED             DAF_DEPRECATED_MSG("this function is deprecated")
#endif

/******************* DAF CONSTANTS ****************************/

#define DAF_DLL_NAME                ACE_TEXT("DAF")

#define DAF_DEBUGGING               ACE_TEXT("DAFDebug")

#define DAF_PROPERTIES              ACE_TEXT("DAFProperties")

// Properties
#define DAF_HEXDUMPWIDTH            ACE_TEXT("DAFHexDumpWidth")
#define DAF_TASKHANDOFFTIMEOUT      ACE_TEXT("DAFTaskHandoffTimeout")
#define DAF_TASKEVICTTIMEOUT        ACE_TEXT("DAFTaskEvictTimeout")
#define DAF_TASKDECAYTIMEOUT        ACE_TEXT("DAFTaskDecayTimeout")
#define DAF_SVCACTIONTIMEOUT        ACE_TEXT("DAFSvcActionTimeout")

/* EPS a small number ~ machine precision (~0.0 for floating maths) */
#if !defined(DAF_M_EPS)
# define DAF_M_EPS  double(1e-15)
#endif

#if !defined(DAF_MSECS_ONE_SECOND)
# define DAF_MSECS_ONE_SECOND       time_t(1  * 1000)
#endif
#if !defined(DAF_MSECS_ONE_MINUTE)
# define DAF_MSECS_ONE_MINUTE       time_t(60 * DAF_MSECS_ONE_SECOND)
#endif
#if !defined(DAF_MSECS_ONE_HOUR)
# define DAF_MSECS_ONE_HOUR         time_t(60 * DAF_MSECS_ONE_MINUTE)
#endif
#if !defined(DAF_MSECS_ONE_DAY)
# define DAF_MSECS_ONE_DAY          time_t(24 * DAF_MSECS_ONE_HOUR)
#endif

#if !defined(DAF_DEFAULT_SERVICE_ACTION_TIMEOUT)
# define DAF_DEFAULT_SERVICE_ACTION_TIMEOUT     time_t(10) /* 10 Seconds */
#endif

#if !defined(DAF_CONFIGURATOR_COMMON_SECTION_NAME)
# define DAF_CONFIGURATOR_COMMON_SECTION_NAME   ACE_TEXT("common")
#endif

/******************* TAO CONSTANTS ****************************/

#define TAO_DEBUGGING               ACE_TEXT("TAODebug")
#define TAO_ORBDEBUGGING            ACE_TEXT("TAOrbDebug")

/******************* TAF CONSTANTS ****************************/

#define TAF_DLL_NAME                ACE_TEXT("TAF")

#define TAF_DEBUGGING               ACE_TEXT("TAFDebug")

// Properties
#define TAF_ORBTHREADS              ACE_TEXT("TAFOrbThreads")
#define TAF_BASECONTEXT             ACE_TEXT("TAFBaseContext")
#define TAF_ORBINITARGS             ACE_TEXT("TAFOrbInitArgs")
#define TAF_EXTENSIONARGS           ACE_TEXT("TAFExtensionArgs")
#define TAF_RESOLVETIMEOUT          ACE_TEXT("TAFResolveTimeout")
#define TAF_DISCOVERYENABLE         ACE_TEXT("TAFDiscoveryEnable")
#define TAF_DISCOVERYENDPOINT       ACE_TEXT("TAFDiscoveryEndpoint")

#define TAF_SERVERNAME              ACE_TEXT("TAFServerName")
#define TAF_SERVERLOADTIMEOUT       ACE_TEXT("TAFServerLoadTimeout")
#define TAF_ORBNAME                 ACE_TEXT("TAFOrbName")
#define TAF_SERVICES                ACE_TEXT("TAFServices")
#define TAF_PROPERTIES              ACE_TEXT("TAFProperties")

// Read-Only Properties
#define TAF_SERVICE_ARGS            ACE_TEXT("TAFServiceArgs")
#define TAF_SERVER_ARGS             ACE_TEXT("TAFServerArgs")
#define TAF_DATESTAMP               ACE_TEXT("TAFDateStamp")
#define TAF_TIMESTAMP               ACE_TEXT("TAFTimeStamp")
#define TAF_HOSTNAME                ACE_TEXT("TAFHostName")

// Object IDs
#define TAF_OBJID_DEFAULTPOA        ACE_TEXT("TAFDefaultPOA")

#define TAF_DATESTAMP_FORMAT        ACE_TEXT("%Dl")  /* Local Date stamp */
#define TAF_TIMESTAMP_FORMAT        ACE_TEXT("%Tu")  /* Local Time stamp */

// This address MUST be within the range for host group addresses: 224.0.0.0 to 239.255.255.255.
#if !defined (TAF_DEFAULT_DISCOVERY_PORT)
# define TAF_DEFAULT_DISCOVERY_PORT unsigned(26857)  /* My BirthDate */
#endif /* DAF_DEFAULT_DISCOVERY_PORT */
#if !defined (TAF_DEFAULT_DISCOVERY_ADDRESS)
# if defined(ACE_HAS_IPV6)
#  define TAF_DEFAULT_DISCOVERY_ADDRESS ACE_DEFAULT_MULTICASTV6_ADDR
# else
#  define TAF_DEFAULT_DISCOVERY_ADDRESS ACE_DEFAULT_MULTICAST_ADDR
# endif /* DAF_DEFAULT_MULTICAST_ADDR */
#endif /* DAF_DEFAULT_DISCOVERY_PORT */

/******************* TAF SECURITY CONSTANTS *******************/

#define TAF_SECURITY                ACE_TEXT("TAFSecurity")
#define TAF_SECURITYDISABLE         ACE_TEXT("TAFSecurityDisable")
#define TAF_DEFAULTALLOWANCE        ACE_TEXT("TAFSecurityDefaultAllowance")
#define TAF_COLLOCATEDALLOWANCE     ACE_TEXT("TAFSecurityCollocatedAllowance")
#define TAF_SSLNOPROTECTION         ACE_TEXT("TAFSSLNoProtection")

/******************* OTHER CONSTANTS *************************/

#endif // DAF_CONSTANTS_H
