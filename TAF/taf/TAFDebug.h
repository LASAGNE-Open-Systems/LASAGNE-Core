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
#ifndef TAF_DEBUG_H
#define TAF_DEBUG_H

#include "TAF_export.h"

#include <tao/debug.h>
#include <daf/DAFDebug.h>

namespace TAO
{
    /**
     * \brief TAO layer debug level.
     *
     * Can be controlled from the CLI with '-TAODebug <n>'
     * \return current TAO debug level.
     * \ingroup debug
     */
    TAF_Export int   debug(void);

    /**
     * \brief TAO layer debug control
     *
     * Control the TAO layer debug level and verbosity.
     * \param onoff set the debug level from range 0..99
     * (see TAO_debug_level )
     * \ingroup debug
     */
    TAF_Export void  debug(int onoff);
}

/**
 * \class TAO_Debug
 * \brief Scoped control of TAO layer debug
 *
 * This structure allows the scoped control of TAO layer debugging output.
 * It allows a developer to control the output from functions and classes
 * in the TAO libraries. TAO level debugging can be used for troubleshooting
 * CORBA, ORB Services and Transport level information.
 *
 * The value is controlled by the lifecycle of the class, it will restore
 * the debug level on destruction. This concept is not thread safe.
 * \ingroup debug
 */
class TAF_Export TAO_Debug {
    int debug_;
public:

    /**
      \param dbug Desired level of TAO debug over the lifetime of the class
     */
    TAO_Debug(int dbug = TAO::debug() + 1);
    ~TAO_Debug(void);

    static void print_debug(void);
};

namespace TAO
{
    /**
     * \brief TAO ORB layer debug level.
     *
     * Can be controlled from the CLI with '-TAOORBDebug <n>'
     *
     * \return current TAO debug level.
     * \ingroup debug
     */
    TAF_Export int   debug_orb(void);

    /**
     * \brief TAO ORB layer debug control
     *
     * Control the TAO ORB layer debug level and verbosity.
     * \param onoff set the debug level from range 0..99
     * (see TAO_debug_level )
     * \ingroup debug
     */
    TAF_Export void  debug_orb(int onoff);
}

/**
 * \class TAO_ORBDebug
 * \brief Scoped control of TAO ORB layer debug
 *
 * This structure allows the scoped control of TAO ORB layer debugging output.
 * It allows a developer to control the output from functions and classes
 * in the TAO libraries. TAO level debugging can be used for troubleshooting
 * CORBA, ORB Services and Transport level information.
 *
 * The value is controlled by the lifecycle of the class, it will restore
 * the debug level on destruction. This concept is not thread safe.
 * \ingroup debug
 */
class TAF_Export TAO_ORBDebug {
    int debug_;
public:
    TAO_ORBDebug(int dbug = TAO::debug_orb() + 1);
    ~TAO_ORBDebug(void);

    static void print_debug(void);
};

namespace TAF
{
    /**
     * \brief TAF layer debug level.
     *
     * Can be controlled from the CLI with '-TAFDebug <n>'
     *
     * \return current TAO debug level.
     * \ingroup debug
     */
    TAF_Export int   debug(void);

    /**
     * \brief TAF layer debug control
     *
     * Control the TAF layer debug level and verbosity.
     * \param onoff set the debug level from range 0..99
     * \ingroup debug
     */
    TAF_Export void  debug(int onoff);
}

/**
 * \class TAF_Debug
 * \brief Scoped control of TAF layer debug
 *
 * This structure allows the scoped control of TAF layer debugging output.
 * It allows a developer to control the output from functions and classes
 * in the TAF libraries. TAF level debugging can be used for troubleshooting
 * Service Loading, discovery, dds connectors.
 *
 * The value is controlled by the lifecycle of the class, it will restore
 * the debug level on destruction. This concept is not thread safe.
 * \ingroup debug
 */
class TAF_Export TAF_Debug {
    int debug_;
public:
    TAF_Debug(int dbug = TAF::debug() + 1);
    ~TAF_Debug(void);

    static void print_debug(void);
};

#endif // TAF_DEBUG_H
