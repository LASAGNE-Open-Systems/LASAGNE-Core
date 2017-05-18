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
#ifndef __TAF_IDL_VISITOR_TAF_SUPPORT_H
#define __TAF_IDL_VISITOR_TAF_SUPPORT_H

#include "TAF_IDL_BE_export.h"

#include "Visitor.h"

namespace TAF
{
namespace IDL
{

class TAF_IDL_BE_Export VisitorTafSupport : public Visitor
{
public:
    VisitorTafSupport(AST_Decl *scope);
    virtual ~VisitorTafSupport(void);

    virtual const char* generator_name(void);

    virtual int visit_root(AST_Root *node);
    virtual int visit_module(AST_Module *node);
    virtual int visit_structure(AST_Structure *node);

protected:

    void add_preamble(const std::string& filename);

    std::string data_support_name(const std::string &name);

    std::string indent(int ident = indentation_count_);

    std::ostringstream header_;
    std::ostringstream impl_;
    std::string current_namespace_;

    static int indentation_count_;

};

}//namespace IDL
}//namespace TAF

#endif //__TAF_IDL_VISITOR_TAF_SUPPORT_H
