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

#include "be_extern.h"
#include "be_global.h"

#include "global_extern.h"
#include "idl_defines.h"

#include "ace/Log_Msg.h"
#include "ace/Date_Time.h"

#include <sstream>


TAF_IDL_BE_Export BE_GlobalData *be_global = 0;

namespace //anonymous
{
    std::string create_date(void)
    {
        std::stringstream stream;
        ACE_Date_Time date(ACE_OS::gettimeofday());

        stream << date.year() << date.month() << date.day() << "_" << date.hour() << date.minute() << date.second();

        return stream.str();
    }
} //namespace anonymous

BE_GlobalData::BE_GlobalData(void)
: output_dir_()
, corba_compat_(false)
, taf_support_(true)
, taf_support_ext_("TAFSupport")
, date_string_(create_date())
, enum_support_(false)
, enum_support_ext_("EnumHelper")
, debug_(0)
{

}

BE_GlobalData::~BE_GlobalData(void)
{

}

void
BE_GlobalData::destroy(void)
{

}

void
BE_GlobalData::output_dir(const char *s)
{
    this->output_dir_ = s;
}

const char*
BE_GlobalData::output_dir(void) const
{
    return this->output_dir_.c_str();
}

void
BE_GlobalData::gen_taf_data_support(bool taf)
{
    this->taf_support_ = taf;
}

bool
BE_GlobalData::gen_taf_data_support(void) const
{
    return this->taf_support_;
}

void
BE_GlobalData::gen_enum_support(bool input)
{
  this->enum_support_ = input;
}

bool
BE_GlobalData::gen_enum_support(void) const
{
  return this->enum_support_;
}


void
BE_GlobalData::parse_args(long &i, char **av)
{
    // NOTE :This is not the same as parse_args for the entire backend, its on a per-file basis

    switch (av[i][1])
    {
        // Directory where all the IDL-Compiler-Generated files are to
        // be kept. Default is the current directory from which the
        // <tao_idl> is called.
        case 'o':
            if (av[i][2] == '\0')
            {
                idl_global->append_idl_flag (av[i + 1]);
                int result = ACE_OS::mkdir (av[i + 1]);

#if !defined (__BORLANDC__)
                if (result != 0 && errno != EEXIST)
#else
                // The Borland RTL doesn't give EEXIST back, only EACCES in case
                // the directory exists, reported to Borland as QC 9495
                if (result != 0 && errno != EEXIST && errno != EACCES)
#endif
                {
                    ACE_ERROR ((
                        LM_ERROR,
                        ACE_TEXT ("IDL: unable to create directory %C")
                        ACE_TEXT (" specified by -o option\n"),
                        av[i + 1]
                    ));

                    break;
                }

                be_global->output_dir (av[i + 1]);
                ++i;
            }
            else
            {
                ACE_ERROR ((
                    LM_ERROR,
                    ACE_TEXT ("IDL: I don't understand")
                    ACE_TEXT (" the '%C' option\n"),
                    av[i]
                ));
            }

        break;

        // TAF Support
        case 'G':
            if (av[i][2] == 't' && av[i][3] == 'a' && av[i][4] == 'f')
            {
                be_global->gen_taf_data_support(true);
            }
        break;

        // CORBA Compatibility -CC
        default:
            ACE_ERROR((LM_ERROR, ACE_TEXT("IDL: LASAGNE BE unknown option '%C'\n"), av[i]));

            idl_global->set_compile_flags(idl_global->compile_flags() | IDL_CF_ONLY_USAGE);
            break;
    }// switch(av[i][1])
}

const std::string&
BE_GlobalData::taf_support_ext(void) const
{
    return this->taf_support_ext_;
}

void
BE_GlobalData::taf_support_ext(const std::string& in)
{
    this->taf_support_ext_ = in;
}

const std::string&
BE_GlobalData::enum_support_ext(void) const
{
  return this->enum_support_ext_;
}

void
BE_GlobalData::enum_support_ext(const std::string& in)
{
  this->enum_support_ext_ = in;
}

const std::string&
BE_GlobalData::date_string(void) const
{
    return this->date_string_;
}

int
BE_GlobalData::debug(void) const
{
  return this->debug_;
}

void
BE_GlobalData::debug(int level)
{
  this->debug_ = level;
}
