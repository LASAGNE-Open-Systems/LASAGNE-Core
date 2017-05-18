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
#include <daf/DAF.h>
#include <daf/ARGV.h>

static const char *testargs1[] = {
    "-Argument1", "val1",
    "'-Argument 2'", "'val 2'",
    0
};

static const char *testargs2 = "-Argument1 val1 '-Argument 2' \"val 2\" ";

int main(int argc, char *argv[])
{
    ACE_DEBUG((LM_INFO, "\nARGV(int argc, char *argv[], bool substitute_env_args = true, bool quote_args = false);\n"));
    DAF::ARGV   args00(argc, argv, true); DAF::print_argv(args00);

    ACE_DEBUG((LM_INFO, "\nARGV(const char *argv[], bool substitute_env_args = true, bool quote_args = false);\n"));
    DAF::ARGV   args10(testargs1, true);  DAF::print_argv(args10);

    ACE_DEBUG((LM_INFO, "\nARGV(const char *args, bool substitute_env_args = true);\n\t%s\n", testargs2));
    DAF::ARGV   args20(testargs2, true);  DAF::print_argv(args20);

    ACE_DEBUG((LM_INFO, "\nARGV::add(const char *args, bool quote_args = true);\n"));
    DAF::ARGV   args30; args30.add(testargs2, true);  DAF::print_argv(args30);
    ACE_DEBUG((LM_INFO, "ARGV::add(const char *args, bool quote_args = false);\n"));
    DAF::ARGV   args31; args31.add(testargs2, false); DAF::print_argv(args31);

    return 0;
}
