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
#include "VisitorTafSupport.h"

#include <fstream>

#include "ace/Log_Msg.h"

#include "be_extern.h"
#include "idl_global.h"
#include "ast_root.h"
#include "ast_structure.h"
#include "ast_module.h"
#include "utl_string.h"
#include "utl_identifier.h"

#include "VisitorUtils.h"



namespace TAF
{
namespace IDL
{

int VisitorTafSupport::indentation_count_ = 0;

VisitorTafSupport::VisitorTafSupport(AST_Decl* scope)
: Visitor(scope)
, current_namespace_()
{
}

VisitorTafSupport::~VisitorTafSupport(void)
{

}

const char*
VisitorTafSupport::generator_name(void)
{
    return "LASAGNE_TAFSupport";
}

int
VisitorTafSupport::visit_root(AST_Root *node)
{
    std::string fileidl = idl_global->stripped_filename()->get_string();
    std::string filename = fileidl.substr(0, fileidl.find("."));
    // Init file
    // Add Pre-processor

    std::string header_id = VisitorUtils::create_header_id(filename, true);

    header_id += generator_name();

    VisitorUtils::header_guard_start(this->header_, header_id);
    VisitorUtils::generation_comment(this->header_, this->generator_name());


    // IDL include files
    // Work out prior to pre-processor input
    size_t const nfiles = idl_global->n_included_idl_files();
    if (nfiles > 0 )
    {
      this->header_ << "// IDL Includes " << nfiles << std::endl;

      for ( size_t i =0 ; i < nfiles; ++i )
      {
        char *idl_name = idl_global->included_idl_files()[i];

        std::string idl_name_str = idl_name;

        size_t pos = idl_name_str.find(".");
        if ( pos != std::string::npos)
        {
          std::string taf_name_str =   idl_name_str.substr(0,pos) + be_global->taf_support_ext() + ".h";
          this->header_ << "#include \"" << taf_name_str << "\" // " << idl_name_str<< "\n";
        }
      }
    }


    VisitorUtils::data_support_includes(this->header_, filename);
    VisitorUtils::taf_dds_headers(this->header_);


    filename += be_global->taf_support_ext();


    this->impl_ << "#include \"" << filename + ".h" << "\"\n\n";


    if ( Visitor::visit_root(node) == -1 )
    {
        return -1;
    }

    VisitorUtils::header_guard_end(this->header_, header_id);


    // Close out File
    // Writing to File


    std::string header = be_global->output_dir() + filename + ".h";
    std::string impl = be_global->output_dir() + filename + ".cpp";

    if ( be_global->debug() )
    {
      ACE_DEBUG((LM_INFO, "(%N:%l) Writing to header file %s\n", header.c_str()));
      ACE_DEBUG((LM_INFO, "(%N:%l) Writing to impl file %s\n", impl.c_str()));
    }

    std::ofstream header_file( header.c_str());
    if ( header_file.is_open() )
    {
        header_file << this->header_.str();
        header_file.close();
    }
    else
    {
        ACE_ERROR((LM_ERROR, "(%N:%l) Failed to open '%s'\n", header.c_str()));
    }


    std::ofstream impl_file( impl.c_str());
    if ( impl_file.is_open() )
    {
        impl_file << this->impl_.str();
        impl_file.close();
    }
    else
    {
        ACE_ERROR((LM_ERROR, "(%N:%l) Failed to open '%s'\n", impl.c_str()));
    }

    return 0;
}

int
VisitorTafSupport::visit_structure( AST_Structure *node)
{
    if ( node->imported() ) return 0;

    IDL_GlobalData::DCPS_Data_Type_Info *info = idl_global->is_dcps_type(node->name());

    if (0 != info )
    {
        if (be_global->debug() ) ACE_DEBUG((LM_INFO, "(%N:%l) Found DDS Type %s  Imported %s\n", node->full_name(), node->imported() ? "TRUE" : "FALSE"));

        std::string name = node->local_name()->get_string();

        this->header_

            // The DEFINE_DDS_TYPESUPPORT macro.
            << indent() << "typedef DEFINE_DDS_TYPESUPPORT(" << current_namespace_ << ", " << name << ") " << data_support_name(name) << ";" << std::endl

            << indent() << "//typedef " << data_support_name(name) << "::_data_holder_type" << name << "_HolderType" << std::endl
            << indent() << "typedef " << data_support_name(name) << "::_data_type    " << name << "_TopicType;" << std::endl
            << indent() << "typedef " << data_support_name(name) << "::_data_type    " << name << "_CORBAType;" << std::endl
            << indent() << "typedef " << data_support_name(name) << "::_data_type    " << name << "_TopicAdapter;" << std::endl
            << indent() << "typedef " << data_support_name(name) << "::_data_type    " << name << "_CORBAAdapter;" << std::endl << std::endl

            << indent() << "typedef class TAFDDS::TOPICReaderListener< " << data_support_name(name) << " > " << name << "_DataListener; " << std::endl
            << indent() << "typedef class TAFDDS::DDS_Subscriber< " << "TAFDDS::SubscriberListener" << " > " << name << "_Subscriber;" << std::endl
            << indent() << "typedef class TAFDDS::DDS_Publisher< " << "TAFDDS::PublisherListener" << " > " << name << "_Publisher;" << std::endl
            << indent() << "typedef class TAFDDS::DDS_Topic< " << data_support_name(name) << " > " << name << "_Topic;" << std::endl
            << indent() << "typedef class TAFDDS::DDS_Reader< " << name << "_Topic" << ", " <<  name << "_DataListener" << " > " << name << "_Reader;" << std::endl
            << indent() << "typedef class TAFDDS::DDS_Writer< " << data_support_name(name) << " > " << name << "_Writer;" << std::endl << std::endl;
    }

    return 0;
}


int
VisitorTafSupport::visit_module(AST_Module *node)
{
    // Don't generated "include" idl files
    if ( node->imported() ) return 0;


    const char *module_name = node->local_name()->get_string();

    header_ << std::endl << indent() << "namespace " << module_name << std::endl << indent() << "{" << std::endl;

    this->impl_ << std::endl << "namespace " << module_name << std::endl << "{" << std::endl;;

    // A new module so indent.
    indentation_count_ += 3;

    // Remember the current module/namespace for later use.
    current_namespace_ = module_name;

    int result = Visitor::visit_module(node);

    indentation_count_ -= 3;

    this->header_ << indent() << "} // namespace " << module_name << std::endl;

    this->impl_ << "} // namespace " << module_name << std::endl;

    return result;
}

std::string
VisitorTafSupport::data_support_name(const std::string& name)
{
    return std::string(name + "_DataSupport");
}

std::string
VisitorTafSupport::indent(int ident)
{
    std::string indent;
    for (int i = 0; i < ident; i++)
    {
        indent += ' ';
    }

    return indent;
}


}//namespace IDL
}//namespace TAF
