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
#define DAF_SIGNALHANDLER_CPP


#include "SignalHandler.h"
#include "WFMOSignalReactor.h"

#if defined(DAF_HAS_EVENTFD) && (DAF_HAS_EVENTFD == 1)
# include "sys/eventfd.h"
#endif

namespace DAF
{
    namespace
    {
        ACE_HANDLE init_eventfd(void)
        {
#if defined(ACE_WIN32)
#elif defined(DAF_HAS_EVENTFD) && (DAF_HAS_EVENTFD == 1)
# if defined (DAF_LACKS_EFD_SEMAPHORE)
            // TO-DO: This should be checked to ensure that the desired behaviour is still achieved
            ACE_HANDLE fd = ACE_HANDLE(::eventfd(0, EFD_NONBLOCK));
# else
            ACE_HANDLE fd = ACE_HANDLE(::eventfd(0, EFD_NONBLOCK | EFD_SEMAPHORE));
# endif
            if (fd != ACE_INVALID_HANDLE) {
                return fd;
            }
            ACE_DEBUG((LM_ERROR,
                ACE_TEXT("FAILED to acquire Kernel eventfd (errno %d)\n")
                , DAF_OS::last_error()));
#endif
            return ACE_INVALID_HANDLE;
        }

    }//namespace anonymous

    SignalHandler::SignalHandler(bool initial_state) : ACE_Auto_Event(initial_state)
        , fd_(init_eventfd())
    {
#if defined(DAF_HAS_EVENTFD) && (DAF_HAS_EVENTFD == 1)
        if (this->fd_ == ACE_INVALID_HANDLE) {
            throw DAF::InitializationException("Unable to acquire handle through Kernal ::eventfd");
        }
#endif

        // There are different method signatures for ACE_Reactor::register_handler.
        // For OS Events under Windows we need to use
        //
        // ACE_Reactor::register_handler(ACE_Event_Handler *, ACE_HANDLE )
        //
        // Under Other systems
        //
        // ACE_Reactor::register_handler(ACE_Event_Handler *, mask )
        //
#if defined(ACE_WIN32)
        if (SingletonWFMOSignalReactor::instance()->register_handler(this, this->get_handle())) {
#else
        if (SingletonWFMOSignalReactor::instance()->register_handler(this, ACE_Event_Handler::READ_MASK)) {
#endif
            throw DAF::InitializationException("Unable to register SignalHandler with Reactor");
        }

    }

    SignalHandler::SignalHandler(ACE_Reactor *reactor, bool initial_state) : ACE_Auto_Event(initial_state)
        , fd_(init_eventfd())
    {
#if defined(DAF_HAS_EVENTFD) && (DAF_HAS_EVENTFD == 1)
        if (this->fd_ == ACE_INVALID_HANDLE) {
            throw DAF::InitializationException("Unable to acquire handle through Kernal ::eventfd");
        }
#endif

#if defined(ACE_WIN32)
        if (reactor ? reactor->register_handler(this, this->get_handle()) : true) {
#else
        if (reactor ? reactor->register_handler(this, ACE_Event_Handler::READ_MASK) : true) {
#endif
            throw DAF::InitializationException("Unable to register SignalHandler with Reactor argument");
        }
    }

    SignalHandler::~SignalHandler(void)
    {
#if defined(DAF_HAS_EVENTFD) && (DAF_HAS_EVENTFD == 1)
        if (this->fd_ != ACE_INVALID_HANDLE) {
            DAF_OS::close(this->fd_);
        }
#endif
        this->cancel_all_timers(); this->reactor()->remove_handler(this, ACE_Event_Handler::DONT_CALL);
    }

    int
    SignalHandler::handle_shutdown(int sig)
    {
        ACE_UNUSED_ARG(sig);
        return -1;  // Causes handle_close to be called
    }

    ACE_HANDLE
    SignalHandler::get_handle(void) const
    {
#if defined(ACE_WIN32)
        return ACE_Auto_Event::handle();
#elif defined(DAF_HAS_EVENTFD) && (DAF_HAS_EVENTFD == 1)
        return this->fd_;
#endif
        return ACE_INVALID_HANDLE;
    }

    int
    SignalHandler::signal(void)
    {
#if defined(ACE_WIN32)
        return ACE_Auto_Event::signal();
#elif defined(DAF_HAS_EVENTFD) && (DAF_HAS_EVENTFD == 1)
        uint64_t input_d = 1;
        if (DAF_OS::write(this->get_handle(), &input_d, sizeof(input_d)) == sizeof(input_d)) {
            return 0;
        }
        ACE_DEBUG((LM_ERROR,
            ACE_TEXT("Failed to write to eventfd %d with (errno %d)\n")
            , this->get_handle(), DAF_OS::last_error()));
#endif
        return -1;
    }

    long
    SignalHandler::schedule_timer(const void *args, const ACE_Time_Value &delay, const ACE_Time_Value &interval)
    {
        return this->reactor()->schedule_timer(this, args, delay, interval);
    }

    int
    SignalHandler::reset_timer_interval(long id, const ACE_Time_Value &interval)
    {
        return this->reactor()->reset_timer_interval(id, interval);
    }

    int
    SignalHandler::cancel_timer(long id)
    {
        return this->reactor()->cancel_timer(id);
    }

    int
    SignalHandler::cancel_all_timers(void)
    {
        return this->reactor()->cancel_timer(this);
    }

    int
    SignalHandler::handle_signal(int sig, siginfo_t *siginfo, ucontext_t *sigcxt)
    {
        if (sig ? false : siginfo != 0) {
#if defined(ACE_WIN32)
          // Temporary WIN32! TODO Work on the Linux Variant.
            if (this->get_handle() == siginfo->si_handle_) try {
                return this->handle_event(sig, siginfo, sigcxt);
            } catch (...) { /* Leave it for default handling */ }
#endif
        } else switch(sig) {
            case SIGINT: return this->handle_shutdown(sig);
        }

        return ACE_Event_Handler::handle_signal(sig, siginfo, sigcxt);
    }

    int
    SignalHandler::handle_input(ACE_HANDLE fd)
    {
        // This shouldn't be used under windows, handle_signal should
        // be called instead via the Reactor.
#if defined(ACE_WIN32)
#elif defined(DAF_HAS_EVENTFD) && (DAF_HAS_EVENTFD == 1)
        if (this->get_handle() == fd) {
            uint64_t output_d = 0;
            if (DAF_OS::read(this->get_handle(), &output_d, sizeof(output_d)) == sizeof(output_d)) try {
                return this->handle_event(fd, 0, 0);
            } catch (...) {
                ACE_DEBUG((LM_ERROR, ACE_TEXT("Failed to read from eventfd %d (errno %d)\n")
                    , this->get_handle(), DAF_OS::last_error()));
                throw;
            }
        }
#endif
        return ACE_Event_Handler::handle_input(fd);
    }

    int
    SignalHandler::handle_event(int sig, siginfo_t*, ucontext_t*)
    {
        ACE_UNUSED_ARG(sig);
        return 0;
    }

    int
    SignalHandler::handle_timeout(const ACE_Time_Value &current_time, const void *act)
    {
        ACE_UNUSED_ARG(current_time); ACE_UNUSED_ARG(act);
        return 0;
    }

    int
    SignalHandler::wait(void)
    {
        return ACE_Auto_Event::wait();
    }

    int
    SignalHandler::pulse(void)
    {
        return ACE_Auto_Event::pulse();
    }

    int
    SignalHandler::reset(void)
    {
        return ACE_Auto_Event::reset();
    }
} // namespace DAF
