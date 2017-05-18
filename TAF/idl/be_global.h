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

#ifndef TAF_IDL_BE_GLOBALS_H
#define TAF_IDL_BE_GLOBALS_H

#include "TAF_IDL_BE_export.h"

#include <string>

class TAF_IDL_BE_Export BE_GlobalData {
public:

    BE_GlobalData(void);
    ~BE_GlobalData(void);

    void destroy(void);

    void parse_args(long&, char **av);

    void output_dir(const char *s);
    const char* output_dir(void) const;


    void corba_compatibility(bool cck);
    bool corba_compatibility(void) const;

    void gen_taf_data_support(bool taf);
    bool gen_taf_data_support(void) const;

    void gen_enum_support(bool taf);
    bool gen_enum_support(void) const;

    int debug(void) const;
    void debug(int in) ;

    const std::string& taf_support_ext(void) const;
    void taf_support_ext(const std::string &);


    const std::string& enum_support_ext(void) const;
    void enum_support_ext(const std::string &);

    const std::string& date_string(void) const;

private:
    std::string output_dir_;
    bool corba_compat_;
    bool taf_support_;
    std::string taf_support_ext_;
    std::string date_string_;
    bool enum_support_;
    std::string enum_support_ext_;
    int debug_;
};


#endif //TAF_IDL_BE_GLOBALS_H
