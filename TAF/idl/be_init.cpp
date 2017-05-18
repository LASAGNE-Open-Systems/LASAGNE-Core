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

#include "ace/Log_Msg.h"
#include "taf/Version.h"

#include "be_extern.h"
#include "ace/Arg_Shifter.h"
#include "ace/Min_Max.h"

void
BE_version()
{
    ACE_DEBUG((LM_INFO, ACE_TEXT("LASAGNE BE, version ") ACE_TEXT(TAF_VERSION) ACE_TEXT("\n")));
}


int
BE_init(int &argc, ACE_TCHAR *argv[])
{
  // Initialise BE Global Data
  ACE_NEW_RETURN(be_global, BE_GlobalData, -1);

  if (argc )
  {
    for ( ACE_Arg_Shifter arg_shifter(argc, argv);
          arg_shifter.is_anything_left();)
    {
      if ( arg_shifter.is_parameter_next()) {
        // Fall through
      }
      else if ( 0 == arg_shifter.cur_arg_strncasecmp(ACE_TEXT("-notaf")))
      {
          arg_shifter.consume_arg();
          be_global->gen_taf_data_support(false);
          continue;
      }
      else if ( 0 == arg_shifter.cur_arg_strncasecmp(ACE_TEXT("-tafext")))
      {
        arg_shifter.consume_arg();
        if ( arg_shifter.is_parameter_next() )
        {
          be_global->taf_support_ext(arg_shifter.get_current());
          arg_shifter.consume_arg();
        }
        continue;
      }
      else if ( 0 == arg_shifter.cur_arg_strncasecmp(ACE_TEXT("-enum")))
      {
        arg_shifter.consume_arg();
        be_global->gen_enum_support(true);
        continue;
      }
      else if ( 0 == arg_shifter.cur_arg_strncasecmp(ACE_TEXT("-enumext")))
      {
        arg_shifter.consume_arg();
        if ( arg_shifter.is_parameter_next() )
        {
          be_global->enum_support_ext(arg_shifter.get_current());
          arg_shifter.consume_arg();
        }

        continue;
      }
      else if ( 0 == arg_shifter.cur_arg_strncasecmp(ACE_TEXT("--debug")))
      {
        arg_shifter.consume_arg();
        be_global->debug(10);
        //if ( arg_shifter.is_parameter_next() )
        //{
        //  be_global->debug(ace_range(0, 10, //ACE_OS::atoi(arg_shifter.get_current())));
        //}
        continue;
      }

      arg_shifter.ignore_arg();
    }
  }

  return 0;
}

void
BE_post_init(char *[], long input)
{
  ACE_UNUSED_ARG(input);
//    ACE_DEBUG((LM_DEBUG, ACE_TEXT("BE_post_init %d\n"), input));

}
