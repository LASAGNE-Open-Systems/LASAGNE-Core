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
#ifndef TAF_IORRESOLVER_T_H
#define TAF_IORRESOLVER_T_H


#include <daf/Runnable.h>
#include <list>


namespace TAF
{
    template <typename T>
    class IORResolver_T : public virtual DAF::Runnable
    {
    public:
        DAF_DEFINE_REFCOUNTABLE(IORResolver_T<T>);

        typedef std::list<typename IORResolver_T<T>::_ref_type> _list_type;

        IORResolver_T(const std::string &name, DAF::Monitor &mon);
        virtual ~IORResolver_T(void);

        typename T::_var_type   getResult(void);

    protected:
        // Methods from DAF::Runnable
        virtual int run(void);

    protected:
        std::string             name_;
        typename T::_var_type   result_;
        DAF::Monitor            &resultMonitor_;
    };


    template <typename T>
    class IORResolverChain_T : public IORResolver_T<T>::_list_type
    {
    public:
        typedef IORResolver_T<T>                        _resolver_type;
        typedef typename IORResolver_T<T>::_ref_type    _resolver_ref_type;

        enum {
            DEFAULT_RESOLVE_TIMEOUT = time_t(2000) // milliseconds
        };

        IORResolverChain_T(void);
        virtual ~IORResolverChain_T(void);

        int addResolver(const _resolver_ref_type &resolver);
        DAF::Monitor & getMonitor(void);
        typename T::_var_type resolve(time_t timeout = DEFAULT_RESOLVE_TIMEOUT);

    private:
        DAF::TaskExecutor       executor_;
        DAF::Monitor            resultMonitor_;
    };


    template <typename T>
    class FileResolver_T : public IORResolver_T<T>
    {
    public:
        const std::string IOR_FILENAME_EXTENSION = ".ior";

        DAF_DEFINE_REFCOUNTABLE(FileResolver_T<T>);

        FileResolver_T(const std::string &name, DAF::Monitor &mon, const std::string &directory = "");
        virtual ~FileResolver_T(void);

    protected:
        // Methods from DAF::Runnable
        virtual int run(void);

    private:
        std::string directory_;
        std::string filename_;
    };


    template <typename T>
    class InitialRefResolver_T : public IORResolver_T<T>
    {
    public:
        DAF_DEFINE_REFCOUNTABLE(InitialRefResolver_T<T>);

        InitialRefResolver_T(const std::string &name, DAF::Monitor &mon);
        virtual ~InitialRefResolver_T(void);

    protected:
        // Methods from DAF::Runnable
        virtual int run(void);
    };


    template <typename T>
    class NamingResolver_T : public IORResolver_T<T>
    {
    public:
        DAF_DEFINE_REFCOUNTABLE(NamingResolver_T<T>);

        NamingResolver_T(const std::string &name, DAF::Monitor &mon, const TAF::NamingContext &context = TheTAFBaseContext());
        virtual ~NamingResolver_T(void);

    protected:
        // Methods from DAF::Runnable
        virtual int run(void);

    private:
        TAF::NamingContext context_;
    };

} // namespace TAF

#if defined (ACE_TEMPLATES_REQUIRE_SOURCE)
# include "IORResolver_T.cpp"
#endif /* ACE_TEMPLATES_REQUIRE_SOURCE */

#if defined (ACE_TEMPLATES_REQUIRE_PRAGMA)
# pragma implementation ("IORResolver_T.cpp")
#endif /* ACE_TEMPLATES_REQUIRE_PRAGMA */

#endif // TAF_IORRESOLVER_T_H
