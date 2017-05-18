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
#ifndef DEV_TTY_STREAM_H
#define DEV_TTY_STREAM_H

#include "TTY_Asynch_export.h"

#include "daf/DAF.h"

#if defined (ACE_HAS_WIN32_OVERLAPPED_IO) || defined (ACE_HAS_AIO_CALLS)

#include "ace/Asynch_IO.h"
#include "ace/Semaphore.h"
#include "ace/Synch_Traits.h"
#include "ace/Message_Block.h"
#include "ace/Condition_Thread_Mutex.h"

namespace DEV
{
    class TTY_Asynch_Export TTY_Stream
    {
        ACE_Auto_Array_Ptr<char> buffer_;

        friend class TTY_Handler;

    public:

        enum {  // USE BUFSIZ but must be same on Linux so set size explicitly
            DEFAULT_TXSTREAM_SIZE   = 512,
            DEFAULT_RXSTREAM_SIZE   = DEFAULT_TXSTREAM_SIZE * 4
        };

        TTY_Stream(size_t buff_size);
        virtual ~TTY_Stream(void) = 0; // Force Inheritance

        int     is_full(void) const;
        int     is_empty(void) const;

        size_t  length(void) const; // Returns the unprocessed available data length.

        size_t  size(void) const;   // Returns the maximum data size of buffer

        virtual int open_stream(ACE_Handler &handler) = 0;  // Force Overload

        virtual int close_stream(bool cancel_io = true);

        virtual int find(const char &c, int off = 0) const;

        virtual int put(const char iov_buf[], int iov_len);
        virtual int get(char iov_buf[], int iov_len) const;

        virtual size_t put_bump(size_t len); // Modifies the head_
        virtual size_t get_bump(size_t len); // Modifies the tail_

        virtual int recv_notify(size_t len);
        virtual int send_notify(size_t len);

        operator ACE_SYNCH_MUTEX & () const
        {
            return this->b_lock_;  // Allow Access to Lock From Protocol
        }

        bool is_active(void) const
        {
            return this->stream_active_;
        }

    protected:

        int signal_writer(void);
        int signal_reader(void);

        char*   ptr(size_t indx = 0) const;

        mutable ACE_SYNCH_MUTEX b_lock_;    // (B)usy   Buffer WaitLock
        ACE_SYNCH_CONDITION     b_cond_;    // (F)ull   Buffer WaitLock
        ACE_Semaphore           b_sema_;    // Lock for starting IO

    private:

        virtual ssize_t send_stream(ACE_Message_Block&)
        {
            errno = EOPNOTSUPP; return -1;  // Unsupported by Default
        }

        virtual ssize_t recv_stream(ACE_Message_Block&)
        {
            errno = EOPNOTSUPP; return -1;  // Unsupported by Default
        }

        bool   stream_active_;

        const size_t    size_;
        volatile size_t head_;
        volatile size_t tail_;
    };
} // namespace DEV

typedef class DEV::TTY_Stream   DEV_TTY_STREAM;

#endif // defined (ACE_HAS_WIN32_OVERLAPPED_IO) || defined (ACE_HAS_AIO_CALLS)
#endif // TTY_STREAM_H
