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
#include <daf/ServiceLoader.h>

#include <ace/ARGV.h>

#include <iostream>

struct ConfigServiceLoader : DAF::ServiceLoader {
    virtual int process_directive(const value_type &val);
    virtual const char * config_switch(void) const
    {
        return DAF_PROPERTIES;
    }
};

const std::string filearg("TEST.conf:A,B,C,D,E");
std::string ok_result("a6c6b3e5d4f8x");
std::string result; // Holds the result

int
ConfigServiceLoader::process_directive(const value_type &val)
{
    result.append(val.first).append(val.second);

    ACE_ERROR_RETURN((LM_INFO, ACE_TEXT("%s=%s\n"), val.first.c_str(), val.second.c_str()), 0);
}

int main(int argc, char *argv[])
{
    ConfigServiceLoader cfgLoader;

    for (const char *daf_root = DAF_OS::getenv("DAF_ROOT"); daf_root;) {
        ok_result.append(daf_root).append("/my bin"); break;
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
                    ACE_ERROR_RETURN((LM_INFO, ACE_TEXT("PASSED OK!!\n")), 0);
                }
            }
        }
    }
    ACE_ERROR_RETURN((LM_INFO, ACE_TEXT("TEST FAILED!!\n")), -1);
}
