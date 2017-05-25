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
#define TTY_STREAM_CPP

#include "TTY_Stream.h"

// This only works on Win32 platforms and on Unix platforms supporting POSIX aio calls.
#if defined (ACE_HAS_WIN32_OVERLAPPED_IO) || defined (ACE_HAS_AIO_CALLS)

#include "TTY_Handler.h"
#include "TTY_Device.h"

#include <iostream>

namespace DEV
{
    TTY_Stream::TTY_Stream(size_t size)
        : buffer_       (new char[size + 1])
        , b_cond_       (b_lock_)
        , b_sema_       (1)
        , stream_active_(true)
        , size_         (size + 1)
        , head_         (0)
        , tail_         (0)
    {
    }

    TTY_Stream::~TTY_Stream(void)
    {
    }

    int
    TTY_Stream::is_full(void) const
    {
        return (((this->head_ + this->size_ + 1) % this->size_) == this->tail_);
    }

    int
    TTY_Stream::is_empty(void) const
    {
        return (this->head_ == this->tail_);
    }

    size_t
    TTY_Stream::size(void) const
    {
        return this->size_ - 1;
    }

    size_t
    TTY_Stream::length(void) const
    {
        return (this->is_empty() ? 0 : (((this->size_ + this->head_) - this->tail_) % this->size_));
    }

    char *
    TTY_Stream::ptr(size_t indx) const
    {
        return &(this->buffer_.get()[indx]);
    }

    size_t
    TTY_Stream::put_bump(size_t len) // Called With Locks Held
    {
        int put_len = 0;

        if (len) while (this->is_full() ? false : len--) {
            this->head_ = ((this->head_ + 1) % this->size_); put_len++;
        }

        this->b_cond_.signal(); return put_len;
    }

    size_t
    TTY_Stream::get_bump(size_t len) // Called With Locks Held
    {
        size_t get_len = 0;

        if (len) while (this->is_empty() ? false : len--) {
            this->tail_ = ((this->tail_ + 1) % this->size_); get_len++;
        }

        this->b_cond_.signal(); return get_len;
    }

    int
    TTY_Stream::find(const char &c, int off) const
    {
        size_t tail = this->tail_; // Take A copy of tail

        for (int pos = 0; tail != this->head_; pos++) {
            if (off > 0) {
                off -= 1;
            } else if (*this->ptr(tail) == c) {
                return pos;
            }
            tail = ((tail + 1) % this->size_);  // Move Forward
        }

        return -1;
    }

    int
    TTY_Stream::put(const char iov_buf[], int iov_len)  // Not Locked So Caller Locks
    {
        int put_len = 0;

        if (iov_buf && iov_len > 0) {
            size_t head = this->head_;

            for (int io_len = 0; iov_len > 0; iov_len -= io_len) {
                const int blen = int(this->size_ - head);

                if ((io_len = int(this->tail_ - head - 1)) == 0) {
                    break; // Full
                } else if (io_len > 0 || (io_len += int(this->size_)) > 0) {
                    io_len = ace_min(iov_len, io_len, blen);

                    DAF_OS::memcpy(this->ptr(head), &iov_buf[put_len], io_len);
                    {
                        head = ((head + io_len) % this->size_); // Same as put_bump()
                    }

                    iov_len -= io_len;
                    put_len += io_len;
                }
            }
        }

        return put_len;
    }

    int
    TTY_Stream::get(char iov_buf[], int iov_len) const  // Not Locked So Caller Locks
    {
        int get_len = 0;

        if (iov_buf && iov_len > 0) {
            size_t tail = this->tail_;

            for (int io_len = 0; iov_len > 0; iov_len -= io_len) {
                const int blen = int(this->size_ - tail);

                if ((io_len = int(this->head_ - tail)) == 0) {
                    break;
                } else if (io_len > 0 || (io_len += int(this->size_)) > 0) {
                    io_len = ace_min(iov_len, io_len, blen);

                    DAF_OS::memcpy(&iov_buf[get_len], this->ptr(tail), io_len);
                    {
                        tail = ((tail + io_len) % this->size_); // Same as get_bump()
                    }

                    iov_len -= io_len;
                    get_len += io_len;
                }
            }
        }

        return get_len;
    }

