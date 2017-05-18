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
#ifndef TAF_OBJECTSTUBREF_T_CPP
#define TAF_OBJECTSTUBREF_T_CPP

#include "taf/ObjectStubRef_T.h"

#include "tao/PortableServer/Servant_Base.h"

namespace TAF
{
    template <typename T>
    typename T::_stub_var_type
    ObjectStubTraits<T>::stub_reference(PortableServer::Servant p)
    {
        if (p) try {
            PortableServer::POA_var poa(p->_default_POA());
            if (!CORBA::is_nil(poa.in())) {
                return T::_stub_type::_narrow(CORBA::Object_var(poa->servant_to_reference(p)));
            }
        } DAF_CATCH_ALL {}
        return T::_stub_type::_nil();
    }

    template <typename T>
    typename T::_stub_var_type
    ObjectStubTraits<T>::activate(PortableServer::Servant p)
    {
        if (p) try {
            for (PortableServer::POA_var poa(p->_default_POA()); poa;) {
                PortableServer::ObjectId_var oid(poa->activate_object(p));
                return T::_stub_type::_narrow(CORBA::Object_var(poa->id_to_reference(oid)));
            }
        }
        catch (const PortableServer::POA::ServantAlreadyActive&) {
            return ObjectStubTraits<T>::stub_reference(p);
        }
        throw PortableServer::POA::NoServant();
    }

    template <typename T>
    typename T::_stub_var_type
    ObjectStubTraits<T>::activate_with_id(PortableServer::Servant p, const PortableServer::ObjectId &oid)
    {
        if (p) try {
            for (PortableServer::POA_var poa(p->_default_POA()); poa;) {
                poa->activate_object_with_id(oid, p);
                return T::_stub_type::_narrow(CORBA::Object_var(poa->id_to_reference(oid)));
            }
        }
        catch (const PortableServer::POA::ServantAlreadyActive&) {
            return ObjectStubTraits<T>::stub_reference(p);
        }
        throw PortableServer::POA::NoServant();
    }

    template <typename T>
    int
    ObjectStubTraits<T>::deactivate(PortableServer::Servant p)
    {
        if (p) try {
            for (PortableServer::POA_var poa(p->_default_POA()); poa;) {
                poa->deactivate_object(PortableServer::ObjectId_var(poa->servant_to_id(p))); break;
            }
        } DAF_CATCH_ALL{ /* Ignore any Error */ }
        return 0;
    }

    template <typename T>
    ObjectStubRef<T>::ObjectStubRef(T *impl, bool auto_deactivate) // We own the IMPL
        : auto_deactivate_  (auto_deactivate)
        , servant_ref_      (impl) // Won't duplicate
    {
        if (!DAF::is_nil(this->servant_ref_)) {
            this->out() = _impl_traits::activate(this->servant_ref_.in())._retn();
        }
    }

    template <typename T>
    ObjectStubRef<T>::ObjectStubRef(const T &impl, bool auto_deactivate) // We don't own the impl
        : auto_deactivate_  (auto_deactivate)
        , servant_ref_      (_servant_ref_type::_duplicate(const_cast<T*>(&impl)))
    {
        if (!DAF::is_nil(this->servant_ref_)) {
            this->out() = _impl_traits::activate(this->servant_ref_.in())._retn();
        }
    }

    template <typename T>
    ObjectStubRef<T>::~ObjectStubRef(void)
    {
        if (DAF::is_nil(this->servant_ref_) ? false : this->get_auto_deactivate()) {
            _impl_traits::deactivate(this->servant_ref_.in());
        }
    }

    template <typename T>
    ObjectStubRef<T> &
    ObjectStubRef<T>::reset(T *impl, bool auto_deactivate)
    {
        if (this->servant_ref_ != impl) { // Resetting to ourselves ??

            if (DAF::is_nil(this->servant_ref_) ? false : this->auto_deactivate_) {
                _impl_traits::deactivate(this->servant_ref_.in());
            }

            if (DAF::is_nil(this->servant_ref_ = impl)) {
                this->out() = T::_stub_type::_nil();
            }
            else {
                this->out() = _impl_traits::activate(this->servant_ref_.in())._retn();
            }
        }

        this->set_auto_deactivate(auto_deactivate); return *this;
    }

    template <typename T>
    ObjectStubRef<T> &
    ObjectStubRef<T>::reset(const T &impl, bool auto_deactivate)
    {
        if (this->servant_ref_ != &impl) { // Resetting to ourselves ??

            if (DAF::is_nil(this->servant_ref_) ? false : this->get_auto_deactivate()) {
                _impl_traits::deactivate(this->servant_ref_.in());
            }

            if (DAF::is_nil(this->servant_ref_ = _servant_ref_type::_duplicate(const_cast<T*>(&impl)))) {
                this->out() = T::_stub_type::_nil();
            }
            else {
                this->out() = _impl_traits::activate(this->servant_ref_.in())._retn();
            }
        }

        this->set_auto_deactivate(auto_deactivate); return *this;
    }

    template <typename T> inline
    void
    ObjectStubRef<T>::set_auto_deactivate(bool auto_deactivate)
    {
        this->auto_deactivate_ = auto_deactivate;
    }

    template <typename T> inline
    bool
    ObjectStubRef<T>::get_auto_deactivate(void) const
    {
        return this->auto_deactivate_;
    }

    template <typename T> inline
    T *
    ObjectStubRef<T>::get_impl(void) const
    {
        return dynamic_cast<T*>(this->servant_ref_.in());
    }

    template <typename T> inline
    T *
    ObjectStubRef<T>::release_impl(void)
    {
        return dynamic_cast<T*>(this->servant_ref_._retn());
    }

} // namespace TAF

#endif // TAF_OBJECTSTUBREF_T_CPP
