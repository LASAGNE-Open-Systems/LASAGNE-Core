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
#ifndef TAF_PROPERTYSERVER_IMPL_H
#define TAF_PROPERTYSERVER_IMPL_H

#include "TAF.h"

#include "PropertyServerS.h"

namespace TAF
{
    typedef struct TAF_Export PropertyServer_impl : virtual POA_taf::PropertyServer
    {
        virtual char *  get_property(const char *ident);
        virtual void    set_property(const char *ident, const char *value);
        virtual void    del_property(const char *ident);

        virtual taf::PropertyValueSequence * list_properties(void);

    } PropertyServerImpl;

} // namespace TAF

typedef struct TAF::PropertyServer_impl     TAFPropertyServerImpl;

#endif
