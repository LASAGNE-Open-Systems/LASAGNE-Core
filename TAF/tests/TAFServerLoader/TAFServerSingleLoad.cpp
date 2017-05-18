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
#include "TAFServerLoader.cpp"

int main(int argc, char *argv[])
{
    do try {
        TAFServerLoader taf_loader;

        if (taf_loader.init(argc, argv)) {
            std::cout << "Failed in init()" << std::endl;
        }

        if (taf_loader.wait_completion(30)) {
            std::cout << "Failed in wait_completion()" << std::endl;
        } else break;

        if (taf_loader.fini()) {
            std::cout << "Failed in fini()" << std::endl;
        }
    }
    catch (const char *s) {
        if (s) { std::cout << s << std::endl; } return -1;
    }
    catch (...) {
        std::cout << "We Broke!!" << std::endl; return -1;
    }
    while (false);

    return 0;
}
