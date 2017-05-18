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
#define TAF_IORBINDER_CPP

#include "IORBinder.h"

#include "ORBManager.h"
#include "IORQueryRepository.h"

#include <fstream>

#define IOR_FILENAME_EXTENSION  ACE_TEXT(".ior")

PortableServer::POA_ptr TAFRootPOA::_default_POA(void)
{
    return PortableServer::POA::_duplicate(TheTAFRootPOA());
}

PortableServer::POA_ptr TAFDefaultPOA::_default_POA(void)
{
    return PortableServer::POA::_duplicate(TheTAFDefaultPOA());
}

namespace {
    const struct DefaultIORBinder : TAF::IORBinderSequence {
        DefaultIORBinder(void) : TAF::IORBinderSequence()
        {
            this->add_binding(new TAF::RegisterIORTableBinder());
            this->add_binding(new TAF::RegisterInitReferenceBinder());
            this->add_binding(new TAF::RegisterIORQueryBinder());
        }
    } DEFAULT_IOR_BINDER;
}

TAF_Export const TAF::IORBinderSequence & TAF_DEFAULT_IOR_BINDER(DEFAULT_IOR_BINDER);

namespace TAF
{
    IORBinderSequence &
    IORBinderSequence::add_binding(const IORBinder_ref &ref)
    {
        if (ref) { this->push_back(ref); } return *this;
    }

    int
    IORBinderSequence::init_bind(const std::string &name, CORBA::Object_ptr obj)
    {
        int errors = 0, index = 0;

        this->bind_name_.assign(DAF::trim_string(name));

        for (iterator it(this->begin()); it != this->end(); index++)  {

            if (*it) { // If a valid reference

                try {

                    if ((*it)->init_bind((*it)->make_bind_name(this->bind_name()), obj) == 0) {
                        it++; continue;
                    }

                } catch (const CORBA::Exception &ex) {
                    ex._tao_print_exception("InitBind-Failed");
                } DAF_CATCH_ALL {
                }

                errors++;

                ACE_DEBUG((LM_WARNING, ACE_TEXT("TAF::IORBinder (%P | %t) ")
                    ACE_TEXT("WARNING init_bind[%d] failure(#%d) in %s (%s) - Removed.\n")
                    , index, errors, typeid(*it).name(), this->bind_name().c_str()));
            }

            it = this->erase(it); // Remove from list to stop fini call()
        }

        return errors;
    }

    int
    IORBinderSequence::fini_bind(void)
    {
        int errors = 0, index(int(this->size()));

        for (const_reverse_iterator it(this->rbegin()); it != this->rend(); it++) { // Make sure we go backwards

            index--;

            try {

                if ((*it)->fini_bind((*it)->make_bind_name(this->bind_name())) == 0) {
                    continue;
                }

            }
            catch (const CORBA::Exception &ex) {
                ex._tao_print_exception("FiniBind-Failed");
            } DAF_CATCH_ALL{
            }

            errors++;

            ACE_DEBUG((LM_WARNING, ACE_TEXT("TAF::IORBinder (%P | %t) ")
                ACE_TEXT("WARNING fini_bind[%d] failure(#%d) in %s (%s).\n")
                , index, errors, typeid(*it).name(), this->bind_name().c_str()));
        }

        return errors;
    }


    /***********************************************************************************/

    int
    RegisterIORTableBinder::init_bind(const std::string &name, CORBA::Object_ptr p)
    {
        TheTAFIORTable()->rebind(name.c_str(), TAFObjectToString(p).in()); return 0;
    }

    int
    RegisterIORTableBinder::fini_bind(const std::string &name)
    {
        TheTAFIORTable()->unbind(name.c_str()); return 0;
    }

    /***********************************************************************************/

    int
    RegisterInitReferenceBinder::init_bind(const std::string &name, CORBA::Object_ptr p)
    {
        TAFRegisterInitialReference(name, p); return 0;
    }

    int
    RegisterInitReferenceBinder::fini_bind(const std::string &name)
    {
        TAFUnRegisterInitialReference(name); return 0;
    }

    /***********************************************************************************/

    int
    RegisterIORQueryBinder::init_bind(const std::string &name, CORBA::Object_ptr p)
    {
        TheIORQueryRepository()->registerQueryService(p, name); return 0;
    }

    int
    RegisterIORQueryBinder::fini_bind(const std::string &name)
    {
        TheIORQueryRepository()->unregisterQueryService(name); return 0;
    }

    /***********************************************************************************/

    NamingServiceBinder::NamingServiceBinder(const TAF::NamingContext &context)
        : context_(context)
    {
    }

    int
    NamingServiceBinder::init_bind(const std::string &name, CORBA::Object_ptr p)
    {
        return (p ? (this->context_.bind_name(name, p), 0) : -1);
    }

    int
    NamingServiceBinder::fini_bind(const std::string &name)
    {
        this->context_.unbind_name(name); return 0;
    }

    /***********************************************************************************/

    IORFileBinder::IORFileBinder(const std::string &directory)
    {
        std::string bind_dir(DAF::format_args(directory, true, false));

        if (bind_dir.length()) {

            for (int pos = int(bind_dir.length()); pos--;) { // remove trailing deliminator
                if (bind_dir[pos] == '\\' || bind_dir[pos] == '/') {
                    bind_dir.erase(pos, 1);
                }
                break;
            }

            this->directory_.assign(bind_dir).append(1, '/');
        }
    }

    std::string
    IORFileBinder::make_bind_name(const std::string &name) const
    {
        if (int(name.find_first_of('.')) > 0) {
            return name;
        }
        return name +  IOR_FILENAME_EXTENSION;
    }

    int
    IORFileBinder::init_bind(const std::string &name, CORBA::Object_ptr p)
    {
        if (p && name.length()) try {

            std::string filename(DAF::trim_string(this->directory_ + name));

            if (filename.length()) {

                if (int(filename.find_first_of(' ')) > 0) {
                    filename.insert(0,1,'\'').append(1,'\'');
                }

                for (std::ofstream ior_file(filename.c_str(), std::ios::out); ior_file;) {
                    this->filename_.assign(filename); // Save for removal in fini_bind
                    ior_file << TAFObjectToString(p).in();
                    return 0;
                }
            }
        } DAF_CATCH_ALL { /* Ignore Error */ }

        return -1;
    }

    int
    IORFileBinder::fini_bind(const std::string &name)
    {
        ACE_UNUSED_ARG(name); return this->filename_.length() ? std::remove(this->filename_.c_str()) : 0;
    }

    /***********************************************************************************/
}
