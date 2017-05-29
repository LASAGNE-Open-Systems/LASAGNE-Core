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
#define TAF_CDRSTREAM_CPP

#include "CDRStream.h"
#include "ORBManager.h"

namespace TAF
{
    OutputCDR::OutputCDR(size_t size, int byte_order)
        : TAO_OutputCDR(size, byte_order)
    {
        DAF_OS::memset(this->current()->wr_ptr(),0,this->current()->space());
    }

    OutputCDR::OutputCDR(char *data, size_t size, int byte_order)
        : TAO_OutputCDR(data, size, byte_order)
    {
        DAF_OS::memset(this->current()->wr_ptr(),0,this->current()->space());
    }

    size_t
    OutputCDR::copy_buffer(char *buf_ptr, size_t buf_length) const
    {
        if (buf_ptr && buf_length) {

            size_t mb_len, total_len = 0;

            for (const ACE_Message_Block *mb_ptr = this->begin(); mb_ptr != this->end(); mb_ptr = mb_ptr->cont()) {

                total_len += mb_len = mb_ptr->length();

                if (total_len > buf_length) { // test for overflow before we copy
                    throw DAF::IndexOutOfRange("Insufficient-Buffer-Size");
                }
                else if (mb_len) {
                    DAF_OS::memcpy(buf_ptr, mb_ptr->rd_ptr(), mb_len); buf_ptr += mb_len;
                }
            }

            return total_len;
        }

        return 0;
    }

    InputCDR::InputCDR(size_t size, int byte_order)
        : TAO_InputCDR(size, byte_order, TAO_DEF_GIOP_MAJOR, TAO_DEF_GIOP_MINOR, TheTAFOrbCore())
    {
    }

    InputCDR::InputCDR(const char *data, size_t size, int byte_order)
        : TAO_InputCDR(data, size, byte_order, TAO_DEF_GIOP_MAJOR, TAO_DEF_GIOP_MINOR, TheTAFOrbCore())
    {
    }

    InputCDR::InputCDR(const TAO_OutputCDR &cdr) : TAO_InputCDR(cdr,0,0,0,TheTAFOrbCore())
    {
    }

} // namespace TAF
