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
#ifndef DAF_SERVICELOADER_H
#define DAF_SERVICELOADER_H

#include "Configurator.h"

#include <list>

namespace DAF
{
    /**
    * \class ServiceLoader
    * \brief Pure Abstract Configurator for parsing service config files.
    *
    * This class is responsible for the parsing and command generation for
    * loading services into a DAF::ServiceGestalt. It leaves the config_swtich
    * undefined for the upper implementation in TAF::GestaltServiceLoader.
    *
    * It is responsible for parsing the value string of a service definition.
    * For example :
    *
    *         ServiceKey = <library> : <_make_factory_hook> <arg1> ....<argN>
    *
    * Should be parsed and broken down into :
    * - key         = ServiceKey
    * - libpathname = <library>
    * - objectclass  = <_make_factory_hook>
    * - params      = <arg1> .... <argN>
    *
    * When processing a set of configuration sections using the process_directives
    * call it will then call this->load_service(..) to be implemented by the
    * loading logic (not implemented here).
    */
    class DAF_Export ServiceLoader : public DAF::Configurator
    {
        typedef std::list< property_key_type >  ident_list_type;

    public:


        ServiceLoader(void);

        virtual int load_config_args(int &argc, ACE_TCHAR *argv[], bool use_poperty = true);

        /**
        This method is used to start the parsing and loading process. It will
        kickoff the parsing of the value string into library, objectclass, params.
        and then up-call to #load_service().
        \return number of failed service loads
         */
        virtual int process_directives(void);  // Takes The Lock

    protected:

        /**
        Abstract load_service method for implementation downstream.
        Inheriting classes will implement this method according to their target.
        In most cases this is a GestaltServiceLoader. But can be used for any
        type of "processing" necessary.
        \return 0 no problem loading, non-zero for failed load
        */
        virtual int load_service(   const std::string &ident,
                                    const std::string &libpathname,
                                    const std::string &objectclass,
                                    const std::string &params);

        /**
        Performs the parsing of the service value string. Calls load_service
        with successfully parsed values.
        \return 0 for success parsing and loading, non-zero for failure to parse
        or load
        */
        virtual int process_directive(const value_type &val);

        /**
        This overloaded method will internally store service idents for subsequent
        processing.
        \sa DAF::Configurator::load_property()
        */
        virtual int load_property(const property_key_type &key, const property_val_type &val); // Locks already held

    private:

        /**
        Pure Abstract from DAF::Configurator.
        \note Must be overloaded
        */
        virtual const char * config_switch(void) const = 0;

        ident_list_type ident_list_;
    };
}   // namespace DAF

#endif // DAF_SERVICELOADER_H
