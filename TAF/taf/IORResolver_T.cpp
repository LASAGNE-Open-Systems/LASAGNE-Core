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


#define IOR_FILENAME_EXTENSION ACE_TEXT(".ior");


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
    IORResolver_T<T>::getResult(void)
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
    IORResolverChain_T<T>::getMonitor(void)
    {
        return this->resultMonitor_;
    }


    template <typename T>
    typename T::_var_type
        IORResolverChain_T<T>::resolve(time_t timeout)
    {
        typename T::_var_type result(T::_nil);

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
                ACE_GUARD(DAF_SYNCH_MUTEX, resultGuard, this->resultMonitor_);
                this->resultMonitor_.wait(timeout);

                for (typename IORResolverChain_T<T>::iterator resolver(this->begin()); resolver != this->end(); resolver++)
                {
                    if (*resolver)
                    {
                        result = (*resolver)->getResult();
                        if (!CORBA::is_nil(result.in()))
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


} // namespace TAF

#endif // TAF_IORRESOLVER_T_CPP
