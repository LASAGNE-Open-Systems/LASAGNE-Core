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
#define TAF_IORQUERYSERVANT_CPP

#include "IORQueryServant.h"

namespace TAF
{
    IORQueryServant::IORQueryServant(const CORBA::Object_ptr obj, const std::string &ident)
        : IORServant_ref(CORBA::Object::_duplicate(obj)), ident_(ident)
    {
    }

    IORQueryServant::IORQueryServant(const IORQueryServant &servant)
        : IORServant_ref(servant), ident_(servant.ident())
    {
    }

    int
    IORQueryServant::is_ident(const std::string &ident) const
    {
        if (ident.length()) {

            if (CORBA::is_nil(this->in())) {
                throw CORBA::INV_OBJREF();             // Not Activated
            }
            else if (ident == this->ident()) {
                return true;
            }
            else if ((*this)->_is_a(ident.c_str())) {  // This may also throw
                return true;
            }

        }

        return false;
    }

} // namespace TAF
