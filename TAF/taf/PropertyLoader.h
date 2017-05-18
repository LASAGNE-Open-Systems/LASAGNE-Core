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
#ifndef TAF_PROPERTYLOADER_H
#define TAF_PROPERTYLOADER_H

#include "TAF.h"

#include <daf/Configurator.h>
#include <daf/ARGV.h>

#define CORBA_ORB_ARGUMENT_SPECIFIER    ACE_TEXT("-ORB")
#define TAF_PROPERTIES_FLAG             ACE_TEXT("-" TAF_PROPERTIES)

namespace TAF
{
    class TAF_Export PropertyLoader : public DAF::Configurator, DAF_ARGV
    {
    public:

        PropertyLoader(int &argc, ACE_TCHAR *argv[], bool use_env = true);

        int load_properties(int &argc, ACE_TCHAR **&argv);

    protected:

        virtual int load_property(const property_key_type &key, const property_val_type &val);

    private:

        int parse_args(int &argc, ACE_TCHAR **&argv);

        const char * config_switch(void) const
        {
            return TAF_PROPERTIES;
        }
    };

} // namespace TAF

#endif  // TAF_PROPERTYLOADER_H
