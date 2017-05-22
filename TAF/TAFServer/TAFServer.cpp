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
#define TAFSERVER_CPP

#include <taf/TAFServer.h>

#include <daf/ShutdownHandler.h>

#if defined(TAF_HAS_EXTENSIONS)
# include <taf/extensions/TAFExtensions.h>
#endif

namespace { // Ensure We Register a shutdown Handler for this exe
    const DAF::ShutdownHandler shutdown_handler; DAF_UNUSED_STATIC(shutdown_handler); // Stop Warning
}

int main(int argc, char *argv[])
{
    try {
        return TAFServer(argc, argv).run(true);
    } catch (const CORBA::Exception &ex) {
        ex._tao_print_exception("TAFServer - exiting");
    } DAF_CATCH_ALL {
    }

    ACE_ERROR_RETURN((LM_ERROR,
        ACE_TEXT("TAFServer (%P | %t) ERROR: Encountered a runtime error - exiting.")), -1);
}
