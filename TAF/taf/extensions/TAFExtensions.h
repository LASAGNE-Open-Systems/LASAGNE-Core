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
#ifndef TAF_EXTENSIONS_H
#define TAF_EXTENSIONS_H

#include "TAFExtensions_export.h"

namespace TAF {
    TAFExtensions_Export int ExtensionsInitializer(void);
}

#if !defined(TAF_EXTENSIONS_CPP)
namespace {  // Stop Warnings
    static int tafExtensionsInitialize = TAF::ExtensionsInitializer(); DAF_UNUSED_STATIC(tafExtensionsInitialize);
}
#endif

#endif
