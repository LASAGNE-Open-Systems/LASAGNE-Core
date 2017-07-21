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
#ifndef DAF_CONFIGURATOR_H
#define DAF_CONFIGURATOR_H

#include "DAF.h"

#include <list>
#include <map>

namespace DAF
{
    namespace {

        typedef std::map< std::string, std::string > property_map_type;
    }

    /** @class Configurator
     * @brief Brief Base Level configuration file parsing infrastructure
     *
     * Configurators are a map of strings that are read in from a configuration
     * file. They form the basis of Property and Service configuration files.
     * Files are broken into sections and key-value pairs. For example:
     *
     *     [sectionA]
     *     key1 = entryA1
     *     key2 = entryA2
     *
     *     [sectionB]
     *     key1 = entryB1
     *     key3 = entryB3
     *
     * Various implicit rules are associated with how Configurators are parsing
     * sections and the key-value pairs.
     */
    class DAF_Export Configurator : public property_map_type
    {
        mutable ACE_SYNCH_RW_MUTEX config_lock_;

    public:
        /**
          \name Metatypes
        */
        ///@{
        typedef std::string                     profile_type;

        typedef std::list< profile_type >       profile_list_type;


        typedef std::string                     section_type;

        typedef std::list< section_type >       section_list_type;


        typedef property_map_type::value_type   value_type;


        typedef property_map_type::key_type     property_key_type;

        typedef property_map_type::mapped_type  property_val_type;


        typedef std::list< value_type >         property_list_type;
        ///@}

        /** Load a file and its profile.
        The format for this argument is specific to how configuration files are
        input. It uses a "filename:<section>,<section>" format.
        */
        virtual int load_file_profile(const std::string &profile_arg);

        /** Load a configuration element via the CLI
        This method couples with the #config_switch method and will parse the
        CLI arguments for the parameter. It will then load the file and sections
        specified. For example:

            TAFServer -DAFProperties file1.conf:secA,secB -DAFProperties file2.conf

        Assuming a #config_switch parameter ('DAFProperties') this method will
        locate the -DAFProperities argument parse 'file1.conf:secA,secB' and also
        find the 'file2.conf' parameter and load the default section of that
        configurator.
        */
        virtual int load(int &argc, ACE_TCHAR *argv[], bool use_env = false);

        /** \return the number of loaded key-value pairs.*/
        size_t  load_count(void) const
        {
            return this->load_count_;
        }

    protected:


        Configurator(void);

        virtual ~Configurator(void);


        /**
        Insert or overwrite the key with new value.
        \note Write Lock must be already Held before Call to load_property()
        \param key the map key to use
        \param val the new value.
        */
        virtual int load_property(const property_key_type &key, const property_val_type &val);

        /**
        Configurator switch for CLI argument parse
        \sa #load()
        \note Must be overloaded to use load from arguments
        */
        virtual const char * config_switch(void) const;

    protected:

        /**
        \name Parse Helpers
        Internal helper to parse the "filename:<section>,<section>"
        into file and sections.
        */
        ///@{
        static size_t   make_config_profiles(const profile_type &profile_arg, profile_list_type &profiles);

        static size_t   make_config_sections(const section_type &section_arg, section_list_type &sections);
        ///@}

    protected:
        /** Parse Helpers
        File parser to aid in filling the key-value pair from a file and section.
         */
        virtual int load_file_sections(const std::string &filename, const section_list_type &sections);

        /** Mutex Accessor */
        operator ACE_SYNCH_RW_MUTEX & () const
        {
            return this->config_lock_;
        }

    private:

        size_t  load_count_;

    private:

        // = Prevent assignment and initialization.
        ACE_UNIMPLEMENTED_FUNC(void operator = (const Configurator &))
        ACE_UNIMPLEMENTED_FUNC(Configurator(const Configurator &))
    };

} // namespace DAF

#endif /* DAF_CONFIGURATOR_H */
