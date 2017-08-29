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
#ifndef TAF_IORRESOLVER_T_CPP
#define TAF_IORRESOLVER_T_CPP


#include "IORResolver_T.h"
#include <taf/ORBManager.h>


namespace TAF
{
    template <typename T>
    IORResolver_T<T>::IORResolver_T(const std::string &name, DAF::Monitor &mon)
        : name_(name)
        , result_(T::_nil())
        , resultMonitor_(mon)
    {
    }


    template <typename T>
    IORResolver_T<T>::~IORResolver_T(void)
    {
    }


    template <typename T>
    typename T::_var_type
    IORResolver_T<T>::getResult(void) const
    {
        return T::_duplicate(this->result_.in());
    }


    template <typename T>
    int
    IORResolver_T<T>::run(void)
    {
        return 0;
    }


    template <typename T>
    IORResolverChain_T<T>::IORResolverChain_T(void)
        : executor_()
        , resultMonitor_()
    {
    }


    template <typename T>
    IORResolverChain_T<T>::~IORResolverChain_T(void)
    {
        resultMonitor_.interrupt();
        this->clear();
    }


    template <typename T>
    int
    IORResolverChain_T<T>::addResolver(const _resolver_ref_type &resolver)
    {
        if (resolver)
        {
            this->push_back(resolver);
        }
        return 0;
    }


    template <typename T>
    DAF::Monitor &
    IORResolverChain_T<T>::getMonitor(void) const
    {
        return this->resultMonitor_;
    }


    template <typename T>
    typename T::_var_type
    IORResolverChain_T<T>::resolve(time_t timeout)
    {
        typename T::_var_type result;

        for (typename IORResolverChain_T<T>::iterator resolver(this->begin()); resolver != this->end(); resolver++)
        {
            if (*resolver)
            {
                this->executor_.execute(*resolver);
            }
        }

        try
        {
            while (CORBA::is_nil(result.in()))
            {
                ACE_GUARD_REACTION(DAF_SYNCH_MUTEX, resultGuard, this->resultMonitor_, DAF_THROW_EXCEPTION(DAF::InternalException));
                if (this->resultMonitor_.wait(timeout))
                {
                    break;
                }

                for (typename IORResolverChain_T<T>::iterator resolver(this->begin()); resolver != this->end(); resolver++)
                {
                    if (*resolver)
                    {
                        result = (*resolver)->getResult();
                        if (CORBA::is_nil(result.in()) ? false : true)
                        {
                            return result._retn();
                        }
                    }
                }
            }
        }
        catch (const DAF::InterruptedException &ie)
        {
            ACE_UNUSED_ARG(ie);
            ACE_DEBUG((LM_INFO, ACE_TEXT("TAF (%P|%t) IORResolverChain_T::resolve - Interrupted\n")));
        }
        DAF_CATCH_ALL
        {
        }

        return T::_nil();
    }


    template <typename T>
    FileResolver_T<T>::FileResolver_T(const std::string &name, DAF::Monitor &mon, const std::string &directory)
        : TAF::IORResolver_T<T>(name, mon)
        , filename_(name)
    {
        std::string bindDirectory(DAF::format_args(directory, true, false));

        if (bindDirectory.length())
        {
            // Remove any trailing delimiter
            for (int pos = int(bindDirectory.length()); pos--;)
            {
                if (bindDirectory[pos] == '\\' || bindDirectory[pos] == '/')
                {
                    bindDirectory.erase(pos, 1);
                }
                break;
            }
        }

        // Add our own delimiter '/' to the end
        this->directory_.assign(bindDirectory).append(1, '/');

        // Ensure the name supplied will represent a file
        if (int(this->filename_.find_first_of('.')) <= 0)
        {
            this->filename_.append(IOR_FILENAME_EXTENSION);
        }
    }


    template <typename T>
    FileResolver_T<T>::~FileResolver_T(void)
    {
    }


