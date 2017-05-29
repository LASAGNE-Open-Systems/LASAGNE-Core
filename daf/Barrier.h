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
#ifndef DAF_BARRIER_H
#define DAF_BARRIER_H

#include "DAF.h"
#include "Runnable.h"
#include "Semaphore.h"


namespace DAF
{
/** @class Barrier
    *@brief A Barrier for a fixed number of threads with an all-or-none breakage model.
    *
    * A Barrier is a reasonable choice for a barrier in
    * contexts involving a fixed sized group of threads that
    * must occasionally wait for each other.
    * (A Rendezvous better handles applications in which
    * any number of threads meet, n-at-a-time.)
    *
    * Barriers use an all-or-none breakage model
    * for failed synchronization attempts: If threads
    * leave a barrier point prematurely because of timeout
    * or interruption, others will also leave abnormally
    * (via BrokenBarrierException), until
    * the barrier is <code>restart</code>ed. This is usually
    * the simplest and best strategy for sharing knowledge
    * about failures among cooperating threads in the most
    * common usages contexts of Barriers.
    * This implementation  has the property that interruptions
    * among newly arriving threads can cause as-yet-unresumed
    * threads from a previous barrier cycle to return out
    * as broken. This transmits breakage
    * as early as possible, but with the possible byproduct that
    * only some threads returning out of a barrier will realize
    * that it is newly broken. (Others will not realize this until a
    * future cycle.) (The Rendezvous class has a more uniform, but
    * sometimes less desirable policy.)
    *
    * Barriers support an optional Runnable command
    * that is run once per barrier point.
    *
    * ATTRIBUTION: Doug Lee Based On 'Concurrency Patterns in Java'
    */

    class DAF_Export Barrier : protected DAF::Monitor
    {

    public:

        /**
        * Create a Barrier for the indicated number of parties.
        * and the given command to run at each barrier point.
        * \param parties  The number of threads participating in the barrier
        * \param cmd    The Runnable (Optional - setBarrierCommand) to execute
        *              when the barrier condition is met
        *              (ie when all parties/threads are present)
        */
        Barrier(size_t parties, const DAF::Runnable_ref& cmd= 0);


        virtual ~Barrier(void);

        /**
        * The barrier has been broken and is in an indeterminate state. This can
        * be due to parties leaving the barrier, due to timeout or interruption.
        */
        bool    broken(void) const
        {
            return this->broken_;
        }

        /** \return the number of parties or threads participating */
        size_t  parties(void) const
        {
            return this->parties_;
        }

        /**
        * Set the command to run at the point at which all threads reach the
        * barrier. This command is run exactly once, by the thread
        * that trips the barrier. The command is not run if the barrier is
        * broken.
        * @param cmd the command to run. If null, no command is run.
        */
        void  setBarrierCommand(const Runnable_ref &cmd)
        {
            ACE_Guard<ACE_SYNCH_MUTEX> ace_mon( *this ); this->barrierCommand_ = cmd;
        }

        /**
        * Enter barrier and wait for the other parties()-1 threads.
        * @return the arrival index: the number of other parties
        * that were still waiting
        * upon entry. This is a unique value from zero to parties()-1.
        * If it is zero, then the current
        * thread was the last party to hit barrier point
        * and so was responsible for releasing the others.
        * @exception BrokenBarrierException if any other thread
        * in any previous or current barrier
        * since either creation or the last <code>restart</code>
        * operation left the barrier
        * prematurely due to interruption or time-out. (If so,
        * the <code>broken</code> status is also set.)
        */
        virtual int barrier(void);

        /**
        * Enter barrier and wait at most msecs for the other parties()-1 threads.
        * @return if not timed out, the arrival index: the number of other parties
        * that were still waiting
        * upon entry. This is a unique value from zero to parties()-1.
        * If it is zero, then the current
        * thread was the last party to hit barrier point
        * and so was responsible for releasing the others.
        * @exception BrokenBarrierException
        * if any other thread
        * in any previous or current barrier
        * since either creation or the last <code>restart</code>
        * operation left the barrier
        * prematurely due to interruption or time-out. (If so,
        * the <code>broken</code> status is also set.)
        * @exception TimeoutException if this thread timed out waiting for
        * the barrier. If the timeout occured while already in the
        * barrier, <code>broken</code> status is also set.
        */
        int barrier(time_t msecs);

        /**
        * Wait For the Barrier to reset after a barrier exchange
        * @return indicates <code>true</code> if the barrier was reset
        * ie all threads exited, <code>false</code> if an error occured.
        */
        bool waitReset(time_t msecs = 0);

    private:

        DAF::Semaphore      entryGate_;
        size_t              parties_, synch_, count_, resets_;
        volatile bool       broken_, shutdown_, triggered_;
        DAF::Runnable_ref   barrierCommand_;
    };
} // namespace DAF

#endif // DAF_BARRIER_H
