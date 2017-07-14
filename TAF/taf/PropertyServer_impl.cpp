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
#define TAF_PROPERTYSERVER_IMPL_CPP

#include "PropertyServer_impl.h"

#include <daf/PropertyManager.h>

namespace TAF
{
    char *
    PropertyServer_impl::get_property(const char *ident)
    {
        if (ident && DAF_OS::strlen(ident) > 0) try {
            return CORBA::string_dup(ThePropertyRepository()->get_property(ident, true).c_str());
        } catch (const DAF::NotFoundException&) {
        } catch (const DAF::IllegalArgumentException&) {
            throw CORBA::BAD_PARAM();
        } catch (const DAF::IllegalThreadStateException&) {
            throw CORBA::BAD_OPERATION();
        } DAF_CATCH_ALL {
            throw CORBA::INVALID_ACTIVITY();
        }

        throw CORBA::INV_IDENT();
    }

    void
    PropertyServer_impl::set_property(const char *ident, const char *value)
    {
        if (ident && DAF_OS::strlen(ident) > 0) try {
            if (ThePropertyRepository()->set_property(ident, (value ? value : "")) == 0) {
                return;
            }
        } catch (const DAF::IllegalThreadStateException&) {
            throw CORBA::BAD_OPERATION();
        } catch (const DAF::IllegalArgumentException&) {
            throw CORBA::BAD_PARAM();
        } DAF_CATCH_ALL {
            throw CORBA::INVALID_ACTIVITY();
        }
        throw CORBA::INV_IDENT();
    }

    void
    PropertyServer_impl::del_property(const char *ident)
    {
        if (ident && DAF_OS::strlen(ident) > 0) try {
            ThePropertyRepository()->del_property(ident); return;
        } catch (const DAF::IllegalThreadStateException&) {
            throw CORBA::BAD_OPERATION();
        } catch (const DAF::IllegalArgumentException&) {
            throw CORBA::BAD_PARAM();
        } DAF_CATCH_ALL {
            throw CORBA::INVALID_ACTIVITY();
        }
        throw CORBA::INV_IDENT();
    }

    taf::PropertyValueSequence *
    PropertyServer_impl::list_properties(void)
    {
        DAF::PropertyManager::property_list_type property_list;
        try {
            CORBA::ULong max_len = ThePropertyRepository()->list_properties(property_list), len = 0, idx = 0;
            taf::PropertyValueSequence_var propertySeq(new taf::PropertyValueSequence(max_len)); propertySeq->length(len);
            for (DAF::PropertyManager::property_list_type::const_iterator it = property_list.begin(); it != property_list.end(); it++) {
                if ((idx = len++) >= max_len) {
                    break;
                } else propertySeq->length(len);
                propertySeq[idx].ident = it->first.c_str();
                propertySeq[idx].value = it->second.c_str();
            }
            return propertySeq._retn();
        } DAF_CATCH_ALL {}
        throw CORBA::BAD_OPERATION();
    }
} // namespace TAF
