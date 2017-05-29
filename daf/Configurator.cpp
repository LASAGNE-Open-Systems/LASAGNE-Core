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
#define DAF_CONFIGURATOR_CPP

#include "Configurator.h"

#include "PropertyManager.h"

#include <ace/ARGV.h>
#include <ace/Arg_Shifter.h>
#include <ace/Thread.h>

#include <fstream>
#include <functional>

namespace DAF
{
    namespace { // annanomous

        typedef std::map< Configurator::section_type, Configurator::property_list_type >  fileloader_map_type;

        struct ConfiguratorFileLoader : fileloader_map_type {
            ConfiguratorFileLoader(const std::string &filename);
        };

        ConfiguratorFileLoader::ConfiguratorFileLoader(const std::string &filename)
        {
            // NOTE: We can only use ACE::debug here as DAF::debug() may not have been set up yet.

            struct ConfigFile : std::ifstream {
                ConfigFile(const std::string &file_name) : std::ifstream(file_name.c_str()) {
                    if (this->is_open() ? this->fail() : true) {
                        DAF_THROW_EXCEPTION(DAF::NotFoundException);  // Could Not Open config File
                    }
                    else if (ACE::debug()) {
                        ACE_DEBUG((LM_DEBUG, ACE_TEXT("DAF::Configurator (%P | %t) OpenFile=%s\n"),
                            file_name.c_str()));
                    }
                }
                ~ConfigFile(void) { if (this->is_open()) this->close(); }
            } configFile(filename);

            iterator it(this->end());   // Set up the Section Map iterator to current end.

            std::string readLine;       // String to receive config file text line.

            for (size_t line_no = 1; std::getline(configFile, readLine); line_no++) try {

                const std::string cfgLine(DAF::trim_string(readLine.substr(0, readLine.find_first_of('#'))));

                if (cfgLine.length() == 0) {
                    continue;
                }

                // Find Possible Section Entry.

                if (cfgLine[0] == '[') {
                    int pos = int(cfgLine.find_first_of(']', 1));
                    if (pos-- > 0 && pos) { // Text for Section Name ?
                        const std::string section(DAF::trim_string(cfgLine.substr(1, pos)));
                        if (section.length()) { // Must have a valid section name
                            if ((it = this->find(section)) == this->end()) { // Do we already know about this section?
                                std::pair<iterator, bool> ib(this->insert(value_type(section, mapped_type()))); // Add Entry to DB.
                                if (ib.second) { // Added OK?
                                    it = ib.first; if (ACE::debug()) {
                                        ACE_DEBUG((LM_DEBUG, ACE_TEXT("Line[%03d]\tSection [%s] Added\n"), line_no, section.c_str()));
                                    }
                                }
                            } else if (ACE::debug()) {
                                ACE_DEBUG((LM_DEBUG, ACE_TEXT("Line[%03d]\tSection [%s]\n"), line_no, section.c_str()));
                            }
                            continue;
                        }
                    }

                    throw "Invalid-Section-Syntax"; // Section Entry is Invalid
                }

                // Look At Entry (Must Be contained within a Section);

                if (it == this->end()) {  // We Are Currently in a Section?
                    throw "Entry-Outside-Section"; // Entry outside a Section - Illegal
                }

                std::string cfgKey, cfgArg; // The resultant property fields

                // We will allow no arguments on the config entry from this parser's perspective

                int pos = int(cfgLine.find_first_of('='));

                if (pos > 0) {
                    cfgKey.assign(DAF::trim_string(cfgLine.substr(0, pos++)));
                    cfgArg.append(DAF::parse_args(cfgLine.substr(pos), false));
                }
                else if (pos) {
                    cfgKey.assign(cfgLine); // No '=' so use full line as entry key with no arg.
                }
                else {  // '=' at column 0 - Invalid
                    throw "Invalid-Property-Syntax";
                }

                if (cfgKey.length() ? int(cfgKey.find_first_of(' ')) > 0 : true) { // Check that we have a valid key
                    throw "Invalid-Key-Syntax";
                }

                if (ACE::debug()) { // Allow for debug output from parser
                    ACE_DEBUG((LM_DEBUG, ACE_TEXT("Line[%03d]\t%s=%s\n"), line_no, cfgKey.c_str(), cfgArg.c_str()));
                }

                it->second.push_back(mapped_type::value_type(cfgKey, cfgArg));
            }
            catch (const char *what_error) {
                ACE_DEBUG((LM_DEBUG,
                    ACE_TEXT("DAF::Configurator (%P | %t) ERROR:[file=%s,line=%d]\n\terror=\"%s\"\n")
                    , filename.c_str(), line_no, what_error)); throw DAF::InitializationException(what_error);
            }
        }

    } // ananomous

