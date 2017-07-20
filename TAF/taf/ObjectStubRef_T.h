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
#ifndef TAF_OBJECTSTUBREF_T_H
#define TAF_OBJECTSTUBREF_T_H

/************************ ObjectStubRef *************************
*
* AUTHOR:   Derek Dominish <derek.dominish@dsto.defence.gov.au>
* DATE:     21st December 2011
*
* TAF::ObjectStubRef provides a safe way of handling the conversion
* of servant instances to CORBA object references while maintaining
* responsibility for the lifecycle of the servant (based on scope).
*
* ImplVar represents ownership of servant lifecycle in the current process
* (ie. is client/server-side neutral) and is intended solely for cases when
* dealing with instances of a CORBA object and it's associated servant.
*
* For example, an application creating function objects for query
* purposes needs control of the servant implementation but also
* requires conversion to CORBA objects for integration with remote interfaces.
*
* This class presents an interface similar to that of the _var classes
* provided by CORBA for managing proxies including reference counting.
*
*****************************************************************/

#include "TAF.h"

#include <daf/ObjectRef_T.h>

#include <tao/PortableServer/PortableServer.h>

namespace TAF
{
    template <typename T>
    struct ObjectStubTraits : DAF::ObjectRefTraits<T>
    {
        typedef typename T::_stub_type              _stub_type;
        typedef typename T::_stub_ptr_type          _stub_ptr_type;
        typedef typename T::_stub_var_type          _stub_var_type;

        static _stub_var_type stub_reference(PortableServer::Servant);

        static _stub_var_type activate(PortableServer::Servant);
        static _stub_var_type activate_with_id(PortableServer::Servant, const PortableServer::ObjectId &);

        static int  deactivate(PortableServer::Servant);
    };

    template <typename T>
    class ObjectStubRef : public T::_stub_var_type
    {
    public:

        typedef T                                           _impl_type;
        typedef ObjectStubTraits<_impl_type>                _impl_traits;

        typedef typename _impl_traits::_obj_type            _servant_type;
        typedef typename _impl_traits::_obj_ptr_type        _servant_ptr_type;
        typedef PortableServer::Servant_var<_servant_type>  _servant_ref_type;

        ObjectStubRef(T *impl = 0, bool auto_deactivate = true);  // We own the IMPL
        ObjectStubRef(const T &impl, bool auto_deactivate = false); // We don't own the IMPL

        ~ObjectStubRef(void);

        ObjectStubRef<T> &  reset(T *impl, bool auto_deactivate = true);
        ObjectStubRef<T> &  reset(const T &impl, bool auto_deactivate = false);

        void    set_auto_deactivate(bool auto_deactivate);
        bool    get_auto_deactivate(void) const;

        T * get_impl(void) const;
        T * release_impl(void);

    private:

        bool                auto_deactivate_;
        _servant_ref_type   servant_ref_;

    private:

        // = Prevent assignment and initialization.
        ACE_UNIMPLEMENTED_FUNC(void operator = (const ObjectStubRef<T> &))
        ACE_UNIMPLEMENTED_FUNC(ObjectStubRef(const ObjectStubRef<T> &))
    };
} // namespace TAF

#if defined (ACE_TEMPLATES_REQUIRE_SOURCE)
# include "ObjectStubRef_T.cpp"
#endif /* ACE_TEMPLATES_REQUIRE_SOURCE */

#if defined (ACE_TEMPLATES_REQUIRE_PRAGMA)
# pragma implementation ("ObjectStubRef_T.cpp")
#endif /* ACE_TEMPLATES_REQUIRE_PRAGMA */

#endif // TAF_OBJECTSUBREF_T_H
