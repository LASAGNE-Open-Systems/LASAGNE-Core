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

// This "Test" is about being able to create and destroy the TAFServer multiple times in the same process space
// its a test-bed for troubleshooting issues surrounding
// - Singleton creation/destruction.
// - Making sure the TAFServer cleans up properly.

#define RELOAD_COUNT 4

int main(int argc, char *argv[])
{
    TAFServerLoader taf_loader;

    for (int i=0 ; i < RELOAD_COUNT; ++i) try {

        std::cout << "Loading TAFServer " << i << "..." << std::endl;

        if (taf_loader.init(argc, argv)) {
            std::cout << "Failed in init()" << i << std::endl;
            break;
        }

        if (taf_loader.wait_completion(3)) {
            std::cout << "Failed in wait_completion()" << i << std::endl;
            break;
        }

        if (taf_loader.fini()) {
            std::cout << "Failed in fini()" << i << std::endl;
            break;
        }
    }
    catch (const char *s) {
        if (s) { std::cout << s << std::endl; } return -1;
    }
    catch (...) {
        std::cout << "We Broke!!" << std::endl; return -1;
    }


    return 0;
}
