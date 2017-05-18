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
#include "VisitorEnumSupport.h"

#include <fstream>

#include "ace/Log_Msg.h"

#include "be_extern.h"
#include "idl_global.h"
#include "ast_root.h"
#include "ast_module.h"
#include "ast_enum.h"
#include "ast_enum_val.h"

#include "utl_string.h"
#include "utl_identifier.h"

#include "VisitorUtils.h"



namespace TAF
{
  namespace IDL
  {

    int VisitorEnumSupport::indentation_count_ = 0;

    VisitorEnumSupport::VisitorEnumSupport(AST_Decl* scope)
    : Visitor(scope)
    , enums_num_(0)
    , which_function_(ERR)
    , indent_size_(3)
    {

    }

    VisitorEnumSupport::~VisitorEnumSupport(void)
    {

    }

    const char*
    VisitorEnumSupport::generator_name(void)
    {
      return "LASAGNE_EnumSupport";
    }

    int
    VisitorEnumSupport::visit_root(AST_Root *node)
    {
      std::string fileidl = idl_global->stripped_filename()->get_string();

      std::string filename = fileidl.substr(0, fileidl.find("."));

      // Init file
      // Add Pre-processor

      std::string header_id = VisitorUtils::create_header_id(filename, true);

      //just a quick fix for the naming of the header guards so this file does not clash with the 'TAFSuport' equivalent.
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
            std::string taf_name_str =   idl_name_str.substr(0,pos) + be_global->enum_support_ext() + ".h";
            this->header_ << "#include \"" << taf_name_str << "\" // " << idl_name_str<< "\n";
          }
        }
      }


      filename += be_global->enum_support_ext();

      if ( Visitor::visit_root(node) == -1 )
      {
        return -1;
      }

      VisitorUtils::header_guard_end(this->header_, header_id);


      // Close out File
      // Writing to File


      std::string header = be_global->output_dir() + filename + ".h";

      if ( be_global->debug() )
      {
        ACE_DEBUG((LM_INFO, "(%N:%l) Writing to header file %s\n", header.c_str()));
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

      return 0;
    }


    int
    VisitorEnumSupport::visit_module(AST_Module *node)
    {
      // Don't generated "include" idl files
      if ( node->imported() ) return 0;

      // A new module so indent.
      indentation_count_ += indent_size_;

      header_ << indent() << "namespace " << node->full_name() << std::endl;
      header_ << indent() << "{" << std::endl;

      int result = Visitor::visit_module(node);

      indentation_count_ -= indent_size_;

      header_ << indent() << "} // namespace " << node->full_name() << std::endl;

      return result;
    }



    int
    VisitorEnumSupport::visit_enum(AST_Enum *node)
    {
      std::string enum_name = node->local_name()->get_string();
      if ( be_global->debug() ) ACE_DEBUG((LM_INFO, "Visiting Enum %s\n",enum_name.c_str()));

      current_enum_ = enum_name;


      which_function_ = STRING_TO_ENUM;
      indentation_count_ += indent_size_;

      this->header_ << indent() << "enum " << node->full_name() << " " << "StringTo" << current_enum_ << "(const std::string &typeString)" << std::endl;
      this->header_ << indent() << "{" << std::endl;
      int result1 = visit_scope(node);

      indentation_count_ += indent_size_;
      this->header_ << indent() << "ACE_ERROR((LM_ERROR, \"ERROR: %N, %l : The input string has not been matched to any of the " << node->full_name() << " enumerator values \\n\", __FILE__, __LINE__)); \n" << std::endl;

      this->header_ << indent() << "std::runtime_error exception(\"ERROR: The input string has not been matched to any of the " << node->full_name() << " enumerator values \\n\");" << std::endl;
      this->header_ << indent() << "throw exception; \n" << std::endl;

      indentation_count_ -= indent_size_;

      this->header_ << indent() << "} // end function stringTo" << current_enum_ << " \n" << std::endl;

      which_function_ = ENUM_TO_STRING;

      this->header_ << indent() << "std::string " << current_enum_ << "ToString(enum " << node->full_name() << " typeEnum)" << std::endl;

      this->header_ << indent() << "{" << std::endl;
      int result2 = visit_scope(node);

      indentation_count_ += indent_size_;
      this->header_ << indent() << "ACE_ERROR((LM_ERROR, \"ERROR: There is no " << node->full_name() << " enumerator value matching the input enumerator value \\n\")); \n" << std::endl;
      this->header_ << indent() << "return \"ERROR: There is no " << node->full_name() << " enumerator value matching the input enumerator value \\n\";" << std::endl;
      indentation_count_ -= indent_size_;

      this->header_ << indent() << "} // end function " << current_enum_ << "ToString \n\n" << std::endl;

      indentation_count_ -= indent_size_;

      which_function_ = ERR;

      howManyEnums();

      int result = 0;
      if (-1 == result1 || -1 == result2)
      {
          result = -1;
      }

      return result;
    }

    int
    VisitorEnumSupport::visit_enum_val(AST_EnumVal* node)
    {
      std::string enum_value = node->local_name()->get_string();
      if ( be_global->debug() ) ACE_DEBUG((LM_INFO, "Visiting Enum Value %s\n", enum_value.c_str()));

      current_enum_val_ = enum_value;

      switch (which_function_)
      {
      case STRING_TO_ENUM:
          indentation_count_ += indent_size_;
          create_stringToEnum(node);
          indentation_count_ -= indent_size_;
          break;

      case ENUM_TO_STRING:
          indentation_count_ += indent_size_;
          create_enumToString(node);
          indentation_count_ -= indent_size_;
          break;

      default:

          return 0;
          //Not including this for now because the function ends up running here and calling the errors. I beleive this has something to do with the 'doubling up' being seen in previous builds.
          //While it looks like the flags stop an output from being given in the cases of re-running through the tree, it does not stop the re-running which is causing these errors to be called.
          //ACE_ERROR((LM_ERROR, "ERROR: in function VisitorEnumSupport::visit_enum_val() and fell through to line %l", __LINE__));
      }

      return 0;
    }


    int
        VisitorEnumSupport::create_stringToEnum(AST_EnumVal* node)
    {
        this->header_ << indent() << "if(\"" << current_enum_val_ << "\" == typeString)" << std::endl;
        this->header_ << indent() << "{" << std::endl;

        enums_num_ = enums_num_ + 1;

        indentation_count_ += indent_size_;

        this->header_ << indent() << "return " << node->full_name() << ";" << std::endl;

        indentation_count_ -= indent_size_;
        this->header_ << indent() << "}\n" << std::endl;
        return 0;
    }


    int
        VisitorEnumSupport::create_enumToString(AST_EnumVal* node)
    {
        this->header_ << indent() << "if(" << node->full_name() << " == typeEnum)" << std::endl;
        this->header_ << indent() << "{" << std::endl;

        indentation_count_ += indent_size_;

        this->header_ << indent() << "return \"" << current_enum_val_ << "\";" << std::endl;

        indentation_count_ -= indent_size_;
        this->header_ << indent() << "}\n" << std::endl;

        return 0;
    }

    std::string
    VisitorEnumSupport::indent(int ident) const
    {
      std::string indent;
      for (int i = 0; i < ident; i++)
      {
        indent += ' ';
      }

      return indent;
    }


    void VisitorEnumSupport::howManyEnums(void)
    {
        indentation_count_ += indent_size_;

        this->header_ << indent() << "int " << current_enum_ << "Size(void)" << std::endl;
        this->header_ << indent() << "{" << std::endl;

        indentation_count_ += indent_size_;

        this->header_ << indent() << "return " << enums_num_ << ";" << std::endl;

        indentation_count_ -= indent_size_;

        this->header_ << indent() << "} \n" << std::endl;

        indentation_count_ -= indent_size_;

        enums_num_ = 0;
    }

  }//namespace IDL
}//namespace TAF
