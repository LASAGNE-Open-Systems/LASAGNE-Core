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
#ifndef TAF_NAMINGCONTEXT_H
#define TAF_NAMINGCONTEXT_H

#include "TAF.h"

#include <orbsvcs/CosNamingC.h>

namespace TAF
{
    struct TAF_Export NamingContext : CosNaming::NamingContext_var
    {
        NamingContext(void); // defaults to ORB::the_rootContext();
        NamingContext(const CosNaming::NamingContext_var&);
        NamingContext(CosNaming::NamingContext_ptr);
        NamingContext(const std::string &cxt_path);
        NamingContext(const NamingContext&);  // Copy Constructor

        NamingContext&  operator = (const CosNaming::NamingContext_var&);
        NamingContext&  operator = (CosNaming::NamingContext_ptr);
        NamingContext&  operator = (const std::string &cxt_path);
        NamingContext&  operator = (const NamingContext&);

        struct TAF_Export BindingIterator : CosNaming::BindingIterator_var {
            BindingIterator(void);
            BindingIterator(const CosNaming::BindingIterator_var&);
            BindingIterator(CosNaming::BindingIterator_ptr);

            virtual ~BindingIterator(void);
        };

        NamingContext   find_context(const std::string &cxt_path) const;
        NamingContext   bind_context(const std::string &cxt_path) const;

        void bind_name(const std::string &cxt_pathname, const CORBA::Object_ptr) const;
        void unbind_name(const std::string &cxt_pathname) const;

        CORBA::Object_var   resolve_name(const std::string &cxt_pathname) const;

        CosNaming::BindingList_var list_context(BindingIterator&, CORBA::ULong max = 32) const;

        static std::string trim_context(const std::string &context);
    };

} // namespace TAF

TAF_Export std::ostream & operator << (std::ostream &os, const TAF::NamingContext &cxt);

#endif // TAF_NAMINGCONTEXT_H
