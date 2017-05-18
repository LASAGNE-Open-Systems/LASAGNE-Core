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
#ifndef DEV_TTY_DEVICE_H
#define DEV_TTY_DEVICE_H

#include "TTY_Asynch_export.h"

#include <daf/DAF.h>
#include <daf/TaskExecutor.h>

// This only works on Win32 platforms and on Unix platforms supporting POSIX aio calls.
#if defined (ACE_HAS_WIN32_OVERLAPPED_IO) || defined (ACE_HAS_AIO_CALLS)

#include <ace/Service_Config.h>
#include <ace/Svc_Handler.h>
#include <ace/Asynch_IO.h>
#include <ace/Proactor.h>
#include <ace/TTY_IO.h>

namespace DEV {
    class TTY_Asynch_Export TTY_Handler;  // Forward Declaration

    class TTY_Asynch_Export TTY_Device : public DAF::TaskExecutor
        , protected ACE_TTY_IO
    {
        TTY_Handler *tty_handler;

        bool delete_handler_;

        volatile bool device_active_;

        int  device_debug_;

    public:

        enum {
            TX_DEVICE_BUFFER = 512, RX_DEVICE_BUFFER = TX_DEVICE_BUFFER * 2
        };

        class ModemStatus
        {
            unsigned int modem_status;

            friend class TTY_Device;

        public:
            unsigned int getStatus(void) const;

            bool CTS(void)  const;
            bool DSR(void)  const;
            bool RING(void) const;
            bool RLSD(void) const;

        protected:
            ModemStatus(unsigned int status);
        };

        TTY_Device(TTY_Handler *handler, bool delete_handler = false);
        virtual ~TTY_Device(void);

        virtual int init(int argc, ACE_TCHAR *argv[]);
        virtual int fini(void)
        {
            return DAF::TaskExecutor::fini();
        }

        virtual int open(void *args = 0);
        virtual int close(u_long flags);

        virtual void        set_handle(ACE_HANDLE);
        virtual ACE_HANDLE  get_handle(void) const;
        virtual const ModemStatus& get_status(void) const;

        virtual ssize_t write_data(const char *ptr, int len, int timeout = 0);

        const char  * get_device_name(void) const
        {
            return this->tty_device_name.c_str();
        }

        TTY_Handler * get_device_handler(void) const
        {
            return this->tty_handler;
        }

        bool    is_active(void) const
        {
            return this->isAvailable() && this->device_active_;
        }

        int     device_debug(void) const
        {
            return this->device_debug_;
        }

    private:

        int parse_args(int &argc, ACE_TCHAR *argv[]);
        int parse_config(const std::string &config);

    protected:

        struct ACE_TTY_IO::Serial_Params tty_params;

        std::string tty_device_config;

    private:
        std::string tty_device_name;
        mutable ModemStatus modem_status_;
    };

    TTY_Asynch_Export std::ostream& operator << (std::ostream &os, const TTY_Device::ModemStatus&);
} /* namespace DEV */

typedef class DEV::TTY_Device   DEV_TTY_DEVICE;

#endif /* ACE_HAS_WIN32_OVERLAPPED_IO || ACE_HAS_AIO_CALLS */

#endif  /* DEV_TTY_DEVICE */
