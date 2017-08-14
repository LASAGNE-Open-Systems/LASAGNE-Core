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
#define DAF_ARGV_CPP


#include "ARGV.h"

#include "daf/DAF.h"

namespace DAF
{
    int
    ARGV::add(const char args[], bool quote_args)
    {
        ACE_ARGV argv(this->substitute_env_args()); return argv.add(args, quote_args) ? -1 : this->push_argv(argv);
    }

    int
    ARGV::add(const char *args[], bool quote_args)
    {
        ACE_ARGV argv(this->substitute_env_args()); return argv.add(const_cast<char **>(args), quote_args) ? -1 : this->push_argv(argv);
    }

    int
    ARGV::push_argv(const ACE_ARGV &argv)
    {
        for (int i = 0; i < argv.argc(); i++) {
            this->args_list_.push_back(DAF::trim_string(const_cast<ACE_ARGV &>(argv)[i]));
            if (this->args_.add(this->args_list_.back().c_str(), true)) {
                return -1;
            }
        }
        return 0;
    }

} // namespace DAF
