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
#ifndef DAF_SIGNALHANDLER_H
#define DAF_SIGNALHANDLER_H

/**
* @file     SignalHandler.h
* @author   Derek Dominish
* @author   $LastChangedBy$
* @date     1st September 2011
* @version  $Revision$
* @ingroup
* @namespace DAF
*/

#include "DAF.h"

#include "Event_Handler.h"

#include <ace/Reactor.h>
#include <ace/Auto_Event.h>

namespace DAF
{
    class DAF_Export SignalHandler;  // Forward Declaration

    template <> inline void
    ObjectRefTraits<DAF::SignalHandler>::release(DAF::SignalHandler *&p);
    template <> inline DAF::SignalHandler *
    ObjectRefTraits<DAF::SignalHandler>::duplicate(const DAF::SignalHandler *p);

    /**
     * @class SignalHandler
     *
     * @brief Handler Class for ACE_Reactors to signal off soft events.
     *
     * This class can be used as a Event Handler for a ACE_Reactor. Particularly
     * designed to be used with DAF::WFMOSignalReactor. The event can be triggered
     * by SignalHandler::signal() and callbacked via SignalHandler::handle_event (pure virtual).
     *
     * Under Windows it uses the ACE_Auto_Event implementation to provide signalling.
     * Under Linux when DAF_HAS_EVENTFD is available eventfd is used as a IO file descriptor
     * for the ACE_HANDLE in registration.
     *
     * This class will perform automatic registration with the Reactor upon construction.
     *
     * \sa DAF::WFMOSignalReactor ACE_Auto_Event DAF::Event_Handler
     *
     */
    class DAF_Export SignalHandler : protected DAF::Event_Handler
        , ACE_Auto_Event
    {
        /// Windows ACE_Auto_Event handler
        virtual int handle_signal(int sig, siginfo_t *siginfo, ucontext_t *sigcxt = 0);

        /// Linux eventfd read handler
        virtual int handle_input(ACE_HANDLE fd);

        virtual int handle_shutdown(int sig = 0);

        /// Linux eventfd File descriptor.
        ACE_HANDLE fd_;

    public:
        /**
         * Constructor will perform automatic registration with the
         * DAF::WFMOSignalReactor Singleton.
         *
         * @param initial_state is passed down to ACE_Auto_Event construction
         *
         * \sa DAF::WFMOSignalReactor
         */
        SignalHandler(bool initial_state = false);

        /**
         * This constructor will perform automatic registration
         *
         * @param reactor the ACE_Reactor to register the handle with.
         * @param initial_state is passed down to ACE_Auto_Event construction
         */
        SignalHandler(ACE_Reactor *reactor, bool initial_state = false);

        virtual ~SignalHandler(void);

        /**
         * Multi-platform support for Handle.
         * For Windows this calls down to ACE_Event::handle()
         * For Linux with DAF_HAS_EVENTFD it writes gives the eventfd.
         *
         * \sa ACE_Event_Handler::get_handle
         */
        virtual ACE_HANDLE get_handle(void) const;

        /**
         * Multi-platform support Auto_Event.
         * For Windows this passes down to ACE_Auto_Event::signal();
         * For Linux with DAF_HAS_EVENTFD it writes to the eventfd.
         *
         * This method is used to signal the underlying reactor that
         * an event has occurred.
         *
         * @note Method from private ACE_Auto_Event inheritance, made virtual.
         *
         * \sa SignalHandler::handle_event
         */
        virtual int signal(void);

        /**
        * 1) sleep till the event becomes signaled
        *    event resets wait() completes.
        * 2) sleep till the event becomes signaled
        *    or until absolute time-of-day
        *    (use_absolute_time is non-0, else it is relative time).
        *
        * @note Method from private ACE_Auto_Event, made virtual. Beware
        * of usage.
        *
        * \sa ACE_Event_Base::wait
        */
        virtual int wait(void);

        /**
         *  wakeup one waiting thread (if present) and reset event
         * \sa ACE_Base_Event::pulse
         */
        virtual int pulse(void);

        /**
         * Set to nonsignaled state.
         *
         * \sa ACE_Base_Event::reset()
         */
        virtual int reset(void);

        /**
            * Schedule a timer event.
            *
            * Schedule a timer event that will expire after an @a delay amount
            * of time.  The return value of this method, a timer_id value,
            * uniquely identifies the @a event_handler in the ACE_Reactor's
            * internal list of timers.  This timer_id value can be used to
            * cancel the timer with the cancel_timer() call.
            *
            * @param arg           Argument passed to the handle_timeout() method of
            *                      event_handler.
            * @param delay         Time interval after which the timer will expire.
            * @param interval      Time interval for which the timer will be
            *                      automatically rescheduled if the handle_timeout()
            *                      callback does not return a value less than 0.
            *
            * @retval              timer id, on success. The id can be used to
            *                      cancel or reschedule this timer.
            * @retval              -1 on failure, with errno set.
            *
            * \sa ACE_Reactor::schedule_timer
            */
        long    schedule_timer( const void *args,
                                const ACE_Time_Value &delay,
                                const ACE_Time_Value &interval = ACE_Time_Value::zero);

        /**
            * Reset recurring timer interval.
            *
            * Resets the interval of the timer represented by @a timer_id to
            * @a interval, which is specified in relative time to the current
            * gettimeofday().  If @a interval is equal to
            * ACE_Time_Value::zero, the timer will become a non-rescheduling
            * timer.  Returns 0 if successful, -1 if not.
            *
            * This change will not take effect until the next timeout.
            */
        int     reset_timer_interval(long id, const ACE_Time_Value &interval);

        /**
            * Cancel timer.
            *
            * Cancel timer associated with @a timer_id that was returned from
            * the schedule_timer() method.  If arg is non-NULL then it will be
            * set to point to the ``magic cookie'' argument passed in when the
            * handler was registered.  This makes it possible to free up the
            * memory and avoid memory leaks.  Returns 1 if cancellation
            * succeeded and 0 if the @a timer_id wasn't found.
            *
            * On successful cancellation, ACE_Event_Handler::handle_close()
            * will be called with ACE_Event_Handler::TIMER_MASK.
            */
        int     cancel_timer(long id);

        /**
            * Cancel all timers associated with this handler.
            *
            * Shorthand for calling cancel_timer(long,const void **,int)
            * multiple times for all timer associated with @a event_handler.
            *
            * handle_close() will be called with ACE_Event_Handler::TIMER_MASK
            * number of timers associated with the event handler.
            *
            * Returns number of handlers cancelled.
            */
        int     cancel_all_timers(void);

    public:

    DAF_DEFINE_REFCOUNTABLE(SignalHandler);

    protected:

        /**
         * User callback hook. Upon receipt of a signal via the reactor using
         * SignalHandler::signal(). This method is called for user implementation
         *
         * \sa SignalHandler::signal()
         */
        virtual int handle_event(int sig, siginfo_t*, ucontext_t*) = 0;

        /**
        * Called when timer expires.  @a current_time represents the current
        * time that the Event_Handler was selected for timeout
        * dispatching and @a act is the asynchronous completion token that
        * was passed in when <schedule_timer> was invoked.
        */
        virtual int handle_timeout(const ACE_Time_Value &current_time, const void *act);

    private:

        // = Prevent assignment and initialization.
        ACE_UNIMPLEMENTED_FUNC(void operator = (const SignalHandler &))
        ACE_UNIMPLEMENTED_FUNC(SignalHandler(const SignalHandler &))
    };

    template <> inline void
    ObjectRefTraits<DAF::SignalHandler>::release(DAF::SignalHandler *&p)
    {
        if (p) { p->remove_reference(); p = ObjectRefTraits<DAF::SignalHandler>::nil(); }
    }

    template <> inline DAF::SignalHandler *
    ObjectRefTraits<DAF::SignalHandler>::duplicate(const DAF::SignalHandler *p)
    {
        if (p) { const_cast<DAF::SignalHandler*>(p)->add_reference(); } return const_cast<DAF::SignalHandler*>(p);
    }

    DAF_DECLARE_REFCOUNTABLE(SignalHandler);
} // namespace DAF

#endif  // DAF_SIGNALHANDLER_H
