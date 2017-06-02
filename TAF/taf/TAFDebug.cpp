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
#define TAF_DEBUG_CPP

#include "TAFDebug.h"

#include <daf/PropertyManager.h>
#include <daf/OS.h>

namespace {
    void    taf_print_debug(const char *id, int dbug)
    {
        char s[32]; if (dbug) {
            DAF_OS::sprintf(s, ACE_TEXT("ON[%d]"), dbug);
        }
        else {
            DAF_OS::strcpy(s, ACE_TEXT("OFF"));
        }
        ACE_DEBUG((LM_INFO, ACE_TEXT("%s=%s - %T\n"), (id ? id : "Debug"), s));
    }
}

namespace TAO
{
    void  debug(int onoff) { TAO_debug_level = ace_range(0, 99, DAF_OS::abs(onoff)); }
    int   debug(void)
    {
        static struct TAODebugging {
            TAODebugging(int tao_debug) {
                TAO::debug(DAF::get_numeric_property<int>(TAO_DEBUGGING, tao_debug, true));
            }
        } taoDebugging_(TAO_debug_level); ACE_UNUSED_ARG(taoDebugging_);

        return int(TAO_debug_level);
    }
}

TAO_Debug::TAO_Debug(int dbug) : debug_(TAO::debug())
{
    if (this->debug_ != (TAO::debug(dbug), TAO::debug())) {
        TAO_Debug::print_debug();
    }
}

TAO_Debug::~TAO_Debug(void)
{
    if (this->debug_ != TAO::debug()) {
        TAO::debug(this->debug_); TAO_Debug::print_debug();
    }
}

void
TAO_Debug::print_debug(void)
{
    taf_print_debug(TAO_DEBUGGING, TAO::debug());
}

namespace TAO
{
    void  debug_orb(int onoff) { TAO_orbdebug = ace_range(0, 99, DAF_OS::abs(onoff)); }
    int   debug_orb(void)
    {
        static struct ORBDebugging {
            ORBDebugging(int orb_debug) {
                TAO::debug(DAF::get_numeric_property<int>(TAO_ORBDEBUGGING, orb_debug, true));
            }
        } orbDebugging_(TAO_orbdebug); ACE_UNUSED_ARG(orbDebugging_);

        return int(TAO_orbdebug);
    }
}

TAO_ORBDebug::TAO_ORBDebug(int dbug) : debug_(TAO::debug_orb())
{
    if (this->debug_ != (TAO::debug_orb(dbug), TAO::debug_orb())) {
        TAO_ORBDebug::print_debug();
    }
}

TAO_ORBDebug::~TAO_ORBDebug(void)
{
    if (this->debug_ != TAO::debug()) {
        TAO::debug(this->debug_); TAO_Debug::print_debug();
    }
}

void
TAO_ORBDebug::print_debug(void)
{
    taf_print_debug(TAO_ORBDEBUGGING, TAO::debug_orb());
}

namespace TAF
{
    namespace {
        int _taf_debug = 0;
    }

    void    debug(int onoff)    { _taf_debug = ace_range(0, 99, DAF_OS::abs(onoff)); }
    int     debug(void)
    {
        static struct TAFDebugging {
            TAFDebugging(int taf_debug) {
                TAF::debug(DAF::get_numeric_property<int>(TAF_DEBUGGING, taf_debug, true));
            }
        } tafDebugging_(_taf_debug); ACE_UNUSED_ARG(tafDebugging_);

        return _taf_debug;
    }
}

TAF_Debug::TAF_Debug(int dbug) : debug_(TAF::debug())
{
    if (this->debug_ != (TAF::debug(dbug), TAF::debug())) {
        TAF_Debug::print_debug();
    }
}

TAF_Debug::~TAF_Debug(void)
{
    if (this->debug_ != TAF::debug()) {
        TAF::debug(this->debug_); TAF_Debug::print_debug();
    }
}

void
TAF_Debug::print_debug(void)
{
    taf_print_debug(TAF_DEBUGGING, TAF::debug());
}
