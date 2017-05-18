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
#define DEV_TTY_HANDLER_CPP

#include "TTY_Handler.h"

// This only works on Win32 platforms and on Unix platforms supporting POSIX aio calls.
#if defined (ACE_HAS_WIN32_OVERLAPPED_IO) || defined (ACE_HAS_AIO_CALLS)

#include "TTY_Device.h"

const int PROACTOR_RUN_RETRY = 2;

namespace DEV
{
    TTY_Handler::TTY_Handler(void) : ACE_Handler()
        , tty_device    (0)
        , handler_active(false)
        , write_stream  (TTY_Stream::DEFAULT_TXSTREAM_SIZE)
        , read_stream   (TTY_Stream::DEFAULT_RXSTREAM_SIZE)
    {
        this->proactor(&this->tty_proactor);
    }

    TTY_Handler::~TTY_Handler(void)
    {
        this->module_closed();
    }

    ACE_HANDLE
    TTY_Handler::get_handle(void) const
    {
        return ACE_Handler::handle();
    }

    TTY_Device *
    TTY_Handler::get_device(void) const
    {
        return this->tty_device;
    }

    int
    TTY_Handler::open(void *args)
    {
        this->tty_device = reinterpret_cast<TTY_Device*>(args);

        this->handle(this->get_device()->get_handle());

        if (this->write_stream.open_stream(*this) != -1) {
            if (this->read_stream.open_stream(*this) != -1) {
                this->handler_active = true; return this->execute(1);
            }
        }

        return -1;
    }

    int
    TTY_Handler::close(u_long flags)
    {
        if (flags) {
            this->handler_active = false;

            this->read_stream.close_stream(true);
            this->write_stream.close_stream(true);

            for (ACE_Proactor *p_actor = this->proactor(); p_actor;) {
                p_actor->proactor_end_event_loop(); break;
            }
        }

        return DAF::TaskExecutor::close(flags);
    }

    void
    TTY_Handler::handle_read_stream(const ACE_Asynch_Read_Stream::Result &result)
    {
        if (result.success() && this->is_active()) try {
            ACE_GUARD(ACE_SYNCH_MUTEX, mon, this->read_stream); // Only One Buffer Update at a time
            if (this->read_stream.recv_notify(result.bytes_transferred())) {
                return;
            }
            this->process_input(result.handle());
            this->read_stream.signal_reader();
        } DAF_CATCH_ALL { /* Catch Any Errors and Ignore */ }
    }

    void
    TTY_Handler::handle_read_file(const ACE_Asynch_Read_File::Result &result)
    {
        if (result.success() && this->is_active()) try {
            ACE_GUARD(ACE_SYNCH_MUTEX, mon, this->read_stream); // Only One Buffer Update at a time
            if (this->read_stream.recv_notify(result.bytes_transferred())) {
                return;
            }
            this->process_input(result.handle());
            this->read_stream.signal_reader();
        } DAF_CATCH_ALL { /* Catch Any Errors and Ignore */ }
    }

    void
    TTY_Handler::handle_write_stream(const ACE_Asynch_Write_Stream::Result &result)
    {
        if (result.success() && this->is_active()) try {
            ACE_GUARD(ACE_SYNCH_MUTEX, mon, this->write_stream); // Only One Process Update at a time
            if (this->write_stream.send_notify(result.bytes_transferred())) {
                return;
            }
            this->process_output(result.handle());
            this->write_stream.signal_writer();
        } DAF_CATCH_ALL { /* Catch Any Errors and Ignore */ }
    }

    void
    TTY_Handler::handle_write_file(const ACE_Asynch_Write_File::Result &result)
    {
        if (result.success() && this->is_active()) try {
            ACE_GUARD(ACE_SYNCH_MUTEX, mon, this->write_stream); // Only One Process Update at a time
            if (this->write_stream.send_notify(result.bytes_transferred())) {
                return;
            }
            this->process_output(result.handle());
            this->write_stream.signal_writer();
        } DAF_CATCH_ALL { /* Catch Any Errors and Ignore */ }
    }

    int
    TTY_Handler::svc(void)
    {
        this->handler_active = true;

        for (int i = 0; i < PROACTOR_RUN_RETRY; i++) try {
            if (this->is_active()) {
                this->proactor()->proactor_run_event_loop();
            } break;
        } DAF_CATCH_ALL {
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("ERROR: TTY_Handler::svc - unexpected exit #%d - Ignored.\n"), i+1));
        }

        this->handler_active = false; return 0;
    }

    ssize_t
    TTY_Handler::write_data(const char *ptr, int len, int timeout)
    {
        return this->write_stream.write_data(ptr, len, timeout);
    }
} // namespace DEV

#endif // defined (ACE_HAS_WIN32_OVERLAPPED_IO) || defined (ACE_HAS_AIO_CALLS)
