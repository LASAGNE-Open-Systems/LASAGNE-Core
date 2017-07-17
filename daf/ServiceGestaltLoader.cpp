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
#define DAF_SERVICEGESTALTLOADER_CPP

#include "ServiceGestaltLoader.h"
#include "PropertyManager.h"

namespace DAF
{
    ServiceGestaltLoader::ServiceGestaltLoader(ServiceGestalt & gestalt) :Configurator()
        , gestalt_(gestalt)
    {
    }

    int
    ServiceGestaltLoader::process_directives(void)
    {
        ACE_WRITE_GUARD_REACTION(ACE_SYNCH_RW_MUTEX, mon, *this, { DAF_OS::last_error(ENOLCK); return -1; });

        int failed_loads = 0; // Say None have failed yet

        while (this->ident_list_.size()) {

            iterator it(this->find(this->ident_list_.front()));

            if (it != this->end()) {

                try {  // Calling user code (init)
                    if (this->process_directive(*it)) {
                        throw "process-directive-error"; // throw to catch on failure
                    }
                } DAF_CATCH_ALL {
                    failed_loads++;
                    if (DAF::debug()) {
                        ACE_DEBUG((LM_ERROR, ACE_TEXT("DAF::ServiceLoader (%P | %t) ERROR; ")
                            ACE_TEXT("Failure[%d] on load directive - ident='%s'\n")
                            , failed_loads, it->first.c_str()));
                    }
                }

                this->erase(it); // remove configuration entry from underlying map
            }

            this->ident_list_.pop_front();
        }

        return failed_loads;
    }

    int
    ServiceGestaltLoader::load_config_args(int & argc, ACE_TCHAR * argv[], bool use_property)
    {
        for (const char * cfg_switch = this->config_switch(); cfg_switch;) {

            const std::string cfg_property(DAF::trim_string(cfg_switch));

            if (cfg_property.length() == 0) {
                break;
            }

            do try {

                if (this->load(argc, argv, false)) {
                    break;
                }
                else if (this->load_count() ? false : use_property) {
                    if (this->load_file_profile(DAF::get_property(cfg_property, false))) {
                        break;
                    }
                }

                return int(this->load_count());

            } catch (const NotFoundException &) {
                // Could not load TAFServices property.
            } DAF_CATCH_ALL {
            } while(false);

            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("DAF (%P | %t) WARNING ServiceLoader: ")
                ACE_TEXT("Unable to locate and load %s options\n")
                , cfg_property.c_str()), int(this->load_count()));
        }

        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("DAF (%P | %t) ERROR ServiceLoader: ")
            ACE_TEXT(" Unable to process switched (config_switch) configuration.\n"))
            , int(this->load_count()));
    }

    int
    ServiceGestaltLoader::process_directive(const value_type & val)
    {
        const property_key_type & ident(val.first);
        const property_val_type & args(DAF::format_args(val.second, true));

        std::string s_class, s_params, s_libpathname; // Parameters to service Load

        {  // Handle Library with (optional) full path

            int lib_pos = int(args.find_first_of(':'));

            switch (lib_pos) {

#if defined(ACE_WIN32)
                /* Handle windows absolute path nomenclature (ie "D:\" which may also be deliminated with quotation marks */
            case 2: if (args[0] == '\"' || args[0] == '\'') {
            case 1:     if (args.length() > size_t(lib_pos + 1) && ::isalpha(args[lib_pos - 1])) {
                if (args[lib_pos + 1] == '\\' || args[lib_pos + 1] == '/') {
                    lib_pos = int(args.find_first_of(':', lib_pos + 1));
                }
            }
            }
#endif
            default:

                if (lib_pos > 0) {
                    s_libpathname.assign(DAF::trim_string(args.substr(0, lib_pos++)));
                    if (s_libpathname.length()) {
                        s_class.assign(DAF::trim_string(args.substr(lib_pos))); break; // Factory method is Next
                    }
                }

                if (DAF::debug()) {
                    ACE_DEBUG((LM_WARNING, ACE_TEXT("DAF (%P | %t) WARNING ServiceLoader: ")
                        ACE_TEXT("Unable to locate Library path name for service with ident='%s' in %s.\n")
                        , ident.c_str(), args.c_str()));
                }

                return -1;
            }
        }

        int c_pos = int(s_class.find_first_of(' '));

        if (c_pos > 0) { // Split at first space after object class name
            int p_pos = int(s_class.find_first_of(')'));
            if (p_pos > 0) {
                s_params = DAF::trim_string(s_class.substr(p_pos + 1));
            }
            else {
                s_params = DAF::trim_string(s_class.substr(c_pos + 1));
            }

            s_class.erase(c_pos);
        }

        if ((c_pos = int(s_class.find_first_of('('))) > 0) {
            s_class.erase(c_pos);
        }

        // clean up factory method
        const std::string s_objectclass(DAF::trim_string(s_class));

        if (s_objectclass.length() == 0) {
            if (DAF::debug()) {
                ACE_DEBUG((LM_ERROR, ACE_TEXT("DAF (%P | %t) WARNING ServiceLoader: ")
                    ACE_TEXT("Unable to locate Object class name for service with ident='%s' in %s.\n")
                    , ident.c_str(), args.c_str()));
            }
            return -1;
        }

        try {

            return this->load_service(ident, s_libpathname, s_objectclass, s_params);

        } DAF_CATCH_ALL { /* Translate any exceptions into Unable-to-Load */ }

        ACE_ERROR_RETURN((LM_WARNING, ACE_TEXT("DAF (%P | %t) WARNING ServiceLoader: ")
            ACE_TEXT("Failure to load service '%s' %s\n")
            , ident.c_str(), DAF::last_error_text().c_str()), -1);
    }

    int
    ServiceGestaltLoader::load_service(const std::string & ident, const std::string & libpathname, const std::string & objectclass, const std::string & params)
    {
        return this->gestalt_.loadDynamic(ident, libpathname, objectclass, params);
    }

    int
    ServiceGestaltLoader::load_property(const property_key_type & key, const property_val_type & val)
    {
        this->ident_list_.push_back(key); return Configurator::load_property(key, val);
    }

} // namespace DAF
