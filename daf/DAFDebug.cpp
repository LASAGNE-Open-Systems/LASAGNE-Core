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
#define DAF_DEBUG_CPP

#include "DAFDebug.h"

#include "OS.h"
#include "PropertyManager.h"

#include <ace/ACE.h>
#include <ace/Log_Msg.h>

namespace {
    void    daf_print_debug(const char *id, int dbug)
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

/*******************************************************************************/

ACE_Debug::ACE_Debug(bool dbug) : debug_(ACE::debug())
{
    if (this->debug_ != (ACE::debug(dbug), ACE::debug())) {
        ACE_Debug::print_debug();
    }
}

ACE_Debug::~ACE_Debug(void)
{
    if (this->debug_ != ACE::debug()) {
        ACE::debug(this->debug_); ACE_Debug::print_debug();
    }
}

void
ACE_Debug::print_debug(void)
{
    daf_print_debug(ACE_DEBUGGING, int(ACE::debug()));
}

/*******************************************************************************/

namespace DAF
{
    namespace {
        int _daf_debug = 0;
    }

    void    debug(int onoff)    { _daf_debug = ace_range(0, 99, DAF_OS::abs(onoff)); }
    int     debug(void)
    {
        static struct DAFDebugging {
            DAFDebugging(int def_debug) {
                DAF::debug(DAF::get_numeric_property<int>(DAF_DEBUGGING, def_debug, true));
            }
        } dafDebugging_(_daf_debug); ACE_UNUSED_ARG(dafDebugging_);

        return _daf_debug;
    }
}

DAF_Debug::DAF_Debug(int dbug) : debug_(DAF::debug())
{
    if (this->debug_ != (DAF::debug(dbug), DAF::debug())) {
         DAF_Debug::print_debug();
    }
}

DAF_Debug::~DAF_Debug(void)
{
    if (this->debug_ != DAF::debug()) {
        DAF::debug(this->debug_); DAF_Debug::print_debug();
    }
}

void
DAF_Debug::print_debug(void)
{
    daf_print_debug(DAF_DEBUGGING, DAF::debug());
}
