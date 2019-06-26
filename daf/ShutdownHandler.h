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
#ifndef DAF_SHUTDOWNHANDLER_H
#define DAF_SHUTDOWNHANDLER_H

#include "DAF.h"

#include <ace/Event_Handler.h>
#include <ace/Signal.h>

namespace DAF
{
    /**
     * \class ShutdownHandler
     * \brief Shutdown hook for process control
     *
     * This class is used to control the teardown of a process and TAF level
     * infrastructure. It hooks the ACE_Reactor signal handler to detect Ctrl-C,
     * kill signals (SIGINT) and supply a central point of control for DAF + TAF libraries.
     * It also provides a single point of access to send a shutdown state to the
     * application. Multiple threads and wait on the state of the shutdown handler.
     */
    class DAF_Export ShutdownHandler : protected ACE_Event_Handler
    {
    public:
        ShutdownHandler();
        virtual ~ShutdownHandler();
        /// Indicates if the shutdown state has been signaled.
        static bool has_shutdown();
        /// Used by a thread to block on the shutdown signal.
        static int  wait_shutdown(const ACE_Time_Value* abs_timeout = NULL); // Absolute Time (nullptr => INFINITE)
        /// Send a shutdown state to the shutdown handler.
        static int  send_shutdown(bool send_state = true);

    protected:
        /// ACE_Reactor callback method on signal
        virtual int handle_signal(int sig, siginfo_t* sig_info, ucontext_t* sig_cxt);
        virtual int handle_shutdown(int signal = 0);

    private:
        ACE_Sig_Set sigs_;

        // = Prevent assignment and initialization.
        ACE_UNIMPLEMENTED_FUNC(void operator = (const ShutdownHandler &))
        ACE_UNIMPLEMENTED_FUNC(ShutdownHandler(const ShutdownHandler &))
    };

} // namespace DAF

#endif // DAF_SHUTDOWNHANDLER_H
