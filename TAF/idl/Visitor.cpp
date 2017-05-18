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

#include "Visitor.h"

#include "ast_root.h"
#include "utl_identifier.h"
#include "ast_structure.h"
#include "be_extern.h"


namespace //anonymous
{
  const int VISITOR_VERBOSE = 4;

}//namespace anonymous


namespace TAF
{
namespace IDL
{

Visitor::Visitor(AST_Decl *scope)
{
    ACE_UNUSED_ARG(scope);
}

Visitor::~Visitor(void)
{

}


const char*
Visitor::generator_name(void)
{
    return "unknown";
}

int
Visitor::visit_root(AST_Root* node)
{
    if ( this->visit_scope(node) == -1 )
    {
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("TAF::IDL::Visitor::visit_root - code generation failed\n")),-1);
    }

    // do we need to finalize the stream

    return 0;
}


// Visit the scope and its elements.
int
Visitor::visit_scope (UTL_Scope *node)
{
  if (node == 0)
    {
      ACE_ERROR_RETURN ((LM_ERROR,
                        "(%N:%l) TAF::IDL::Visitor::visit_scope - "
                         "nill node passed\n"),
                        -1);
    }

  // Proceed if the number of members in our scope is greater than 0.
  this->elem_number_ = 0;

  for (UTL_ScopeActiveIterator si (node, UTL_Scope::IK_decls);
       !si.is_done ();
       si.next ())
    {
        AST_Decl *d = si.item ();

        if (d == 0)
        {
            ACE_ERROR_RETURN ((LM_ERROR,
                                "(%N:%l) TAF::IDL::Visitor::visit_scope - "
                                "bad node in this scope\n"),
                                -1);
        }

       if ( be_global->debug() > VISITOR_VERBOSE) ACE_DEBUG((LM_INFO, "Element %d  %s type %d\n", this->elem_number_, d->full_name(), d->node_type()));

        if (d->node_type() == AST_Decl::NT_pre_defined)
        {
            if ( be_global->debug() > VISITOR_VERBOSE ) ACE_DEBUG((LM_INFO, "(%N:%l) TAF::IDL::Visitor - NT_pre_defined found\n"));
            continue;
        }

        ++this->elem_number_;



        if (d->ast_accept(this) == -1 )
        {
            ACE_ERROR_RETURN((LM_ERROR, "(%N:%l) TAF::IDL::Visitor::visit_scope - code generation for scope failed\n"),-1);
        }

    }

  return 0;
}


int
Visitor::visit_decl(AST_Decl *)
{
    return 0;
}

int
Visitor::visit_type(AST_Type* )
{
    return 0;
}

int
Visitor::visit_predefined_type(AST_PredefinedType*)
{
    if ( be_global->debug() > VISITOR_VERBOSE ) ACE_DEBUG((LM_INFO, "(%N:%l) TAF::IDL::Visitor - pre_defined \n"));

    return 0;
}


int
Visitor::visit_module(AST_Module* node)
{
    // check for imported
    //
    //ACE_DEBUG((LM_INFO, "(%N:%l) module %s\n", node->local_name()->get_string() ));

    if (this->visit_scope(node) == -1 )
    {
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%N:%l) TAF::IDL::Visitor::visit_module - visit scope failed\n")),-1);
    }

    return 0;
}


int
Visitor::visit_interface(AST_Interface* )
{
    return 0;
}


int
Visitor::visit_interface_fwd(AST_InterfaceFwd*)
{
    return 0;
}


int
Visitor::visit_valuetype(AST_ValueType* )
{
    return 0;
}


int
Visitor::visit_valuetype_fwd(AST_ValueTypeFwd* )
{
    return 0;
}


int
Visitor::visit_component(AST_Component* )
{
    return 0;
}


int
Visitor::visit_component_fwd(AST_ComponentFwd* )
{
    return 0;
}


int
Visitor::visit_eventtype(AST_EventType* )
{
    return 0;
}


int
Visitor::visit_eventtype_fwd(AST_EventTypeFwd* )
{
    return 0;
}


int
Visitor::visit_home(AST_Home*)
{
    return 0;
}


int
Visitor::visit_factory(AST_Factory*)
{
    return 0;
}


int
Visitor::visit_structure(AST_Structure* node)
{
  ACE_UNUSED_ARG(node);
  if ( be_global->debug() > VISITOR_VERBOSE ) ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%N:%l) Structure %s\n"), node->local_name()->get_string()));
    return 0;
}


int
Visitor::visit_structure_fwd(AST_StructureFwd* )
{
    return 0;
}


int
Visitor::visit_exception(AST_Exception* )
{
    return 0;
}


int
Visitor::visit_expression(AST_Expression* )
{
    return 0;
}


int
Visitor::visit_enum(AST_Enum* )
{
    return 0;
}


int
Visitor::visit_operation(AST_Operation* )
{
    return 0;
}


int
Visitor::visit_field(AST_Field* )
{
    return 0;
}


int
Visitor::visit_argument(AST_Argument* )
{
    return 0;
}


int
Visitor::visit_attribute(AST_Attribute* )
{
    return 0;
}


int
Visitor::visit_union(AST_Union* )
{
    return 0;
}


int
Visitor::visit_union_fwd(AST_UnionFwd* )
{
return 0;
}


int
Visitor::visit_union_branch(AST_UnionBranch* )
{
    return 0;
}


int
Visitor::visit_union_label(AST_UnionLabel* )
{
    return 0;
}


int
Visitor::visit_constant(AST_Constant* )
{
    return 0;
}


int
Visitor::visit_enum_val(AST_EnumVal* )
{
    return 0;
}


int
Visitor::visit_array(AST_Array* )
{
    return 0;
}


int
Visitor::visit_sequence(AST_Sequence* )
{
    return 0;
}


int
Visitor::visit_string(AST_String* )
{
    return 0;
}

int
Visitor::visit_typedef(AST_Typedef* )
{
    return 0;
}

int
Visitor::visit_native(AST_Native* )
{
    return 0;
}


int
Visitor::visit_valuebox(AST_ValueBox* )
{
    return 0;
}


int
Visitor::visit_template_module (AST_Template_Module* )
{
    return 0;
}


int
Visitor::visit_template_module_inst (AST_Template_Module_Inst*)
{
    return 0;
}


int
Visitor::visit_template_module_ref(AST_Template_Module_Ref* )
{
    return 0;
}


int
Visitor::visit_param_holder(AST_Param_Holder* )
{
    return 0;
}


int
Visitor::visit_porttype(AST_PortType* )
{
    return 0;
}


int
Visitor::visit_provides(AST_Provides* )
{
    return 0;
}


int
Visitor::visit_uses(AST_Uses* )
{
    return 0;
}


int
Visitor::visit_publishes(AST_Publishes* )
{
    return 0;
}


int
Visitor::visit_emits(AST_Emits* )
{
    return 0;
}


int
Visitor::visit_consumes(AST_Consumes* )
{
    return 0;
}


int
Visitor::visit_extended_port(AST_Extended_Port* )
{
    return 0;
}


int
Visitor::visit_mirror_port(AST_Mirror_Port* )
{
    return 0;
}


int
Visitor::visit_connector(AST_Connector* )
{
    return 0;
}


int
Visitor::visit_finder(AST_Finder* )
{
    return 0;
}


}//namespace IDL
}//namespace TAF
