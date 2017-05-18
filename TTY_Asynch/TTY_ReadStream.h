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
#ifndef DEV_TTY_READSTREAM_H
#define DEV_TTY_READSTREAM_H

#include "TTY_Stream.h"

// This only works on Win32 platforms and on Unix platforms supporting POSIX aio calls.
#if defined (ACE_HAS_WIN32_OVERLAPPED_IO) || defined (ACE_HAS_AIO_CALLS)

namespace DEV
{
    class TTY_Asynch_Export TTY_ReadStream : public ACE_Asynch_Read_File
        , public TTY_Stream
    {
        mutable ACE_SYNCH_MUTEX lock_;      // Reentrancy Lock

    public:

        TTY_ReadStream(size_t buff_size = DEFAULT_RXSTREAM_SIZE);
        ~TTY_ReadStream(void);

        virtual int open_stream(ACE_Handler &handler);

        virtual int close_stream(bool cancel_io = true);

        virtual ssize_t read_data(char iov_buf[], int iov_len, int timeout = 0);
        virtual ssize_t peek_data(char iov_buf[], int iov_len);

    protected:

        virtual ssize_t recv_stream(ACE_Message_Block &msg);
    };
} // namespace DEV

#endif // defined (ACE_HAS_WIN32_OVERLAPPED_IO) || defined (ACE_HAS_AIO_CALLS)

#endif // TTY_READSTREAM_H
