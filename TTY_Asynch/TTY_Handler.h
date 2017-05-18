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
#ifndef DEV_TTY_HANDLER_H
#define DEV_TTY_HANDLER_H

#include "TTY_Asynch_export.h"

#include "daf/DAF.h"
#include "daf/TaskExecutor.h"

// This only works on Win32 platforms and on Unix platforms supporting POSIX aio calls.
#if defined (ACE_HAS_WIN32_OVERLAPPED_IO) || defined (ACE_HAS_AIO_CALLS)

#include "TTY_ReadStream.h"
#include "TTY_WriteStream.h"

#include "ace/Asynch_IO.h"
#include "ace/Proactor.h"
#include "ace/Synch.h"
#include "ace/Task.h"

namespace DEV {
    class TTY_Asynch_Export TTY_Device; // Forward Declaration

    class TTY_Asynch_Export TTY_Handler : public ACE_Handler
        , public DAF::TaskExecutor
    {
        mutable ACE_SYNCH_MUTEX lock_;

        ACE_Proactor tty_proactor;
        TTY_Device  *tty_device;

        volatile bool handler_active;

        virtual int svc(void);  // Run The Proactor

    public:

        TTY_Handler(void);
        ~TTY_Handler(void);

        virtual int open(void *args);
        virtual int close(u_long flags);

        ACE_HANDLE  get_handle(void) const;
        TTY_Device* get_device(void) const;

        operator ACE_SYNCH_MUTEX & () const
        {
            return this->lock_;
        }

        virtual ssize_t write_data(const char *ptr, int len, int timeout = 0);

        bool    is_active(void) const
        {
            return this->isAvailable() && this->handler_active;
        }

    protected:

        virtual int process_input   (ACE_HANDLE)    { return 0; }
        virtual int process_output  (ACE_HANDLE)    { return 0; }

    protected:

        virtual void handle_read_stream(const ACE_Asynch_Read_Stream::Result&);
        virtual void handle_read_file(const ACE_Asynch_Read_File::Result&);
        virtual void handle_write_stream(const ACE_Asynch_Write_Stream::Result&);
        virtual void handle_write_file(const ACE_Asynch_Write_File::Result&);

        TTY_WriteStream     write_stream;
        TTY_ReadStream      read_stream;
    };
} // namespace DEV

typedef class DEV::TTY_Handler   DEV_TTY_HANDLER;

#endif // defined (ACE_HAS_WIN32_OVERLAPPED_IO) || defined (ACE_HAS_AIO_CALLS)

#endif // DEV_TTY_HANDLER_H
