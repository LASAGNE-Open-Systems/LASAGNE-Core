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

#ifndef LASAGNE_IDL_BE_EXTERN_H
#define LASAGNE_IDL_BE_EXTERN_H

#include "TAF_IDL_BE_export.h"
#include "be_global.h"

extern TAF_IDL_BE_Export BE_GlobalData* be_global;

extern TAF_IDL_BE_Export int BE_init(int&, ACE_TCHAR*[]);
extern TAF_IDL_BE_Export void BE_post_init(char*[], long);
extern TAF_IDL_BE_Export void BE_version();
extern TAF_IDL_BE_Export void BE_produce();
extern TAF_IDL_BE_Export void BE_abort();
extern TAF_IDL_BE_Export void BE_cleanup();

#endif /* LASAGNE_IDL_BE_EXTERN_H */
