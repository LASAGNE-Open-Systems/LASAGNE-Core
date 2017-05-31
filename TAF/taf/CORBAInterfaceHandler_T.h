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
#ifndef TAF_CORBAInterfaceHandler_T_H
#define TAF_CORBAInterfaceHandler_T_H


#include "IORBinder.h"
#include "ObjectStubRef_T.h"

#include <tao/PortableServer/PortableServer.h>

#include <ace/Copy_Disabled.h>

#include <string>

/*
    NOTE: TAF_DEFAULT_IOR_BINDER:
    1) TAF::RegisterIORTableBinder()
    2) TAF::RegisterInitReferenceBinder()
    3) TAF::RegisterIORQueryBinder()
*/

namespace TAF
{
    template < typename INTERFACE_TYPE, typename DEFAULTPOA_TYPE = TAF_DEFAULT_POA >
    class CORBAInterfaceHandler_T : virtual public INTERFACE_TYPE
        , protected DEFAULTPOA_TYPE, ACE_Copy_Disabled
    {
        IORBinderSequence   ior_binder_;

    public:

        typedef INTERFACE_TYPE                                  _interface_type;
        typedef DEFAULTPOA_TYPE                                 _default_poa_type;

        typedef typename INTERFACE_TYPE::_stub_type             _interface_stub_type;
        typedef typename INTERFACE_TYPE::_stub_var_type         _interface_stub_var_type;

        typedef typename TAF::ObjectStubTraits<INTERFACE_TYPE>  _interface_activator_type;

        CORBAInterfaceHandler_T(const IORBinderSequence & = TAF_DEFAULT_IOR_BINDER);
        virtual ~CORBAInterfaceHandler_T(void);

        const _interface_stub_var_type & stub_reference(void) const
        {
            return this->stub_reference_;
        }

    protected:

        virtual int init_bind(const std::string &name);
        virtual int init_bind(const std::string &name, const PortableServer::ObjectId &id);

        virtual int fini_bind(void);

    protected:

        void    addIORBinding(const IORBinder_ref &binding)
        {
            this->ior_binder_.add_binding(binding);
        }

    private:

        virtual int init_bind_i(const std::string &name);

        _interface_stub_var_type    stub_reference_;

        bool    stub_activated_;
    };

} // namespace TAF

#if defined (ACE_TEMPLATES_REQUIRE_SOURCE)
# include "CORBAInterfaceHandler_T.cpp"
#endif /* ACE_TEMPLATES_REQUIRE_SOURCE */

#if defined (ACE_TEMPLATES_REQUIRE_PRAGMA)
# pragma implementation ("CORBAInterfaceHandler_T.cpp")
#endif /* ACE_TEMPLATES_REQUIRE_PRAGMA */

#endif // TAF_CORBAInterfaceHandler_T_H