    template <typename T>
    int
    FileResolver_T<T>::run(void)
    {
        CORBA::Object_var obj;

        std::string filename(DAF::trim_string(this->directory_ + this->filename_));
        if (filename.length())
        {
            for (std::ifstream iorFile(filename_.c_str(), std::ios::in); iorFile;)
            {
                std::string iorFileString((std::istreambuf_iterator<char>(iorFile)), (std::istreambuf_iterator<char>()));
                if (iorFileString.length())
                {
                    try
                    {
                        obj = TAFStringToObject(iorFileString);
                        this->result_ = T::_narrow(obj.in());
                        ACE_GUARD_REACTION(DAF_SYNCH_MUTEX, resultGuard, this->resultMonitor_, DAF_THROW_EXCEPTION(DAF::InternalException));
                        this->resultMonitor_.signal();
                        if (TAF::debug())
                        {
                            ACE_DEBUG((LM_INFO, ACE_TEXT("TAF (%P|%t) FileResolver_T::run - Resolved %C from %C\n"), this->name_.c_str(), filename.c_str()));
                        }
                        return 0;
                    }
                    catch (const CORBA::Exception &ce)
                    {
                        ACE_ERROR((LM_ERROR, ACE_TEXT("TAF (%P|%t) FileResolver_T::run - Caught %C trying to resolve %C from %C\n"), ce._info().c_str(), this->name_.c_str(), filename.c_str()));
                    }
                }
            }
        }

        if (TAF::debug())
        {
            ACE_DEBUG((LM_INFO, ACE_TEXT("TAF (%P|%t) FileResolver_T::run - Failed to resolve %C from %C\n"), this->name_.c_str(), filename.c_str()));
        }
        return -1;
    }


    template <typename T>
    InitialRefResolver_T<T>::InitialRefResolver_T(const std::string &name, DAF::Monitor &mon)
        : TAF::IORResolver_T<T>(name, mon)
    {
    }


    template <typename T>
    InitialRefResolver_T<T>::~InitialRefResolver_T(void)
    {
    }


    template <typename T>
    int
    InitialRefResolver_T<T>::run(void)
    {
        CORBA::Object_var obj;

        try
        {
            obj = TAFResolveInitialReferences(this->name_.c_str());
            this->result_ = T::_narrow(obj.in());
            if (!CORBA::is_nil(this->result_.in()))
            {
                ACE_GUARD_REACTION(DAF_SYNCH_MUTEX, resultGuard, this->resultMonitor_, DAF_THROW_EXCEPTION(DAF::InternalException));
                this->resultMonitor_.signal();
                if (TAF::debug())
                {
                    ACE_DEBUG((LM_INFO, ACE_TEXT("TAF (%P|%t) InitialRefResolver_T::run - Resolved %C from initial references\n"), this->name_.c_str()));
                }
                return 0;
            }
        }
        catch (const CORBA::Exception &ce)
        {
            ACE_ERROR((LM_ERROR, ACE_TEXT("TAF (%P|%t) InitialRefResolver_T::run - Caught %C when resolving %C from initial references\n"), ce._info().c_str(), this->name_.c_str()));
        }

        if (TAF::debug())
        {
            ACE_DEBUG((LM_INFO, ACE_TEXT("TAF (%P|%t) InitialRefResolver_T::run - Failed to resolve %C from initial references\n"), this->name_.c_str()));
        }
        return -1;
    }


    template <typename T>
    NamingResolver_T<T>::NamingResolver_T(const std::string &name, DAF::Monitor &mon, const TAF::NamingContext &context)
        : TAF::IORResolver_T<T>(name, mon)
        , context_(context)
    {
    }


    template <typename T>
    NamingResolver_T<T>::~NamingResolver_T(void)
    {
    }


    template <typename T>
    int
    NamingResolver_T<T>::run(void)
    {
        CORBA::Object_var obj;

        try
        {
            obj = this->context_.resolve_name(this->name_.c_str());
            this->result_ = T::_narrow(obj.in());

            if (!CORBA::is_nil(this->result_.in()))
            {
                ACE_GUARD_REACTION(DAF_SYNCH_MUTEX, resultGuard, this->resultMonitor_, DAF_THROW_EXCEPTION(DAF::InternalException));
                this->resultMonitor_.signal();
                if (TAF::debug())
                {
                    ACE_DEBUG((LM_INFO, ACE_TEXT("TAF (%P|%t) NamingResolver_T::run - Resolved %C using the Naming Service\n"), this->name_.c_str()));
                }
                return 0;
            }
        }
        catch (const CosNaming::NamingContext::NotFound &nfe)
        {
            ACE_ERROR((LM_ERROR, ACE_TEXT("TAF (%P|%t) NamingResolver_T::run - Caught %C when attempting to resolve %C!\n"), nfe._info().c_str(), this->name_.c_str()));
        }
        catch (const CORBA::Exception &ce)
        {
            ACE_ERROR((LM_ERROR, ACE_TEXT("TAF (%P|%t) NamingResolver_T::run - Caught %C when attempting to resolve %C using the Naming Service\n"), ce._info().c_str(), this->name_.c_str()));
        }

        if (TAF::debug())
        {
            ACE_DEBUG((LM_INFO, ACE_TEXT("TAF (%P|%t) NamingResolver_T::run - Failed to resolve %C using the Naming Service\n"), this->name_.c_str()));
        }
        return -1;
    }

} // namespace TAF

#endif // TAF_IORRESOLVER_T_CPP
