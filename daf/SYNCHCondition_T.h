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
#ifndef DAF_SYNCHCONDITION_T_H
#define DAF_SYNCHCONDITION_T_H

/**
* @file     SYNCHCondition.h
* @author   Derek Dominish
* @author   $LastChangedBy$
* @date     1st September 2011
* @version  $Revision$
* @ingroup
* @namespace DAF
*/

#include "OS.h"
#include "Exception.h"

#include <ace/Guard_T.h>
#include <ace/Atomic_Op.h>
#include <ace/Synch_Traits.h>
#include <ace/Copy_Disabled.h>
#include <ace/Condition_Thread_Mutex.h>

typedef ACE_SYNCH_MUTEX                 DAF_SYNCH_MUTEX;
typedef ACE_SYNCH_NULL_MUTEX            DAF_SYNCH_NULL_MUTEX;
typedef ACE_SYNCH_RECURSIVE_MUTEX       DAF_SYNCH_RECURSIVE_MUTEX;
typedef ACE_SYNCH_RW_MUTEX              DAF_SYNCH_RW_MUTEX;

namespace DAF
{
    /** @class SYNCHCondition
    * @brief An ACE_Condition<> based implementation that is interruptable
    *
    * ACE_Condition is not Interruptable through program control, so this
    * implementation now keeps track of waiters and therefore interruptable
    * state.  If interrupted, a broadcast is sent to all waiters with the
    * indication that they have been interrupted.
    */
    template <typename T> class SYNCHCondition : ACE_Copy_Disabled
    {
        ACE_Condition<T> condition_mutex_; // Use underlying condition variable emulation

        typedef ACE_Atomic_Op<DAF_SYNCH_MUTEX, int> _waiters_type;

    public:
        /// Define these meta types
        typedef T                   _mutex_type;
        typedef SYNCHCondition<T>   _condition_type;

        /** Constructor - lock Condition into mutex */
        SYNCHCondition(_mutex_type &mutex) : condition_mutex_(mutex)
            , waiters_(0), interrupted_(false)
        {}

        /** Destructor - set condition state to Interrupted */
        ~SYNCHCondition(void)       { this->interrupt(); }

        /** Access the current condition "Interrupted" state */
        int interrupted(void) const { return int(this->interrupted_); }

        /** Access the current condition "waiters" count state */
        int waiters(void)     const { return int(this->waiters_.value()); }

        /** Set the condition to an Interrupted state and notify all possible waiters */
        int interrupt(void);

        /**
        * Block on condition, or until absolute time-of-day has passed.  If
        * abstime == 0 use "blocking" <wait> semantics.  Else, if @a abstime
        * != 0 and the call times out before the condition is signaled
        * <wait> returns -1 and sets errno to ETIME.
        */
        int wait(const ACE_Time_Value *abstime = 0);

        /** Signal one waiting thread. */
        int signal(void)
        {
            return this->condition_mutex_.signal();
        }

        /** Signal *all* waiting threads. */
        int broadcast(void)
        {
            return this->condition_mutex_.broadcast();
        }

    private:

        _waiters_type waiters_;
        volatile bool interrupted_;
    };

    template <typename T> int
    SYNCHCondition<T>::interrupt(void)
    {
        if (this->interrupted() ? this->waiters() > 0 : true) {

            this->interrupted_ = true; DAF_OS::thr_yield(); // Set our flag.

            const int REMOVE_RETRY_MAXIMUM = 3; // Retry's before throw on condition

            /*
            1st shot will notify all waters and hope they get the message.
            2nd shot will get any sneekers that got in during process (we are not locked)
            3rd shot is for good luck
            */

            const ACE_Time_Value remove_delay(0, 5000); // Delay 5ms after the broadcast

            for (int i = REMOVE_RETRY_MAXIMUM; i--; DAF_OS::sleep(remove_delay)) {  // Allow threads to exit wait
                if (this->waiters() > 0) {
                    ACE_GUARD_ACTION(_mutex_type, cond_lock, this->condition_mutex_.mutex(), this->broadcast(), break);
                } else return 0;
            }

            DAF_OS::last_error(EBUSY); return -1;
        }

        return 0;
    }

    template <typename T> int
    SYNCHCondition<T>::wait(const ACE_Time_Value *abstime)
    {
        int result = -1;

        if (!this->interrupted()) try { // Mutex locked by wait_condition caller
            ++this->waiters_; result = this->condition_mutex_.wait(abstime); --this->waiters_;
        } catch (...) { // JB: Deliberate catch(...) - DON'T replace with DAF_CATCH_ALL
            --this->waiters_; throw;
        }

        if (this->interrupted()) {
            DAF_OS::last_error(EINTR); DAF_THROW_EXCEPTION(DAF::InterruptedException);
        }

        return result;
    }

}
/// define these condition types dependant on mutex type
typedef DAF::SYNCHCondition<DAF_SYNCH_MUTEX>            DAF_SYNCH_CONDITION;
typedef DAF::SYNCHCondition<DAF_SYNCH_NULL_MUTEX>       DAF_SYNCH_NULL_CONDITION;
typedef DAF::SYNCHCondition<DAF_SYNCH_RECURSIVE_MUTEX>  DAF_SYNCH_RECURSIVE_CONDITION;

#endif // DAF_SYNCHCONDITION_T_H
