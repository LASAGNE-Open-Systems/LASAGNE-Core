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
#define TAF_NAMINGCONTEXT_CPP

#include "NamingContext.h"

#include "ORBManager.h"

#include <sstream>

namespace {

    const size_t MAX_GRAPH_DEPTH = 10;
    const CORBA::ULong MAX_CHUNK = 32;

    std::string make_naming_graph(const TAF::NamingContext &cxt, size_t depth)
    {
        std::stringstream ss;

        if (CORBA::is_nil(cxt.in()) ? false : MAX_GRAPH_DEPTH > depth) try {

            TAF::NamingContext::BindingIterator it;
            CosNaming::BindingList_var bl(cxt.list_context(it, MAX_CHUNK));

            do {

                const CORBA::ULong bl_len = (bl ? bl->length() : 0U);

                if (bl_len) for (CORBA::ULong i = 0; i < bl_len; i++) {

                    const CosNaming::Name &binding_name = bl[i].binding_name;

                    for (size_t pad = 0; pad++ < depth; ss << '|' << ' ')
                        ;

                    ss << '+' << ' ' << binding_name[0].id.in() << ' ';

                    switch (bl[i].binding_type) {
                    case CosNaming::nobject:    ss << "[ref]" << std::endl; break;
                    case CosNaming::ncontext:   ss << "[cxt]" << std::endl;
                        {
                            TAF::NamingContext nxt_cxt(CosNaming::NamingContext::_narrow(cxt->resolve(binding_name)));
                            if (CORBA::is_nil(nxt_cxt.in())) {
                                throw CORBA::BAD_CONTEXT();
                            }
                            ss << make_naming_graph(nxt_cxt, depth + 1);
                        } break;

                    default: ss << "[-?-]" << std::endl; break;
                    }

                } else break;

            } while (CORBA::is_nil(it.in()) ? false : it->next_n(MAX_CHUNK, bl));

        } catch (const CORBA::Exception &ex) {
            for (size_t pad = 0; pad++ < depth; ss << '|' << ' ')
                ;
            ss << "|->\"" << ex._rep_id() << '\"' << std::endl;
        } DAF_CATCH_ALL{
        }

        return ss.str();
    }
}

std::ostream & operator << (std::ostream &os, const TAF::NamingContext &cxt)
{
   return os << make_naming_graph(cxt, 0).append(1,0); // Append a Null
}

namespace TAF
{
    NamingContext::NamingContext(void)
    {
    }

    NamingContext::NamingContext(CosNaming::NamingContext_ptr p)
        : CosNaming::NamingContext_var(p)
    {
    }

    NamingContext::NamingContext(const CosNaming::NamingContext_var &cxt)
        : CosNaming::NamingContext_var(cxt)
    {
    }

    NamingContext::NamingContext(const std::string &cxt_path)
        : CosNaming::NamingContext_var()
    {
        this->out() = TheTAFRootContext().bind_context(cxt_path)._retn();
    }

    NamingContext::NamingContext(const NamingContext &cxt)
        : CosNaming::NamingContext_var(cxt)
    {
    }

    NamingContext&
    NamingContext::operator = (const CosNaming::NamingContext_var &cxt)
    {
        CosNaming::NamingContext_ptr p = cxt.in(); if (*this != p) {
            this->out() = CosNaming::NamingContext::_duplicate(p);
        }
        return *this;
    }

    NamingContext&
    NamingContext::operator = (CosNaming::NamingContext_ptr cxt_ptr)
    {
        this->out() = cxt_ptr; return *this;
    }

    NamingContext&
    NamingContext::operator = (const std::string &cxt_path)
    {
        return *this = TheTAFRootContext().bind_context(cxt_path)._retn();
    }

    NamingContext&
    NamingContext::operator = (const NamingContext &cxt)
    {
        this->out() = CosNaming::NamingContext::_duplicate(cxt.in()); return *this;
    }

    std::string
    NamingContext::trim_context(const std::string &context)
    {
        if (int(context.find_first_of('/')) == 0) {
            return std::string("/").append(DAF::trim_string(context.substr(1), '/'));
        }
        return DAF::trim_string(context, '/');
    }

    NamingContext
    NamingContext::find_context(const std::string &cxt_pathname) const
    {
        const std::string cxt_path(trim_context(cxt_pathname));

        int pos = int(cxt_path.find_first_of('/')); if (pos == 0) {
            return TheTAFRootContext().find_context(cxt_path.substr(1))._retn();
        }

        for (const std::string cxt_name(trim_context(cxt_path.substr(0,pos++))); cxt_name.length() > 0;) {
            CosNaming::Name cos_name(1); cos_name.length(1);
            cos_name[0].id = cxt_name.c_str();

            for (CosNaming::NamingContext_var cxt_obj(CosNaming::NamingContext::_narrow((*this)->resolve(cos_name))); cxt_obj;) {
                return pos ? NamingContext(cxt_obj._retn()).find_context(cxt_path.substr(pos))._retn() : cxt_obj._retn();
            }

            throw CosNaming::NamingContext::NotFound();
        }

        return CosNaming::NamingContext::_duplicate(this->in());
    }

