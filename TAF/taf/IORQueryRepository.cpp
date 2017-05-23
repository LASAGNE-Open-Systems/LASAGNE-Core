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
#define TAF_IORQUERYREPOSITORY_CPP

#include "IORQueryRepository.h"

#include <daf/PropertyManager.h>

namespace TAF
{
    /***********************************************************************************************/

    bool    isDiscoveryEnabled(void)
    {
        return DAF::get_numeric_property<bool>(TAF_DISCOVERYENABLE, true, true);
    }

    /***********************************************************************************************/

    IORServantRepository::IORServantRepository(void)
    {
    }

    IORServantRepository::~IORServantRepository(void)
    {
        this->clear();
    }

    IORServantRepository::operator ACE_SYNCH_MUTEX & () const
    {
        return this->lock_;
    }

    int
    IORServantRepository::registerQueryService(CORBA::Object_ptr p, const std::string &name)
    {
        const std::string ident(DAF::trim_string(name));

        if (p && ident.length()) do {

            ACE_GUARD_REACTION(ACE_SYNCH_MUTEX, taf_mon, *this, break); // Protect List

            for (iterator it(this->begin()); it != this->end(); it++) {
                if (ident == it->ident()) { // If already exists - Just replace Object_ptr
                    it->out() = CORBA::Object::_duplicate(p); return 0; // Replace Duplicate
                }
            }

            this->push_back(IORQueryServant(p,ident)); return 0;

        } while (false);

        return -1;
    }

    int
    IORServantRepository::unregisterQueryService(const std::string &name)
    {
        const std::string ident(DAF::trim_string(name));

        if (ident.length()) do {

            ACE_GUARD_REACTION(ACE_SYNCH_MUTEX, taf_mon, *this, break); // Protect List

            for (iterator it(this->begin()); it != this->end(); it++) {
                if (ident == it->ident()) {
                    this->erase(it); return 0;
                }
            }

        } while (false);

        return -1;
    }

}
