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
#ifndef TAF_CDR_H
#define TAF_CDR_H

#include "TAF.h"

#include <tao/CDR.h>

namespace TAF {

    struct TAF_Export OutputCDR : TAO_OutputCDR {
        OutputCDR(size_t size = 0, int byte_order = ACE_CDR::BYTE_ORDER_BIG_ENDIAN);
        OutputCDR(char *data, size_t size, int byte_order = ACE_CDR::BYTE_ORDER_BIG_ENDIAN);

        template <typename T> bool write(const T &t);

        size_t  size(void) const
        {
            return this->total_length();
        }

        size_t  copy_buffer(char *buf_ptr, size_t buf_length) const;

        // TODO: Need further specializations here (i.e. const char * etc)
    };

    template <typename T> bool OutputCDR::write(const T &t)
    {
        size_t j = sizeof(T);  // How Big is the type.

        if (this->good_bit()) for (char *s = 0; this->adjust(j--, s) == 0;) {
            *reinterpret_cast<T*>(s) = t;
            if (this->do_byte_swap()) {
                for (size_t i = 0; j > i; i++, j--) {
                    s[i] ^= s[j];
                    s[j] ^= s[i];
                    s[i] ^= s[j];
                }
            }
            return true;
        }
        return false;
    }

    struct TAF_Export InputCDR : TAO_InputCDR {
        InputCDR(size_t size = 0, int byte_order = ACE_CDR::BYTE_ORDER_BIG_ENDIAN);
        InputCDR(const char *data, size_t size, int byte_order = ACE_CDR::BYTE_ORDER_BIG_ENDIAN);
        InputCDR(const TAO_OutputCDR &cdr);

        template <typename T> bool read(T &t);
    };

    template <typename T> bool InputCDR::read(T &t)
    {
        size_t j = sizeof(T);  // How Big is the type.

        if (this->good_bit()) for (char *s = 0; this->adjust(j--, s) == 0;) {
            t = *reinterpret_cast<T*>(s);
            if (this->do_byte_swap()) {
                s = reinterpret_cast<char*>(&t);
                for (size_t i = 0; j > i; i++, j--) {
                    s[i] ^= s[j];
                    s[j] ^= s[i];
                    s[i] ^= s[j];
                }
            }
            return true;
        }
        return false;
    }
}

#endif
