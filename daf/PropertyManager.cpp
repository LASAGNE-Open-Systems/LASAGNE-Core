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
#define DAF_PROPERTYMANAGER_CPP

#include "PropertyManager.h"

#include <ace/Singleton.h>

#include <iostream>

namespace DAF
{
    namespace {
        struct AlphabeticalPredicate : std::binary_function < Configurator::value_type, Configurator::value_type, bool > {
            bool operator () (const first_argument_type &l, const second_argument_type &r) const {
                return DAF_OS::strcasecmp(r.first.c_str(), l.first.c_str()) > 0; // this does a LowerCase Compare
            }
        };
    }

    std::string
    PropertyManager::get_property(const property_key_type &ident, bool use_env) const
    {
        for (const property_key_type key(DAF::trim_string(ident)); key.length();) {

            ACE_READ_GUARD_REACTION(ACE_SYNCH_RW_MUTEX, mon, *this, DAF_THROW_EXCEPTION(DAF::ResourceExhaustionException));

            try {
                return DAF::format_args(this->at(key), use_env);
            } catch (const std::out_of_range &) {
                /* Not in Map */
            }

            if (use_env) {
                const char * env_val = DAF_OS::getenv(key.c_str()); use_env = false;
                if (env_val && DAF_OS::strlen(env_val)) {
                    static_cast< ACE_SYNCH_RW_MUTEX & >(*this).tryacquire_write_upgrade(); // Try to switch to write access
                    if (const_cast<PropertyManager*>(this)->load_property(key, DAF::trim_string(env_val)) == 0) {
                        continue;
                    }
                }
            }

            DAF_THROW_EXCEPTION(DAF::NotFoundException);
        }

        DAF_THROW_EXCEPTION(DAF::IllegalPropertyException);
    }

    std::string
    PropertyManager::get_property(const property_key_type &ident, const property_val_type &default_val, bool use_env) const
    {
        try {
            for (property_val_type val; val.assign(this->get_property(ident, use_env)).length();) {
                return val;
            }
        }
        catch (const DAF::IllegalArgumentException&) {
        }

        return DAF::format_args(default_val);
    }

    int
    PropertyManager::set_property(const property_key_type &ident, const property_val_type &value)
    {
        for (const property_key_type key(DAF::trim_string(ident)); key.length();) {
            ACE_WRITE_GUARD_REACTION(ACE_SYNCH_RW_MUTEX, mon, *this, DAF_THROW_EXCEPTION(DAF::ResourceExhaustionException));
            return this->load_property(key, DAF::trim_string(value));
        }

        DAF_THROW_EXCEPTION(DAF::IllegalPropertyException);
    }

    void
    PropertyManager::del_property(const property_key_type &ident)
    {
        for (const property_key_type key(DAF::trim_string(ident)); key.length();) {
            ACE_WRITE_GUARD_REACTION(ACE_SYNCH_RW_MUTEX, mon, *this, DAF_THROW_EXCEPTION(DAF::ResourceExhaustionException));
            this->erase(key); return;
        }
        DAF_THROW_EXCEPTION(DAF::IllegalPropertyException);
    }

    int
    PropertyManager::list_properties(property_list_type &value_list) const
    {
        value_list.clear();
        {
            ACE_READ_GUARD_REACTION(ACE_SYNCH_RW_MUTEX, mon, *this, DAF_THROW_EXCEPTION(DAF::ResourceExhaustionException));
            for (const_iterator it = this->begin(); it != this->end(); it++) try {
                value_list.push_back(property_list_type::value_type(it->first, DAF::format_args(it->second)));
            } DAF_CATCH_ALL { /* Ignore any Errored Formatting */ }
            value_list.sort(AlphabeticalPredicate());
        }
        return int(value_list.size());
    }

    int
    PropertyManager::print_properties(void) const
    {
        const int tab_position = 24;

        property_list_type properties;
        {
            if (this->list_properties(properties)) {

                std::stringstream ss;

                for (property_list_type::const_iterator it = properties.begin(); it != properties.end(); it++) {
                    ss << it->first  // ident
                       << std::string(ace_max(1, int(tab_position - it->first.length())), ' ')
                       << "= "
                       << it->second // param
                       << std::endl;
                }

                ss << std::endl; std::cout << ss.str(); // Make sure it is out
            }
        }
        return int(properties.size());
    }

    /**********************************************************************************/

    PropertySingleton::PropertySingleton(bool preload)
    {
        if (preload) { // Preload DAFProperties from environment variable if it exists
            for (const char * properties = DAF_OS::getenv(this->config_switch()); properties;) try {
                this->load_file_profile(properties); break;
            } DAF_CATCH_ALL {
                ACE_ERROR_BREAK((LM_WARNING, ACE_TEXT("DAF (%P | %t) PropertySingleton:")
                    ACE_TEXT(" Failed to preload property singleton with %s; value=\"%s\"\n")
                    , this->config_switch(), properties));
            }
        }
    }

    /**********************************************************************************/

    DAF::PropertySingleton * PropertyRepository(void)
    {
        return ACE_DLL_Singleton_T<DAF::PropertySingleton, ACE_SYNCH_MUTEX>::instance();
    }
}
