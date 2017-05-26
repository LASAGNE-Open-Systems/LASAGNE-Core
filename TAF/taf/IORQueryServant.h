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
#ifndef TAF_IORQUERYSERVANT_H
#define TAF_IORQUERYSERVANT_H

#include "TAF.h"

#include <daf/ObjectRef_T.h>

#include <tao/Object.h>

#include <ace/INET_Addr.h>

#include <string>
#include <list>

namespace TAF
{
    // Need to use our proxy as CORBA::Object_var causes linkage errors
    typedef class DAF::ObjectRef<CORBA::Object> IORServant_ref;

    class TAF_Export IORQueryServant : public IORServant_ref
    {
    public:

        IORQueryServant(void) {} // Default Constructor
        IORQueryServant(const IORQueryServant &); // Copy Constructor
        IORQueryServant(const CORBA::Object_ptr obj, const std::string &ident);

        const std::string & ident(void) const { return this->ident_; }

        int is_ident(const std::string &ident) const;

    private:

        const std::string ident_;
    };

    typedef class std::list<IORQueryServant>    IORQueryServantList;

} // namespace TAF

#endif // TAF_IORQUERYSERVANT_H
