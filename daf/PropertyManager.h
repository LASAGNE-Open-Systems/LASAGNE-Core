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
#ifndef DAF_PROPERTYMANAGER_H
#define DAF_PROPERTYMANAGER_H

#include "DAF.h"
#include "Configurator.h"

namespace DAF
{
    /**
    * \class PropertyManager
    * \brief Key-Value Property Map
    *
    * This base level class is responsible for loading properties from a file
    * via the Configurator mechanism and providing accessor methods to to the
    * property map. It also has the ability to cache environment variables as
    * configuration properties.
    * Some behaviour to be aware of when using Properties.
    * - properties are evaluated when requested via a #get_property method.
    * - properties can have variable values. These are generally prefix with '%'
    *        Format a message simular to the ACE Log message mechanisms.  Valid
    *        options (prefixed by "%", as in printf format strings) include:
    *         - 'P' : print out the current process id
    *         - 'p' : print out current property deliminated by {...}
    *         - 'Dg': print GMT timestamp 2015-10-08 13:15:35 format
    *         - 'DG': print GMT timestamp Thursday, 8th October 2015 1:15:35PM format
    *         - 'Dl': print LOCAL timestamp 2015-10-08 13:15:35 format
    *         - 'DL': print LOCAL timestamp Thursday, 8th October 2015 1:15:35PM format
    *         - 'Tu': print UTC Time
    *         - 't' : print thread id (1 if single-threaded)
    *         - 'H' : print HostName
    *         - "%" : print out a single percent sign, "%"
    * - for example properties with value "%Dg" will evaluate a timestamp
    *   every time the value is called and evaluated.
    */
    class DAF_Export PropertyManager : public DAF::Configurator
    {
    public:

        /**
        Access the property within the property map with a key value.
        The order of precedence is :
        - return the key-value already present in the property map.
        - check the environment variable present with key name. Insert the
          environment variable value into the property map.
          return the key-value present in the property map.
        \param ident the key to use to find an entry in the map.
        \param use_env additionally check the environment variable if
        it doesn't already exist in the property map and insert it into the map.
        \throw IllegalArgumentException when the ident is not a valid key and
        the corresponding environment variable is not specified.
        \return property value
         */
        std::string get_property(const property_key_type &ident, bool use_env = true) const;

        /**
        Access the property within the property map with a key value.
        This variation also takes a default parameter as an input. This allows
        the developer to specify in one call a configuration call.
        \param ident the key to use to find an entry in the map.
        \param default_val default value to use if the key-value is not present
        in the map
        \param use_env additionally check the environment variable if
        it doesn't already exist in the property map and insert it into the map.

        \return property value
         */
        std::string get_property(const property_key_type &ident, const property_val_type &default_val, bool use_env = true) const;

        /**
        Set the property key with value.
        \param ident key into the map
        \param value value associated with key in property map
        \return result from #load_property
        */
        int         set_property(const property_key_type &ident, const property_val_type &value);

        /**
        Delete the property associated with key from the map.
        \param ident key to remove from map.
        */
        void        del_property(const property_key_type &ident);

        /**
        Return all properties in the property map.
        \param value_list returned list of all the properties
        \return property count in value_list.
        */
        int         list_properties(property_list_type &value_list) const;

        /**
        Print all the properties to std::cout
        */
        int         print_properties(void) const; // Returns number of lines (properties) printed

        /**
        Templated variants to perform numeric conversion of the map's key value
        into a numeric value.
        \note makes use of atof for string to number conversion.
        Usage:

              int the_value = properties->get_numeric_property<int>("key");
              bool bool_val = properties->get_numeric_property<bool>("key");
              double d_val = properties->get_numeric_property<double>("key");

        \sa #get_property
        */
        template <typename T>
        T get_numeric_property(const property_key_type &ident, bool use_env = true) const;

        /**
        Templated variants to perform numeric conversion of the map's key value
        into a numeric value. Includes default value
        \note usage of bool can create a conflict with template ambiguity, you
        must specify all three values.
        Usage:

              int the_value = properties->get_numeric_property<int>("key", 10);
              bool bool_val = properties->get_numeric_property<bool>("key", true, true );
              double d_val = properties->get_numeric_property<double>("key", 10.0);

        \sa #get_property
        */
        template <typename T>
        T get_numeric_property(const property_key_type &ident, const T &default_val, bool use_env = true) const;
    };


