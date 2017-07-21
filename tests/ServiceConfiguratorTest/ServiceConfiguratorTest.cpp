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
#include <daf/ServiceGestaltLoader.h>

#include <daf/PropertyManager.h>

#include <ace/ARGV.h>

#include <iostream>

struct ConfigServiceLoader : DAF::ServiceGestaltLoader {
    ConfigServiceLoader(DAF::ServiceGestalt & gestalt) : DAF::ServiceGestaltLoader(gestalt) {}
    virtual int process_directive(const value_type &val);
    virtual const char * config_switch(void) const
    {
        return DAF_PROPERTIES;
    }
};

const std::string filearg("TEST.conf:A,B,C,D,E");
std::string ok_result("a6c6wy9b3e5d4nf8v4x");
std::string result; // Holds the result

const char *TEST_NAME = "ServiceConfiguratorTest";

int
ConfigServiceLoader::process_directive(const value_type &val)
{
    const std::string key(val.first), arg(DAF::format_args(val.second));

    DAF::set_property(key, arg);

    result.append(key).append(arg);

    if (arg == val.second) {
        ACE_DEBUG((LM_INFO, ACE_TEXT("%C=%C\n"), key.c_str(), val.second.c_str()));
    }
    else {
        ACE_DEBUG((LM_INFO, ACE_TEXT("%C=%C [%C]\n"), key.c_str(), val.second.c_str(), arg.c_str()));
    }

    return 0;
}

int main(int argc, char *argv[])
{
    ACE_DEBUG((LM_INFO, ACE_TEXT("(%P|%t) %T - %C\n"), TEST_NAME));

    DAF::ServiceGestalt gestalt(argc ? argv[0] : 0);

    ConfigServiceLoader cfgLoader(gestalt);

    for (const char *ace_root = DAF_OS::getenv("ACE_ROOT"); ace_root;) {
        ok_result.append(ace_root).append("/my bin"); break;
    }

    ACE_ARGV args(true); if (argc) {  // Put In Program Name
        args.add(argv[0]);
    }

    std::string cfg_switch("-"); cfg_switch.append(cfgLoader.config_switch());
    args.add(cfg_switch.c_str());
    args.add(filearg.c_str());

    argc = args.argc();

    {
        DAF::debug(5);  // Get lots of output

        if (cfgLoader.load(argc, args.argv(), true) == 0)     {
            if (cfgLoader.process_directives() == 0)    {
                if (result == ok_result) {
                    ACE_ERROR_RETURN((LM_INFO, ACE_TEXT("\nreturned = \"%C\"\nexpected = \"%C\"\nTEST PASSED OK!!\n")
                        , result.c_str(), ok_result.c_str()), 0);
                }
            }
        }
    }

    ACE_ERROR_RETURN((LM_INFO, ACE_TEXT("\nreturned = \"%C\"\nexpected = \"%C\"\nTEST FAILED!!\n")
        , result.c_str(), ok_result.c_str()), -1);
}
