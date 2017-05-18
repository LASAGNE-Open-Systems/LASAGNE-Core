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
#ifndef TAF_TAF_H
#define TAF_TAF_H

#include "TAF_export.h"

#include "TAFDebug.h"

#include <daf/DAF.h>

/*************** TAO Conditional Support Checks ***************/

#include <tao/Version.h>

#if !defined(TAO_GTEQ_VERSION)
# define TAO_GTEQ_VERSION(MAJOR,MINOR,BETA) \
    ((TAO_MAJOR_VERSION >  MAJOR) || \
    ((TAO_MAJOR_VERSION == MAJOR) && (TAO_MINOR_VERSION >  MINOR)) || \
    ((TAO_MAJOR_VERSION == MAJOR) && (TAO_MINOR_VERSION == MINOR)  && (TAO_BETA_VERSION >= BETA)))
#endif

/**************************************************************/

#define TAFDISCOVERY_DLL_NAME   ACE_TEXT("TAFDiscovery")

/**
  \namespace TAF
  \brief Distributed Computing extensions to TAO based on #DAF

  TAF is responsible for providing a developer friendly environment
  for writing distributed concurrent programs. It contains the following:
  - TAFServer - An executable container for managing service based applications
  - CORBA management

*/
namespace TAF {
    TAF_Export bool isDiscoveryEnabled(void);   // taf/IORQueryService.cpp
    TAF_Export bool isSecurityEnabled(void);    // taf/ORBManager.cpp
}

#endif