    /*********************************************************************************************************/

    Configurator::Configurator(void) : load_count_(0)
    {
    }

    Configurator::~Configurator(void)
    {
        this->clear();
    }

    int
    Configurator::load(int &argc, ACE_TCHAR *argv[], bool use_env)
    {
        if (this->config_switch() && DAF_OS::strlen(this->config_switch())) {

            const std::string cfg_switch(DAF::trim_string(this->config_switch()));
            const std::string arg_switch("-" + cfg_switch);

            bool load_property = true;

            if (argc) for (ACE_Arg_Shifter arg_shifter(argc, argv); arg_shifter.is_anything_left();) {
                if (arg_shifter.is_option_next()) {
                    if (0 == arg_shifter.cur_arg_strncasecmp(arg_switch.c_str())) {
                        for (arg_shifter.consume_arg(); arg_shifter.is_parameter_next(); arg_shifter.consume_arg()) {
                            if (this->load_file_profile(DAF::trim_string(arg_shifter.get_current())) == 0) {
                                load_property = false; // Dont use properties if we found from arguments with switch
                            }
                        }
                        continue;
                    }
                }
                arg_shifter.ignore_arg();
            }

            if (load_property) try {
                this->load_file_profile(ThePropertyRepository()->get_property(cfg_switch, use_env));
            } catch (const DAF::NotFoundException &) {
            }

            return 0;
        }

        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("DAF (%P | %t) ERROR: Configurator;")
            ACE_TEXT(" Unable to process configuration without a valid config_switch() overload.\n")),-1);
    }

    int
    Configurator::load_file_profile(const std::string &profile_arg)
    {
        profile_list_type profiles;

        if (make_config_profiles(profile_arg, profiles)) {

            for (profile_list_type::const_iterator it = profiles.begin(); it != profiles.end(); it++) {

                std::string filename(*it), section_args; // say no sections

                // Find sections deliminator
                for (int pos = 0; (pos = int(it->find_first_of(':', pos))) > 0; pos++) {
#if defined(ACE_WIN32)
                    if (int(it->length()) > ++pos) {
                        if (::isalpha(int((*it)[0]))) switch ((*it)[pos]) {   // look for windows 'D:\' or 'D:/' type string
                        case '/': case '\\': if (pos == 2) continue;    // Not section seperator
                        }
                    } pos -= 1; // Point back to ':'
#endif
                    filename.assign(DAF::trim_string(it->substr(0, pos++))); section_args.assign(it->substr(pos)); break;
                }

                if (int(filename.find_first_of(' ')) > 0) {
                    filename.insert(0, 1, '\'').append(1, '\''); // Deliminate
                }

                section_list_type sections;

                for (const std::string &host(DAF_OS::gethostname()); host.length();) { // add "host:HOSTNAME" section
                    sections.push_front(std::string("host:").append(host)); break;
                }

                sections.push_front(DAF_CONFIGURATOR_COMMON_SECTION_NAME); // Always include a "common" section first

                if (make_config_sections(section_args, sections)) try {
                    if (this->load_file_sections(filename, sections) == 0) { // Can throw (file) NotFoundException
                        this->load_count_++; continue;
                    }
                } catch (const DAF::NotFoundException &) {
                    ACE_DEBUG((LM_ERROR, ACE_TEXT("DAF (%P | %t) ERROR: Configurator; Unable to open file %s\n")
                        , filename.c_str())); continue; // Try Next profile
                } DAF_CATCH_ALL {
                }

                ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("DAF (%P | %t) ERROR: Configurator;")
                    ACE_TEXT(" Unable to process '%s' profile from file-profile argument:\n\t'%s'.\n")
                    , it->c_str(), profile_arg.c_str()), -1);
            }

            return 0;
        }

        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("DAF (%P | %t) ERROR: Configurator;")
            ACE_TEXT(" Unable to open and process configuration from file-profile argument:\n\t'%s'.\n")
            , profile_arg.c_str()), -1);
    }

    int
    Configurator::load_file_sections(const std::string &filename, const section_list_type &sections)
    {
        ConfiguratorFileLoader conf_loader(filename); // Load up the file entries -> can throw DAF::NotFoundException

        for (section_list_type::const_iterator it = sections.begin(); it != sections.end(); it++) {

            ConfiguratorFileLoader::iterator cit = conf_loader.find(*it); // Config iterator

            if (cit != conf_loader.end()) {  // May have already been loaded

                do {    // Scope Lock
                    ACE_WRITE_GUARD_REACTION(ACE_SYNCH_RW_MUTEX, mon, *this, break);
                    for (property_list_type::const_iterator pit = cit->second.begin(); pit != cit->second.end(); pit++) {  // Parameter iterator
                        if (this->load_property(pit->first, DAF::parse_args(pit->second, false)) == 0 && ACE::debug()) {
                            ACE_DEBUG((LM_INFO, ACE_TEXT("DAF::Configurator (%P | %t) INFO: ")
                                ACE_TEXT("Loaded property from file \"%s\";\n\t[key=%s;val=%s]\n")
                                , filename.c_str(), pit->first.c_str(), pit->second.c_str()));
                        }
                    }
                } while (false);

                conf_loader.erase(cit); // Remove config section entry so cannot be reused on subsequent passes
            }
        }

        return 0;
    }

    size_t
    Configurator::make_config_profiles(const profile_type &profile_arg, profile_list_type &profiles)
    {
        profiles.clear(); ACE_ARGV args(profile_arg.c_str(), true); // Convert Env variables

        for (int i = 0; i < args.argc(); i++) {
            int pos = 0; const std::string arg(args[i]);
            if (arg.length()) do {
                int e_pos = int(arg.find_first_of(';', pos));
                const std::string profile(DAF::trim_string(arg.substr(pos, e_pos - pos))); pos = e_pos;
                if (profile.length()) {
                    profiles.push_back(profile);
                }
            } while (pos++ > 0);
        }

        return profiles.size();
    }

    size_t  // Process the ',' separated sections into a section list.
    Configurator::make_config_sections(const section_type &section_arg, section_list_type &sections)
    {
        ACE_ARGV args(section_arg.c_str(), true);

        for (int i = 0; i < args.argc(); i++) {
            int pos = 0; const std::string arg(args[i]);
            if (arg.length()) do {
                int e_pos = int(arg.find_first_of(',', pos));
                const std::string section(DAF::trim_string(arg.substr(pos, e_pos - pos))); pos = e_pos;
                if (section.length()) {
                    sections.push_back(section);
                }
            } while (pos++ > 0);
        }

        return sections.size();
    }

    int
    Configurator::load_property(const property_key_type &key, const property_val_type &val)
    {
        if (val.length() || this->find(key) == this->end()) { // Locks already held
            (*this)[key] = val; return 0;
        }
        return -1;
    }

    const char *
    Configurator::config_switch(void) const
    {
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("DAF::Configurator (%P | %t) ERROR:")
            ACE_TEXT(" Must overload config_switch() to use with Configurator::load(argc,argv)./n")),0);
    }

} // namespace DAF
