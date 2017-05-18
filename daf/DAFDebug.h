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
#ifndef DAF_DEBUG_H
#define DAF_DEBUG_H

#include "DAF_export.h"

/**
  \defgroup debug Developer Level Debug

  This group gives a number of constructs and the ability to control the debugging
  output through various layers.
*/

/** \class ACE_Debug
 * \brief Scoped ACE debug output
 *
 * This structure allows the scoped control of ACE layer debugging output.
 * It allows a developer to control the output from functions and classes
 * in the ACE libraries. ACE level debugging can be used for troubleshooting
 * the loading of services and the service containers.
 *
 * Globally this can also be controlled via CLI with -ACEDebug
 * You can also use the ACE::debug() function
 * \ingroup debug
 */
class DAF_Export ACE_Debug {
    bool debug_;
public:
    /** Set the ACE Debug level (on /off)
    \param dbug true (ON) false(OFF) to use of the lifetime of the class
      */
    ACE_Debug(bool dbug = true);

    ~ACE_Debug(void);

    /** Print the Current Value of ACE debug .  */
    static void print_debug(void);
};

namespace DAF {
    /**
     * \brief DAF Level debug setting
     *
     * Global DAF level debug verbosity can be controlled from the CLI with
     *  -DAFDebug <n>
     * \return current level of debug verbosity for the DAF Layer
     * \ingroup debug
     */
    DAF_Export int  debug(void);

    /**
     * \brief DAF Level debug control
     *
     * Controls the DAF layer debug output. Allows a developer to get different
     * Log level information out of DAF infrastructure. This is primarily related
     * to thread control, service lifecycle control.
     * \param onoff change log level in DAF. This is value from 0 (off) 1-10,
     *              with differing levels of verbosity (low 1, high 10)
     * \ingroup debug
     */
    DAF_Export void debug(int onoff);
}

/** \class DAF_Debug
 * \brief Scoped control of DAF level debug verbosity
 *
 * This construct provides scoped DAF level debug verbosity. It allows a developer
 * to provide a level of debug verbosity in the DAF layer. The life time of the
 * class controls will restore the current level. This concept is not thread safe.
 * \ingroup debug
 */
class DAF_Export DAF_Debug {
    int debug_;
public:
    /** Desired level of DAF debug verbosity
     * \param dbug desired debug output over the lifetime of the class.
     */
    DAF_Debug(int dbug = DAF::debug() + 1);

    ~DAF_Debug(void);


    static void print_debug(void);
};

#endif // DAF_DEBUG_H
