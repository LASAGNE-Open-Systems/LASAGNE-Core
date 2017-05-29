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
#define TAF_PROPERTYLOADER_CPP

#include "PropertyLoader.h"

#include <daf/PropertyManager.h>

#include <ace/Arg_Shifter.h>

namespace TAF
{
    namespace {
        const ACE_TCHAR * property_options[] = { // Only valid command line argument overrides
            TAF_RESOLVETIMEOUT,
            TAF_DISCOVERYENABLE,
            TAF_DISCOVERYENDPOINT,
#if defined(TAF_HAS_SECURITY)
            TAF_SECURITYDISABLE,
            TAF_DEFAULTALLOWANCE,
            TAF_COLLOCATEDALLOWANCE,
# if defined(TAF_HAS_SSLIOP)
            TAF_SSLNOPROTECTION,
# endif
#endif
            TAF_ORBTHREADS,
            TAF_BASECONTEXT,
            TAF_SERVERNAME,
            ACE_BASETHREADS,
            ACE_DEBUGGING,
            TAO_DEBUGGING,
            DAF_DEBUGGING,
            TAF_DEBUGGING,
            TAO_ORBDEBUGGING,
            0  // Signal end of options list
        };
    }

    PropertyLoader::PropertyLoader(int &argc, ACE_TCHAR *argv[], bool use_env) : DAF_ARGV(false)
    {
        if (argc) for (ACE_Arg_Shifter arg_shifter(argc, argv); arg_shifter.is_anything_left();) {

            if (arg_shifter.is_option_next()) {
                if (arg_shifter.cur_arg_strncasecmp(TAF_PROPERTIES_FLAG) == 0) {
                    for (arg_shifter.consume_arg(); arg_shifter.is_parameter_next(); arg_shifter.consume_arg()) {
                        try {
                            if (this->load_file_profile(arg_shifter.get_current()) == 0) {
                                continue;
                            }
                        } DAF_CATCH_ALL { /* Drop Through to WARNING */ }

                        ACE_DEBUG((LM_WARNING, ACE_TEXT("TAF (%P | %t) PropertyLoader WARNING: ")
                            ACE_TEXT("Unable to load properties from file argument '%s' - Removed.\n")
                            , arg_shifter.get_current()));
                    }
                    continue;
                }
            }
            arg_shifter.ignore_arg();
        }

        if (this->load_count() ? false : use_env) {

            for (const char *properties = DAF_OS::getenv(this->config_switch()); properties;) {
                try {
                    if (this->load_file_profile(properties) == 0) {
                        break;
                    }
                } DAF_CATCH_ALL{
                    /* Ignore */
                }

                ACE_ERROR_BREAK((LM_WARNING, ACE_TEXT("TAF (%P | %t) PropertyLoader WARNING: ")
                    ACE_TEXT("Unable to load properties from environment %s, with value '%s' - Ignored.\n")
                    , this->config_switch(), properties));
            }
        }
    }

    int
    PropertyLoader::parse_args(int &argc, ACE_TCHAR **&argv)
    {
        ACE_Arg_Shifter arg_shifter(argc, argv); // Use temporary argc

        try { // Try and establish new argument count

            const std::string cmdLineEx(DAF::get_property(TAF_EXTENSIONARGS, false));

            ACE_ARGV cmdLine_argv(cmdLineEx.c_str(), false);

            if (cmdLine_argv.argc()) {
                for (; arg_shifter.is_parameter_next(); arg_shifter.ignore_arg()) {
                    this->add(arg_shifter.get_current(), true);
                }

                for (int i = 0; i < cmdLine_argv.argc(); i++) {
                    this->add(cmdLine_argv[i], true);
                }
            }

        } catch (const DAF::NotFoundException &) {
        }

        for (; arg_shifter.is_anything_left(); arg_shifter.ignore_arg()) {
            this->add(arg_shifter.get_current(), true);
        }

        argc = this->argc(); argv = this->argv(); return 0;
    }

    int
    PropertyLoader::load_properties(int &argc, ACE_TCHAR **&argv)
    {
        if (this->parse_args(argc, argv)) {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("TAF (%P | %t) ERROR PropertyLoader: ")
                ACE_TEXT("Unable to process the argument list to the loader.\n")), -1);
        }

        for (ACE_Arg_Shifter arg_shifter(argc, argv); arg_shifter.is_anything_left();) {

            if (arg_shifter.is_parameter_next()) {
                arg_shifter.ignore_arg(); continue; // If Parameter Ignore
            }

            if (arg_shifter.cur_arg_strncasecmp(CORBA_ORB_ARGUMENT_SPECIFIER) >= 0) {
                arg_shifter.ignore_arg(); continue; // If ORB Argument Ignore
            }

            // If Option is a TAFProperties flag - Illegal Here (consume parameters)
            if (arg_shifter.cur_arg_strncasecmp(TAF_PROPERTIES_FLAG) == 0) {
                for (arg_shifter.consume_arg(); arg_shifter.is_parameter_next(); arg_shifter.consume_arg()) {
                    ACE_DEBUG((LM_WARNING, ACE_TEXT("TAF (%P | %t) PropertyLoader WARNING: ")
                        ACE_TEXT("Illegal %s Context from %s; Property='%s' - Ignored.\n")
                        , TAF_PROPERTIES_FLAG, TAF_EXTENSIONARGS, arg_shifter.get_current()));
                }
                continue;
            }

            bool arg_ignore = true; // Is The Option one in our valid cmd line list?

            for (int i = 0; property_options[i]; i++) { // Is Argument in list

                const std::string arg_option(property_options[i]);

                if (arg_shifter.cur_arg_strncasecmp(("-" + arg_option).c_str()) == 0) {

                    ACE_ARGV param_argv(false); arg_ignore = false;

                    std::string param("1"); // Set default parameter to "true"

                    // Convert CommandLine Option to a property
                    for (arg_shifter.consume_arg(); arg_shifter.is_parameter_next(); arg_shifter.consume_arg()) {
                        param_argv.add(arg_shifter.get_current(), true);
                    }

                    if (param_argv.argc()) {
                        param.assign(DAF::parse_argv(param_argv, true));
                    }

                    this->load_property(arg_option, param); break;
                }
            }

            if (arg_ignore) {
                arg_shifter.ignore_arg();
            }
        }

        return 0;
    }

    int
    PropertyLoader::load_property(const property_key_type &key, const property_val_type &val)
    {
        return ((DAF::Configurator::load_property(key, val) || DAF::set_property(key, val)) ? -1 : 0);
    }

} // namespace TAF
