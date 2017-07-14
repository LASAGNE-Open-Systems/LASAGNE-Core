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
#ifndef TAF_CORBAInterfaceHandler_T_CPP
#define TAF_CORBAInterfaceHandler_T_CPP

namespace TAF
{
    /*******************************************************************************************/

    template < typename INTERFACE_TYPE, typename DEFAULTPOA_TYPE >
    CORBAInterfaceHandler_T< INTERFACE_TYPE, DEFAULTPOA_TYPE >::CORBAInterfaceHandler_T(const IORBinderSequence &binder)
        : INTERFACE_TYPE(), ior_binder_(binder), stub_reference_(0), stub_activated_(false)
    {
    }

    template < typename INTERFACE_TYPE, typename DEFAULTPOA_TYPE >
    CORBAInterfaceHandler_T< INTERFACE_TYPE, DEFAULTPOA_TYPE >::~CORBAInterfaceHandler_T(void)
    {
        if (this->stub_activated_) {
            ACE_ERROR((LM_ERROR, ACE_TEXT("TAF (%P | %t) ERROR: %C\n")
                ACE_TEXT(" -->> destructor called with servant still activated - use fini_bind()\n")
                , typeid(*this).name()));
        }
    }

    template < typename INTERFACE_TYPE, typename DEFAULTPOA_TYPE > int
    CORBAInterfaceHandler_T< INTERFACE_TYPE, DEFAULTPOA_TYPE >::init_bind(const std::string &name)
    {
        if (!this->stub_activated_) {
            try {
                if (!CORBA::is_nil((this->stub_reference_ = _interface_activator_type::activate(this)).in())) {
                    return this->init_bind_i(name);
                }
            } DAF_CATCH_ALL{
            }
        }
        return -1;
    }

    template < typename INTERFACE_TYPE, typename DEFAULTPOA_TYPE > int
    CORBAInterfaceHandler_T< INTERFACE_TYPE, DEFAULTPOA_TYPE >::init_bind(const std::string &name, const PortableServer::ObjectId &id)
    {
        if (!this->stub_activated_) {
            try {
                if (!CORBA::is_nil((this->stub_reference_ = _interface_activator_type::activate_with_id(this, id)).in())) {
                    return this->init_bind_i(name);
                }
            } DAF_CATCH_ALL {
            }
        }
        return -1;
    }

    template < typename INTERFACE_TYPE, typename DEFAULTPOA_TYPE > int
    CORBAInterfaceHandler_T< INTERFACE_TYPE, DEFAULTPOA_TYPE >::fini_bind(void)
    {
        int rtn = -1;

        if (this->stub_activated_) try {
            this->stub_activated_ = false; rtn = this->ior_binder_.fini_bind(); _interface_activator_type::deactivate(this);
        } DAF_CATCH_ALL {
        }

        return rtn;
    }

    template < typename INTERFACE_TYPE, typename DEFAULTPOA_TYPE > int
    CORBAInterfaceHandler_T< INTERFACE_TYPE, DEFAULTPOA_TYPE >::init_bind_i(const std::string &name)
    {
        for (const _interface_stub_var_type & stub(this->stub_reference()); stub;) {
            this->stub_activated_ = true; return this->ior_binder_.init_bind(name, stub);
        }
        return -1;
    }

} // namespace TAF

#endif // TAF_CORBAInterfaceHandler_T_CPP
