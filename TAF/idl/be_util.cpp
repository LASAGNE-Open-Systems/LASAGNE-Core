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
#include "be_util.h"
#include "ace/Log_Msg.h"

#include "ast_generator.h"

void
be_util::usage(void)
{
    ACE_DEBUG((LM_INFO, "\nLASAGNE IDL BE\n"));
    ACE_DEBUG((LM_INFO, "\t-notaf          : Prohibit TAF Support  file generation\n"));
    ACE_DEBUG((LM_INFO, "\t-tafext <name>    : TAF Support extension -  default \"TAFSupport.h,cpp\"\n"));
    ACE_DEBUG((LM_INFO, "\t-enum             : Generate Enum Suppport -  default \"EnumSupport.h\"\n"));
}

AST_Generator*
be_util::generator_init(void)
{
    AST_Generator *gen = 0;
    ACE_NEW_RETURN(gen,AST_Generator, 0);
    return gen;
}

void
be_util::prep_be_arg(char *in)
{
  ACE_UNUSED_ARG(in);
  //ACE_DEBUG((LM_INFO, "BE Prep Arg %s\n", in));
}


void
be_util::arg_post_proc(void)
{
    //ACE_DEBUG((LM_INFO, "BE Arg Post_Proc\n"));
}