    NamingContext
    NamingContext::bind_context(const std::string &cxt_pathname) const
    {
        const std::string cxt_path(trim_context(cxt_pathname));

        int pos = int(cxt_path.find_first_of('/')); if (pos == 0) {
            return TheTAFRootContext().bind_context(cxt_path.substr(1))._retn();
        }

        for (const std::string cxt_name(trim_context(cxt_path.substr(0,pos++))); cxt_name.length() > 0;) {
            CosNaming::Name cos_name(1); cos_name.length(1);
            cos_name[0].id = cxt_name.c_str();

            CosNaming::NamingContext_var cxt_obj;

            try {
                cxt_obj = CosNaming::NamingContext::_narrow((*this)->resolve(cos_name));
            } catch (const CosNaming::NamingContext::NotFound&) {
                cxt_obj = (*this)->bind_new_context(cos_name);
            }

            return pos ? NamingContext(cxt_obj._retn()).bind_context(cxt_path.substr(pos))._retn() : cxt_obj._retn();
        }

        return CosNaming::NamingContext::_duplicate(this->in());
    }

    void
    NamingContext::bind_name(const std::string &obj_pathname, const CORBA::Object_ptr obj) const
    {
        const std::string obj_name(trim_context(obj_pathname));

        if (obj_name.length() == 0) {
            throw CORBA::BAD_PARAM();
        }

        int pos = int(obj_name.find_last_of('/'));

        for (int cpos = pos; cpos++ > 0;) {
            NamingContext cxt(this->bind_context(obj_name.substr(0, size_t(cpos))));
            cxt.bind_name(obj_name.substr(size_t(cpos)), obj);
            return;
        }

        if (pos++ == 0) {
            TheTAFRootContext().bind_name(obj_name.substr(pos), obj); return;
        } else if (CORBA::is_nil(obj)) {
            return;  // No Object to bind (But we have put the path in anyway).
        }

        CosNaming::Name cos_name(1); cos_name.length(1);

        cos_name[0].id = obj_name.c_str();
        cos_name[0].kind = "";

        CosNaming::NamingContext_var cxt(CosNaming::NamingContext::_narrow(obj)); // Try For a Context

        if (CORBA::is_nil(cxt.in())) {
            (*this)->rebind(cos_name, obj);
        } else {
            (*this)->rebind_context(cos_name, cxt.in());
        }
    }

    void
    NamingContext::unbind_name(const std::string &obj_pathname) const
    {
        const std::string obj_name(trim_context(obj_pathname));

        if (obj_name.length() == 0) {
            throw CORBA::BAD_PARAM();
        }

        int pos = int(obj_name.find_last_of('/'));

        for (int cpos = pos; cpos++ > 0;) {
            NamingContext cxt(this->find_context(obj_name.substr(0, size_t(cpos))));
            cxt.unbind_name(obj_name.substr(size_t(cpos)));
            return;
        }

        if (pos++ == 0) {
            TheTAFRootContext().unbind_name(obj_name.substr(size_t(pos))); return;
        }

        CosNaming::Name cos_name(1); cos_name.length(1);

        cos_name[0].id      = obj_name.c_str();
        cos_name[0].kind    = "";

        (*this)->unbind(cos_name);
    }

    CORBA::Object_var
    NamingContext::resolve_name(const std::string &obj_pathname) const
    {
        for (const std::string obj_name(trim_context(obj_pathname)); obj_name.length() > 0;) {
            int pos = int(obj_name.find_last_of('/')); if (pos > 0) { int cpos = pos++;
                return NamingContext(this->find_context(obj_name.substr(0,size_t(cpos)))).resolve_name(obj_name.substr(size_t(pos)));
            } else if (pos == 0) {
                return TheTAFRootContext().resolve_name(obj_name.substr(size_t(++pos)));
            }

            CosNaming::Name cos_name(1); cos_name.length(1);

            cos_name[0].id      = obj_name.c_str();
            cos_name[0].kind    = "";

            return (*this)->resolve(cos_name);
        }

        throw CosNaming::NamingContext::NotFound();
    }

    CosNaming::BindingList_var
    NamingContext::list_context(NamingContext::BindingIterator &it, CORBA::ULong max) const
    {
        CosNaming::BindingList_var bl;
        (*this)->list(CORBA::ULong(ace_range(1, 100, int(max))), bl, it);
        return bl._retn();
    }

    /***************************************************************************************/

    NamingContext::BindingIterator::BindingIterator(void)
    {
    }

    NamingContext::BindingIterator::BindingIterator(const CosNaming::BindingIterator_var &s)
        : CosNaming::BindingIterator_var(s)
    {
    }

    NamingContext::BindingIterator::BindingIterator(CosNaming::BindingIterator_ptr p)
        : CosNaming::BindingIterator_var(p)
    {
    }

    NamingContext::BindingIterator::~BindingIterator(void)
    {
        if (!CORBA::is_nil(this->in())) {
            (*this)->destroy();
        }
    }

} // namespace TAF