    template <typename T> inline T
    PropertyManager::get_numeric_property(const property_key_type &ident, bool use_env) const
    {
        return static_cast<T>(DAF_OS::atof(this->get_property(ident, use_env).c_str()));
    }


    template <typename T> inline T
    PropertyManager::get_numeric_property(const property_key_type &ident, const T &default_val, bool use_env) const
    {
        try {
            return this->get_numeric_property<T>(ident, use_env);
        }
        catch (const DAF::IllegalArgumentException &) {
        }

        return default_val;
    }


    template <> inline bool  /* Sepecialization for bool type property */
    PropertyManager::get_numeric_property<bool>(const property_key_type &ident, bool use_env) const
    {
        const property_val_type val(this->get_property(ident, use_env));
        if (DAF_OS::strncasecmp(val.c_str(), ACE_TEXT("true"), 4) == 0) {
            return true;
        }
        return static_cast<bool>(DAF_OS::atoi(val.c_str()));
    }


    template <> inline bool
    PropertyManager::get_numeric_property<bool>(const property_key_type &ident, const bool &default_val, bool use_env) const
    {
        try {
            return this->get_numeric_property<bool>(ident, use_env);
        } catch (const DAF::IllegalArgumentException &) {
        }

        return default_val;
    }



    /**
    * \class PropertySingleton
    * \brief Singleton PropertyManager
    *
    * This class provides singleton access to a property map. The TAFServer
    * contains a process level (singleton) property map that can be accessed
    * via the DAF::get_property() methods. Direct access can be made via:
    *
    *         ThePropertyRepository - macro
    *         ThePropertyRepository->get_property("global_key");
    */
    class DAF_Export PropertySingleton : public DAF::PropertyManager
    {
    public:

        /**
        Constructor for Property Singleton.
        \param preload Load the Environment Variable parameter in "DAFProperties"

        For example:

                Environment Variable :
                DAFProperties = host.conf:sectionA,sectionB

        Will load the host.conf file with sections "sectionA" and "sectionB"
        */
        PropertySingleton(bool preload = true);

        /// ACE DLL Singleton Management
        const ACE_TCHAR * dll_name(void) const
        {
            return DAF_DLL_NAME;
        }

        /// ACE DLL Singleton Object Manager Key
        const ACE_TCHAR * name(void) const
        {
            return typeid(*this).name();
        }

    private:

        virtual const char * config_switch(void) const
        {
            return DAF_PROPERTIES;
        }

    };

    DAF_Export DAF::PropertySingleton * PropertyRepository(void);

} //nameservice DAF

#if !defined(ThePropertyRepository)
# define ThePropertyRepository (DAF::PropertyRepository)
#endif

namespace DAF
{
    template <typename T> inline T
    get_numeric_property(const PropertyManager::property_key_type &ident, bool use_env = true)
    {
        return ThePropertyRepository()->get_numeric_property<T>(ident, use_env);
    }

    template <typename T> inline T
    get_numeric_property(const PropertyManager::property_key_type &ident, const T &default_val, bool use_env = true)
    {
        return ThePropertyRepository()->get_numeric_property<T>(ident, default_val, use_env);
    }

    inline std::string
    get_property(const PropertyManager::property_key_type &ident, bool use_env = true)
    {
        return ThePropertyRepository()->get_property(ident, use_env);
    }

    inline std::string
    get_property(const PropertyManager::property_key_type &ident, const PropertyManager::property_val_type &default_val, bool use_env = true)
    {
        return ThePropertyRepository()->get_property(ident, default_val, use_env);
    }

    inline int
    set_property(const PropertyManager::property_key_type &ident, const PropertyManager::property_val_type &val)
    {
        return ThePropertyRepository()->set_property(ident, val);
    }


    inline int
    print_properties(void)
    {
        return ThePropertyRepository()->print_properties();
    }
}

#endif
