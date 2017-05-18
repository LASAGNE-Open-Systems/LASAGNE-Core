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
#ifndef DEV_TTY_WRITESTREAM_H
#define DEV_TTY_WRITESTREAM_H

#include "TTY_Stream.h"

// This only works on Win32 platforms and on Unix platforms supporting POSIX aio calls.
#if defined (ACE_HAS_WIN32_OVERLAPPED_IO) || defined (ACE_HAS_AIO_CALLS)

namespace DEV
{
    class TTY_Asynch_Export TTY_WriteStream : public ACE_Asynch_Write_File
        , public TTY_Stream
    {
        mutable ACE_SYNCH_MUTEX lock_;      // Reentrancy Lock
        ACE_HANDLE write_handle_;

    public:

        TTY_WriteStream(size_t buff_size = DEFAULT_TXSTREAM_SIZE);
        ~TTY_WriteStream(void);

        virtual int open_stream(ACE_Handler &handler);

        virtual int close_stream(bool cancel_io = true);

        virtual ssize_t write_data(const char iov_buf[], int iov_len, int timeout = 0);

    protected:

        virtual ssize_t send_stream(ACE_Message_Block &msg);
    };
} // namespace DEV

#endif // defined (ACE_HAS_WIN32_OVERLAPPED_IO) || defined (ACE_HAS_AIO_CALLS)

#endif // TTY_WRITESTREAM_H
