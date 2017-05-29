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
#define DAF_SHUTDOWNHANDLER_CPP

#include "ShutdownHandler.h"
#include "Monitor.h"

#include <ace/Reactor.h>

namespace {

    class ShutdownMonitor : public DAF::Monitor
    {
        volatile bool has_shutdown_;

    public:

        ShutdownMonitor(void) : has_shutdown_(false)
        {
        }

        void set_shutdown(bool val)
        {
            this->has_shutdown_ = val;
        }

        bool has_shutdown(void) const
        {
            return this->has_shutdown_;
        }

    } shutdown_monitor_;
}

namespace DAF
{
    ShutdownHandler::ShutdownHandler(int signal) : signal_(signal)
    {
        if (ACE_Reactor::instance()->register_handler(this->signal_, this)) {
            ACE_DEBUG((LM_ERROR,
                ACE_TEXT("(%04P | %04t) ERROR: Unable to register shutdown task signal handler with reactor.\n")));
        }
    }

    ShutdownHandler::~ShutdownHandler(void)
    {
        ACE_Reactor::instance()->remove_handler(this->signal_, (ACE_Sig_Action*)0);
    }

    int
    ShutdownHandler::wait_shutdown(const ACE_Time_Value *abs_timeout)
    {
        if (!ShutdownHandler::has_shutdown()) {
            ACE_GUARD_RETURN(ACE_SYNCH_MUTEX, ace_mon, shutdown_monitor_, -1);
            while (!ShutdownHandler::has_shutdown()) {  // DCL
                if (shutdown_monitor_.wait(abs_timeout) && errno == ETIME) {
                    return -1;
                }
            }
        }
        return 0;
    }

    bool
    ShutdownHandler::has_shutdown(void)
    {
        return shutdown_monitor_.has_shutdown();
    }

    int
    ShutdownHandler::send_shutdown(bool send_state)
    {
        shutdown_monitor_.set_shutdown(send_state);
        {
            ACE_GUARD_RETURN(ACE_SYNCH_MUTEX, ace_mon, shutdown_monitor_, -1); shutdown_monitor_.notifyAll();
        }
        DAF_OS::thr_yield(); return 0;
    }

    int
    ShutdownHandler::handle_shutdown(int signal)
    {
        ACE_UNUSED_ARG(signal); return 0;
    }

    int
    ShutdownHandler::handle_signal(int signal,siginfo_t *sig_info,ucontext_t *sig_cxt)
    {
        if (signal == this->signal_) try { // try for user-code
            if (this->handle_shutdown(signal) == 0) {
                ACE_ERROR_RETURN((LM_INFO, ACE_TEXT("\nINFO: [%D] shutdown signalled.\n"))
                    , (this->send_shutdown(), -1)); // Return -1 to unhook reactor
            }
        } DAF_CATCH_ALL {
            this->send_shutdown(); return -1;
        }

        return ACE_Event_Handler::handle_signal(signal, sig_info, sig_cxt);
    }
} // namespace DAF
