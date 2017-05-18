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
#ifndef _TAF_IDL_VISITOR_UTILS_H
#define _TAF_IDL_VISITOR_UTILS_H

#include "TAF_IDL_BE_export.h"

#include <ostream>


namespace TAF
{
namespace IDL
{

class TAF_IDL_BE_Export VisitorUtils
{
public:
    static void header_guard_start(std::ostream &out, const std::string &name);

    static void header_guard_end(std::ostream &out, const std::string &name);

    static void data_support_includes(std::ostream &out, const std::string &filename);

    static void data_support_metatype(std::ostream &out, const std::string &type);

    static std::string create_header_id(const std::string &name, bool unique = true);

    static void generation_comment(std::ostream &out, const std::string &generator_name);

    static void taf_dds_headers(std::ostream &out);

    static int char_convert(int in);
};


}//namespace IDL
}//namespace TAF

#endif // _TAF_IDL_VISITOR_UTILS_H
