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
#ifndef DAF_CONFIG_H
#define DAF_CONFIG_H

#include <ace/config.h>

/**
* @file    Config.h
* @author
* @author   $LastChangedBy$
* @date
* @version  $Revision$
* @ingroup  \todo{what group?}
*/

/********** Ensure Environment Validity ***********/

#if !defined(ACE_ENABLE_SWAP_ON_WRITE) && (ACE_BYTE_ORDER == ACE_LITTLE_ENDIAN)
# error "DAF requires ACE_ENABLE_SWAP_ON_WRITE to be set for this configuration."
#endif

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

// Used by DAF::TaskExecutor for access to ACE_Thread_Descriptor::terminate access
#if !defined(ACE_HAS_THREAD_DESCRIPTOR_TERMINATE_ACCESS) && ACE_GTEQ_VERSION(6, 4, 3)
# define ACE_HAS_THREAD_DESCRIPTOR_TERMINATE_ACCESS 1
#endif

/********** DAF Configuration *********************/

#if !defined(ACE_WIN32) && !defined(DAF_HAS_EVENTFD)
# define DAF_HAS_EVENTFD 1
#endif

#define DAF_UNUSED_ARG(ID) \
template <typename T> inline void ID ## _UNUSED(const T& = (ID)) {}

// Exception catching for abi (linux)
#if !defined(ACE_WIN32) && defined(ACE_HAS_PTHREADS)
# include <cxxabi.h>
# define DAF_CATCH_ALL catch (const abi::__forced_unwind& ) { throw; } catch(...)
# define DAF_CATCH_ALL_ACTION(ACTION) catch (const abi::__forced_unwind& ) { ACTION; throw; } catch(...)
#else
# define DAF_CATCH_ALL catch (...)
# define DAF_CATCH_ALL_ACTION(ACTION) catch (...)
#endif

#define DAF_HANDLES_THREAD_CLEANUP  1 // If we changed this to zero change the other in OS.cpp Thread_Adapter::invoke

#if defined(__ANDROID_API__)
# define DAF_LACKS_EFD_SEMAPHORE 1 // This affects the SignalHandler when using file descriptors
#endif
/********** TAF Configuration *********************/

/* Compile Time Options - Maybe need a TAFConfig.h eventually */
#define TAF_HAS_UNIQUE_DEFAULT_POA  1

#endif // DAF_CONFIG_H
