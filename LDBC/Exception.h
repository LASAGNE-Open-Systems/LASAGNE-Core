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
#ifndef LDBC_EXCEPTION_H
#define LDBC_EXCEPTION_H

#include <daf/Exception.h>

namespace LDBC {

    struct Exception : std::runtime_error {
        Exception(const char *_msg = typeid(Exception).name())
            : std::runtime_error(_msg) {
        }
    };

    struct InternalException : Exception {
        InternalException(const char *_msg = typeid(InternalException).name())
            : Exception(_msg) {
        }
    };

    struct InvalidConfiguration : Exception {
        InvalidConfiguration(const char *_msg = typeid(InvalidConfiguration).name())
            : Exception(_msg) {
        }
    };

    struct InitializationException : Exception {
        InitializationException(const char *_msg = typeid(InitializationException).name())
            : Exception(_msg) {
        }
    };

    struct IllegalArgumentException : Exception {
        IllegalArgumentException(const char *_msg = typeid(IllegalArgumentException).name())
            : Exception(_msg) {
        }
    };

    struct IllegalUseException : Exception {
        IllegalUseException(const char *_msg = typeid(IllegalUseException).name())
            : Exception(_msg) {
        }
    };

    struct RecordNotFound : Exception {
        RecordNotFound(const char *_msg = typeid(RecordNotFound).name())
            : Exception(_msg) {
        }
    };

    struct IndexOutOfRange : Exception {
        IndexOutOfRange(const char *_msg = typeid(IndexOutOfRange).name())
            : Exception(_msg) {
        }
    };

}   // namespace LDBC

#endif // LDBC_EXCEPTION_H
