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
#ifndef DAF_TASKEXECUTOR_H
#define DAF_TASKEXECUTOR_H

/**
* ATTRIBUTION: Doug Lee Based On 'Concurrency Patterns in Java'
*
* @file     TaskExecutor.h
* @author   Derek Dominish
* @author   $LastChangedBy$
* @date     1st September 2011
* @version  $Revision$
* @ingroup
* @namespace DAF
*/

#include "DAF.h"
#include "Executor.h"
#include "SynchronousChannel_T.h"

#include <ace/Task.h>
#include <ace/Singleton.h>
#include <ace/Thread_Manager.h>

namespace DAF
{
    /** @class TaskExecutor
    *@brief A tunable, extensible thread pool class.
    *
    * The main supported public method is execute(Runnable) which can be
    * called instead of directly creating threads to execute commands.
    *
    * Thread pools can be useful for several, usually intertwined reasons:
    * -# To bound resource use.  A limit can be placed on the maximum
    *    number of simultaneously executing threads.
    * -# To manage concurrency levels.  A targeted number of threads
    *    can be allowed to execute simultaneously.
    * -# To manage a set of threads performing related tasks.
    * -# To minimize overhead, by reusing previously constructed
    *    threads, rather than creating new ones.  NOTE: however
    *    thread pools are hardly ever cure-alls for performance
    *    problems associated with thread construction, especially
    *    within environments that themselves pool or recycle threads.
    *
    * These kinds of goals introduce a number of policy parameters
    * that are encapsulated in this implementation.  All of these
    * parameters have defaults and are tunable, either by set/get methods,
    * or, in cases where decisions should hold across lifetimes, via
    * that can be easily overridden in subclasses.  The main, most
    * commonly set parameters can be established in constructors.
    * Policy choices across these dimensions can and do interact.
    *
    * Probably the most common use of pools is in statics or singletons
    * accessible from a number of classes.
    * --> Doug Lee
    */
    class DAF_Export TaskExecutor : public ACE_Task_Base
        , public DAF::Executor // Ensure we extend DAF::Executor abstract base
    {
        using ACE_Task_Base::activate;  // make this private - Use this->execute()
        using ACE_Task_Base::thr_count; // make this private - Use this->size() const
        using ACE_Task_Base::grp_id;    // make this private - Use this->grp_id() const

    public:

        /// default timeout values
        enum {
            THREAD_HANDOFF_TIMEOUT  = time_t(1),    // 1 millisecond
            THREAD_EVICT_TIMEOUT    = time_t(6000), // 6 Seconds
            THREAD_DECAY_TIMEOUT    = time_t(30000) // 30 Seconds
        };

        /** Constructor
        * \param thr_mgr: if 0 then the Singleton thread manager is used
        */
        TaskExecutor(ACE_Thread_Manager * thr_mgr = 0);

        virtual ~TaskExecutor(void);

        // = Active object activation method.
        /**
        * Turn the task into an active object, i.e., having @a n_threads of
        * control, all running at the @a priority level (see below) with the
        * same group ID which is an adaptation of "this". ALL threads invoked
        * through this method invoke <TaskExecutor::svc>.  Returns -1 if
        * failure occurs, returns 1 if Task is already an active object and
        * @a force_active is false (i.e., do *not* create a new thread in
        * this case), and returns 0 if Task was not already an active
        * object and a thread is created successfully or thread is an
        * active object and @a force_active is true.
        *
        * The <{flags}> are a bitwise-OR of the following:
        * = BEGIN<INDENT>
        * THR_CANCEL_DISABLE, THR_CANCEL_ENABLE, THR_CANCEL_DEFERRED,
        * THR_CANCEL_ASYNCHRONOUS, THR_BOUND, THR_NEW_LWP, THR_DETACHED,
        * THR_SUSPENDED, THR_DAEMON, THR_JOINABLE, THR_SCHED_FIFO,
        * THR_SCHED_RR, THR_SCHED_DEFAULT, THR_EXPLICIT_SCHED,
        * THR_SCOPE_SYSTEM, THR_SCOPE_PROCESS
        * = END<INDENT>
        * If THR_SCHED_INHERIT is not desirable, applications should
        * specifically pass in THR_EXPLICIT_SCHED.
        *
        * By default, or if <{priority}> is set to
        * ACE_DEFAULT_THREAD_PRIORITY, an "appropriate" priority value for
        * the given scheduling policy (specified in <{flags}>, e.g.,
        * <THR_SCHED_DEFAULT>) is used.  This value is calculated
        * dynamically, and is the median value between the minimum and
        * maximum priority values for the given policy.  If an explicit
        * value is given, it is used.  Note that actual priority values are
        * EXTREMEMLY implementation-dependent, and are probably best
        * avoided.
        *
        * If @a thread_handles != 0 it is assumed to be an array of @a n
        * thread_handles that will be assigned the values of the thread
        * handles being spawned.  Returns -1 on failure (@c errno will
        * explain...), otherwise returns the group id of the threads.
        *
        * If @a stack != 0 it is assumed to be an array of @a n pointers to
        * the base of the stacks to use for the threads being spawned.
        * Likewise, if @a stack_size != 0 it is assumed to be an array of
        * @a n values indicating how big each of the corresponding @a stacks
        * are.
        */
        virtual int     execute(size_t  nthreads,
                                bool    force_active = true,
                                long    flags = (THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED),
                                long    priority = ACE_DEFAULT_THREAD_PRIORITY,
                                ACE_hthread_t thread_handles[] = 0,
                                void *  stack[] = 0,
                                size_t  stack_size[] = 0,
                                ACE_thread_t thread_ids[] = 0,
                                const char * thr_name[] = 0);

        // = Active object activation method through DAF::Executor abstract interface.
        /**
        * Turn the task into an active object, i.e., having a single thread of
        * control, all running at the priority level set through the DAF::Runnable.
        * All threads created within this TaskExecutor have the same grp_id which
        * is an adaptation of "this", all of which invoke <DAF::Runnable::run>.
        * Returns -1 if handoff failure occurs otherwise 0.
        */
        virtual int     execute(const DAF::Runnable_ref &);

        /// Return the current thread count.  NOTE: support for the DAF::Executor interface
        virtual size_t  size(void) const    { return this->thr_count_;  }

        /// Return the current thread count.  NOTE: ACE_Task_Base::thr_count() causes unnecessary locking;
        size_t  thr_count(void) const       { return this->thr_count_;  }   // ACE_Task_Base interface compatability

        /// Return the current grp_id. NOTE: grp_id is derived from *this*.
        int     grp_id(void) const          { return this->grp_id_;     }   // ACE_Task_Base interface compatability

        /// Return the thread count is greater than zero
        bool    isActive(void) const        { return this->size() > 0;  }

        /// Return the closed state
        bool    isClosed(void) const        { return this->executorClosed_; }

        /**
        * Return the availability status of the TaskExecutor to accept further active
        * object spawn requests through <TaskExecutor::execute>. If <TaskExecutor:module_closed>
        * has been called (typically through destructor / fini) OR if shutdown has been
        * signalled (i.e. DAF::ShutdownHandler::has_shutdown) then the TaskExecutor pool
        * will be marked as not available (i.e. !isAvailable())
        */
        bool    isAvailable(void) const;

        /**
        * Sets the active object thread decay time (milliseconds) that a thread
        * will remain blocked waiting for a re-use activity spawned by
        * <TaskExecutor::execute> of a <DAF::Runnable>. If this timeout is exceeded,
        * the thread will autonomously remove itself from the pool and destroy itself.
        */
        void    setDecayTimeout(time_t decay_timeout);

        /**
        * Accessor to the active object thread decay time (milliseconds).
        */
        time_t  getDecayTimeout(void) const
        {
            return this->decay_timeout_; // milliseconds
        }

        /**
        * Sets the eviction timeout (milliseconds) that a TaskExecutor will wait for
        * its threads to cooperatively remove themselves after the pool has been marked
        * as !isAvailable(). The remaining threads within this thread group will then
        * be forcefully evictied through terminate thread mechanisms.
        */
        void    setEvictTimeout(time_t evict_timeout);

        /**
        * Accessor to the eviction timeout (milliseconds).
        */
        time_t  getEvictTimeout(void) const
        {
            return this->evict_timeout_; // milliseconds
        }

        /**
        * Set the wait timeout for a thread to takeup a DAF::Runnable through
        * <TaskExecutor::execute> before a new thread is force created. This alleviates
        * the possibility of infinate overtaking through thread creation.
        */
        void    setHandoffTimeout(time_t handoff_timemout);

        /**
        * Accessor to the handoff timeout to thread of a DAF::Runnable (milliseconds).
        */
        time_t  getHandoffTimeout(void) const
        {
            return this->handoff_timeout_; // milliseconds
        }

        /**
        * Hook called during ACE_Module::close().  The default
        * implementation calls forwards the call to close(1).  Please
        * notice the changed value of the default argument of close().
        * This allows tasks to differ between the call has been originated
        * from ACE_Thread_Exit or from module_closed().  Be aware that
        * close(0) will be also called when a thread associated with the
        * ACE_Task instance exits.
        */
        virtual int module_closed(void);

    protected:  // Default Service Interface to Not Supported

        virtual int suspend(void)
        {
            ACE_NOTSUP_RETURN(-1);
        }

        virtual int resume(void)
        {
            ACE_NOTSUP_RETURN(-1);
        }

    protected:

        /**
        * Thread svc routine for all *long-lived* threads created and invoked directly
        * through the TaskExecutor::execute()
        */
        virtual int svc(void); // Do Nothing Service Routine

        /**
        * Thread svc routine for all threads created through TaskExecutor::execute() with
        * a DAF::Runnable argument.  This is invoked internally through the worker thread.
        */
        virtual int svc(const DAF::Runnable_ref &);

        /**
        * Handoff a Runnable to possibly an existing (waiting) thread already resident
        * within the pool.  This performs a positive (coupled) handoff to an available thread
        * within the pool if such exists (for upto HandoffTimeout value) otherwise it
        * will create a new pool thread directly to accomplish the handoff.  In this case
        * the new thread will coallesce back into the pool after the DAF::Runnable request
        * has been completed.
        */
        virtual int task_handOff(const DAF::Runnable_ref &);

        /**
        * Dispatch a DAF::Runnable within the context of an existing pool thread.
        * This clears thread state (i.e. errno etc) as well as requested thread prorities
        * to that required by the DAF::Runnable instance being dispatched.
        */
        virtual int task_dispatch(DAF::Runnable_ref = DAF::Runnable::_nil());

    protected:

        /// Thread pool condition - used to notify the eviction process as threads become removed.
        DAF_SYNCH_CONDITION  zero_condition_;

        /**
        * Hook called from ACE_Thread_Exit when during thread exit and from
        * the default implementation of @c module_closed().  In general, this
        * method shouldn't be called directly by an application,
        * particularly if the Task is running as an Active Object.
        * Instead, a special message should be passed into the Task via
        * the put() method defined below, and the svc() method should
        * interpret this as a flag to shut down the Task.
        */
        using ACE_Task_Base::close;

    private:

        /** Worker Task */
        class DAF_Export WorkerExTask : public DAF::Runnable
        {
        public:
            WorkerExTask(ACE_Task_Base *task, const DAF::Runnable_ref &cmd)
                : task_(task), cmd_(cmd) {}
            virtual int run(void);
            ACE_Task_Base * task_base(void) const { return this->task_; }
        private:
            ACE_Task_Base  * task_;
            DAF::Runnable_ref cmd_;
        public:
            DAF_DEFINE_REFCOUNTABLE(WorkerExTask);
        };

        DAF_DECLARE_REFCOUNTABLE(WorkerExTask);

    private:

        /**
        * Trickery with inheritance to allow access to enable the force termination of
        * un-cooperative threads when closing the pool.
        */
        static struct ThreadManager : ACE_Thread_Manager {
            using ACE_Thread_Manager::thread_desc_self;
            int terminate_thr(ACE_Thread_Descriptor *td, int terminate_index = 0);
            int terminate_grp(int grp_id, int terminate_index = 0);
        } threadManager_;

        /// Routine that runs the service execute routine as a daemon thread.
        static  ACE_THR_FUNC_RETURN execute_run(void *workerExTask);
        static  ACE_THR_FUNC_RETURN execute_svc(void *aceTaskBase);

        /// Routine that cleans up the manager hooks and thread state to reflect a thread closing
        static  void cleanup(void *obj, void *args = 0);

    protected:

        /// Positive (coupled) handoff channel for DAF::Runnable to a waiting pool thread.
        DAF::SynchronousChannel<DAF::Runnable_ref>   taskChannel_;

    private:

        time_t  decay_timeout_;  // Decay Time for Threads      (milliseconds)
        time_t  evict_timeout_;  // Time for closing threads    (milliseconds)
        time_t  handoff_timeout_;// Time for handing off to existing threads before creating a new one (milliseconds)

        mutable volatile bool executorAvailable_;

        bool executorClosing_;
        bool executorClosed_;
    };

    /// Execute a DAF::Runnable within the context of the singleton TaskExecutor.
    DAF_Export int SingletonExecute(const DAF::Runnable_ref &command);

} // namespace DAF

#endif // DAF_TASKEXECUTOR_H
