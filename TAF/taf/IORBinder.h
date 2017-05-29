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
#ifndef TAF_IORBINDER_H
#define TAF_IORBINDER_H

#include "TAF.h"

#include "ORBManager.h"
#include "NamingContext.h"

#include <daf/RefCount.h>

#include <tao/Object.h>
#include <tao/PortableServer/PortableServer.h>

#include <list>

typedef struct TAF_Export TAFBasePOA {
    virtual ~TAFBasePOA(void) {}
} TAF_BASE_POA;

typedef struct TAF_Export TAFRootPOA : TAFBasePOA {
    virtual PortableServer::POA_ptr _default_POA(void);
} TAF_ROOT_POA;

typedef struct TAF_Export TAFDefaultPOA : TAFBasePOA {
    virtual PortableServer::POA_ptr _default_POA(void);
} TAF_DEFAULT_POA;

namespace TAF
{
    struct TAF_Export IORBinder : DAF::RefCount
    {
        DAF_DEFINE_REFCOUNTABLE(IORBinder);

        typedef class std::list < IORBinder::_ref_type > _list_type;

        virtual int init_bind(const std::string &name, CORBA::Object_ptr)
        {
            ACE_UNUSED_ARG(name); return 0;
        }

        virtual int fini_bind(const std::string &name)
        {
            ACE_UNUSED_ARG(name); return 0;
        }

        virtual std::string make_bind_name(const std::string &name) const
        {
            return name;
        }
    };
    DAF_DECLARE_REFCOUNTABLE(IORBinder);

    class TAF_Export IORBinderSequence : protected IORBinder::_list_type
    {
    public:

        virtual ~IORBinderSequence(void)
        {
            this->clear();
        }

        virtual int init_bind(const std::string &name, CORBA::Object_ptr);
        virtual int fini_bind(void);

        IORBinderSequence & add_binding(const IORBinder_ref &);

        const std::string & bind_name(void) const
        {
            return this->bind_name_;
        }

    private:

        std::string bind_name_;
    };

    /* Stateless Binders - Can be used within the TAF_DEFAULT_IOR_BINDER */

    struct TAF_Export RegisterIORTableBinder : IORBinder {
        virtual int init_bind(const std::string &name, CORBA::Object_ptr);
        virtual int fini_bind(const std::string &name);
    };

    struct TAF_Export RegisterInitReferenceBinder : IORBinder {
        virtual int init_bind(const std::string &name, CORBA::Object_ptr);
        virtual int fini_bind(const std::string &name);
    };

    struct TAF_Export RegisterIORQueryBinder : IORBinder {
        virtual int init_bind(const std::string &name, CORBA::Object_ptr);
        virtual int fini_bind(const std::string &name);
    };

    /* Statefull Binders - Can NOT be used within the TAF_DEFAULT_IOR_BINDER */

    class TAF_Export IORFileBinder : public IORBinder
    {
        std::string directory_, filename_;

    public:

        IORFileBinder(const std::string &directory = "");

        virtual int init_bind(const std::string &name, CORBA::Object_ptr);
        virtual int fini_bind(const std::string &name);

    protected:

        virtual std::string make_bind_name(const std::string &name) const;
    };

    class TAF_Export NamingServiceBinder : public IORBinder
    {
        const TAF::NamingContext context_;
    public:
        NamingServiceBinder(const TAF::NamingContext &context = TheTAFBaseContext());
        virtual int init_bind(const std::string &name, CORBA::Object_ptr);
        virtual int fini_bind(const std::string &name);
    };

} // namespace TAF

extern TAF_Export const TAF::IORBinderSequence & TAF_DEFAULT_IOR_BINDER;

#endif // TAF_IORBINDER_H