    int
    TTY_Stream::recv_notify(size_t len)
    {
        if (len) {
//          ACE_DEBUG((LM_DEBUG, ACE_TEXT(">>RX-Notify Length=%d\n"), len));
            this->put_bump(len);
        }
        this->b_sema_.release(); return 0;
    }

    int
    TTY_Stream::send_notify(size_t len)
    {
        if (len) {
//          ACE_DEBUG((LM_DEBUG, ACE_TEXT(">>TX-Notify Length=%d\n"), len));
            this->get_bump(len);
        }
        this->b_sema_.release(); return 0;
    }

    int
    TTY_Stream::signal_writer(void) // Locks Held on Entry
    {
        if (this->is_empty() || this->b_sema_.tryacquire()) {
            return 0;
        } else if (this->is_empty()) { // Now We have permission to proceed
            this->head_ = this->tail_ = 0;
        }

        const size_t head = this->head_, tail = this->tail_, blen = this->size_ - tail;

        int io_len = int(head - tail);

        if (this->is_active() && (io_len > 0 || (io_len += int(this->size_)) > 0)) {
            size_t len = ace_min(size_t(io_len), blen, this->size() / 2);

             // Used to debug troublesome circular buffer arithmetic (smiley)
             //char s[128]; DAF_OS::sprintf(s, "Writer: len=%04d;io_len=%04d;head=%04d;tail=%04d;blen=%04d"
             //       , len, io_len, head, tail, blen); std::cout << s << std::endl;

            ACE_Message_Block mb_data(this->ptr(tail), len); mb_data.wr_ptr(len);

//          ACE_DEBUG((LM_DEBUG, ACE_TEXT(">>TX-Send Length=%d\n"), len));

            if (len && this->send_stream(mb_data) == 0) {
                DAF_OS::thr_yield(); return 0;
            }

            DAF::print_last_error();
        }

        this->b_sema_.release(); return -1;
    }

    int
    TTY_Stream::signal_reader(void) // Locks Held on Entry
    {
        if (this->is_full() || this->b_sema_.tryacquire()) {
            return 0;
        } else if (this->is_empty()) {
            this->head_ = this->tail_ = 0;
        }

        const size_t head = this->head_, tail = this->tail_, blen = this->size_ - head;

        int io_len = int(tail - head - 1);

        if (this->is_active() && (io_len > 0 || (io_len += int(this->size_)) > 0)) {
            size_t len = ace_min(size_t(io_len), blen, this->size() / 2);

             // Used to debug troublesome circular buffer arithmetic (smiley)
             //char s[128]; DAF_OS::sprintf(s, "Reader: len=%04d;io_len=%04d;head=%04d;tail=%04d;blen=%04d"
             //       , len, io_len, head, tail, blen); std::cout << s << std::endl;

            ACE_Message_Block mb_data(this->ptr(head), len);

//          ACE_DEBUG((LM_DEBUG, ACE_TEXT(">>RX-Read Length=%d\n"), len));

            if (len && this->recv_stream(mb_data) == 0) {  // Start Next Read.
                DAF_OS::thr_yield(); return 0;
            }

            DAF::print_last_error();
        }

        this->b_sema_.release(); return -1;
    }

    int
    TTY_Stream::close_stream(bool cancel_io)
    {
        ACE_UNUSED_ARG(cancel_io); this->stream_active_ = false;
        {
            ACE_GUARD_RETURN(ACE_SYNCH_MUTEX, mon, *this, -1); this->b_cond_.signal();
        }
        return 0;
    }
} // namespace DEV

// This only works on Win32 platforms and on Unix platforms supporting POSIX aio calls.
#endif // defined (ACE_HAS_WIN32_OVERLAPPED_IO) || defined (ACE_HAS_AIO_CALLS)
