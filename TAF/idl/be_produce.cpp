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

#include "TAF_IDL_BE_export.h"

#include "ace/Log_Msg.h"
#include "global_extern.h"
#include "be_extern.h"
#include "be_global.h"

#include "ast_root.h"
#include "VisitorTafSupport.h"
#include "VisitorEnumSupport.h"


void
BE_cleanup()
{
    idl_global->destroy();
}


void
BE_abort()
{
    ACE_ERROR((LM_ERROR, ACE_TEXT("Fatal Error - Aborting\n")));
    BE_cleanup();
    ACE_OS::exit(1);
}

void
BE_produce()
{
    if ( be_global->debug() ) ACE_DEBUG((LM_DEBUG, ACE_TEXT("LASAGNE IDL Compiler - DO STUFF!\n")));

    // At this stage we are using our be_global to run through what needs to be generated!
    AST_Root *root = idl_global->root();

    if ( be_global->gen_taf_data_support() )
    {
        if (be_global->debug()) ACE_DEBUG((LM_DEBUG, ACE_TEXT("LASAGNE IDL Compiler - Generating TAF Support Files\n")));
        TAF::IDL::VisitorTafSupport visitor(root);

        if ( root->ast_accept(&visitor) == -1 )
        {
            ACE_ERROR((LM_ERROR, ACE_TEXT("(%N:%l) BE_produce - Failed to accept Visitor TafSupport\n")));
            BE_abort();
        }

    }

    if ( be_global->gen_enum_support() )
    {
      if (be_global->debug()) ACE_DEBUG((LM_DEBUG, ACE_TEXT("LASAGNE IDL Compiler - Generating Enum Support Files\n")));
      TAF::IDL::VisitorEnumSupport visitor(root);

      if ( root->ast_accept(&visitor) == -1 )
      {
        ACE_ERROR((LM_ERROR, ACE_TEXT("(%N:%l) BE_produce - Failed to accept Visitor Enum Support\n")));
        BE_abort();
      }
    }
}
