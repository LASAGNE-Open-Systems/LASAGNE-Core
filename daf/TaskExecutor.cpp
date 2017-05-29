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
#define DAF_TASKEXECUTOR_CPP

#include "TaskExecutor.h"
#include "ShutdownHandler.h"
#include "PropertyManager.h"

#if defined (ACE_HAS_SIG_C_FUNC)
extern "C" void DAF_TaskExecutor_cleanup(void *obj, void *args)
{
    DAF::TaskExecutor::cleanup(obj, args);
}
#endif /* ACE_HAS_SIG_C_FUNC */

//#define DAF_USES_TERMINATE_THREAD  /* Breaks ACE_Object_Manager Singleton */

namespace DAF
{
    namespace { // Ananomous

        int make_grp_id(void *p)
        {
            return int(reinterpret_cast<size_t>(p) & (size_t(-1) >> 1));
        }

        struct DAF_Thread_Descriptor : ACE_Thread_Descriptor // Allow Access to thread descriptor bits
        {
            ACE_hthread_t   threadHandle(void)  const   { return this->thr_handle_; }
            long            threadFlags(void)   const   { return this->flags_; }
            ACE_thread_t    threadID(void)      const   { return this->thr_id_; }
            ACE_Task_Base * taskBase(void)      const   { return this->task_; }
            ACE_UINT32 &    threadState(void)           { return this->thr_state_; }
        };

    } // Ananomous

    template <> inline DAF::Runnable_ref
    SynchronousChannel<DAF::Runnable_ref>::extract(void)
    {
        ACE_GUARD_REACTION(ACE_SYNCH_MUTEX, guard, *this, DAF_THROW_EXCEPTION(DAF::ResourceExhaustionException));
        DAF::Runnable_ref t(this->item_._retn()); this->itemTaken_.release();
        if (this->itemError_) {
            this->itemError_ = false; DAF_THROW_EXCEPTION(DAF::InternalException);
        }
        return t._retn();
    }

    /*********************************************************************************/

    ACE_THR_FUNC_RETURN
    TaskExecutor::execute_run(void *args)
    {
        DAF_OS::thread_0_SIGSET_T(); // Ignore all signals to avoid ERROR:

        TaskExecutor::WorkerExTask_ref worker(reinterpret_cast<TaskExecutor::WorkerExTask_ptr>(args));

        // This is exactly the same logics as ACE_Task_Base::svc_run however it works through a svc proxy

        if (worker) for (ACE_Task_Base * task(worker->task_base()); task;) {

            ACE_Thread_Descriptor * td = static_cast<TaskExecutor::ThreadManager *>(task->thr_mgr())->thread_desc_self();

            // Register ourself with our <Thread_Manager>'s thread exit hook
            // mechanism so that our close() hook will be sure to get invoked
            // when this thread exits.

#if defined(ACE_HAS_SIG_C_FUNC)
            td->at_exit(task, DAF_TaskExecutor_cleanup, td);
#else
            td->at_exit(task, DAF::TaskExecutor::cleanup, td);
#endif /* ACE_HAS_SIG_C_FUNC */

            ACE_THR_FUNC_RETURN status = 0;

            try {  // Stop Application code from killing Server

                // Call the Task's run() hook method.
#if defined(ACE_HAS_INTEGRAL_TYPE_THR_FUNC_RETURN)
                status = static_cast<ACE_THR_FUNC_RETURN>(worker->run());
#else
                status = reinterpret_cast<ACE_THR_FUNC_RETURN>(worker->run());
#endif /* ACE_HAS_INTEGRAL_TYPE_THR_FUNC_RETURN */

            } DAF_CATCH_ALL {
                if (ACE::debug() || DAF::debug()) {
                    ACE_DEBUG((LM_ERROR, ACE_TEXT("DAF (%P | %t) TaskExecutor ERROR: ")
                        ACE_TEXT("Unhandled exception caught from execute_run() : TaskExecutor=0x%@.\n"), task));
                }
            }

#if defined(DAF_HANDLES_THREAD_CLEANUP) && (DAF_HANDLES_THREAD_CLEANUP == 1)
            td->at_exit(task, 0, 0); TaskExecutor::cleanup(task, td); // This prevents a second invocation of the cleanup code
#endif
            return status;
        }

        return ACE_THR_FUNC_RETURN(-1);
    }

    ACE_THR_FUNC_RETURN
    TaskExecutor::execute_svc(void *args)
    {
        DAF_OS::thread_0_SIGSET_T(); // Ignore all signals to avoid ERROR:

        for (ACE_Task_Base * task(reinterpret_cast<ACE_Task_Base *>(args)); task;) {

            ACE_Thread_Descriptor * td = static_cast<TaskExecutor::ThreadManager *>(task->thr_mgr())->thread_desc_self();

            // Register ourself with our <Thread_Manager>'s thread exit hook
            // mechanism so that our close() hook will be sure to get invoked
            // when this thread exits.

#if defined(ACE_HAS_SIG_C_FUNC)
            td->at_exit(task, DAF_TaskExecutor_cleanup, td);
#else
            td->at_exit(task, DAF::TaskExecutor::cleanup, td);
#endif /* ACE_HAS_SIG_C_FUNC */

            ACE_THR_FUNC_RETURN status = 0;

            try {  // Stop Application code from killing Server

                // Call the Task's run() hook method.
#if defined(ACE_HAS_INTEGRAL_TYPE_THR_FUNC_RETURN)
                // Reinterpret case between integral types is not mentioned in the C++ spec
                status = static_cast<ACE_THR_FUNC_RETURN>(task->svc());
#else
                status = reinterpret_cast<ACE_THR_FUNC_RETURN>(task->svc());
#endif /* ACE_HAS_INTEGRAL_TYPE_THR_FUNC_RETURN */

            } DAF_CATCH_ALL {
                if (ACE::debug() || DAF::debug()) {
                    ACE_DEBUG((LM_ERROR, ACE_TEXT("DAF (%P | %t) TaskExecutor ERROR: ")
                        ACE_TEXT("Unhandled exception caught from execute_svc() : TaskExecutor=0x%@.\n"), task));
                }
            }

#if defined(DAF_HANDLES_THREAD_CLEANUP) && (DAF_HANDLES_THREAD_CLEANUP == 1)
            td->at_exit(task, 0, 0); TaskExecutor::cleanup(task, td); // This prevents a second invocation of the cleanup code
#endif
            return status;
        }

        return ACE_THR_FUNC_RETURN(-1);
    }

    void
    TaskExecutor::cleanup(void *obj, void *args)
    {
        if (obj) for (TaskExecutor * task = dynamic_cast<TaskExecutor *>(reinterpret_cast<ACE_Task_Base *>(obj)); task;) {

            try { task->close(0); } DAF_CATCH_ALL { /* Calling application code */ }

            ACE_thread_t thr_self = ACE_Thread::self();

            {
                ACE_GUARD_REACTION(ACE_Thread_Mutex, ace_mon, task->lock_, break);

                if (args) for (DAF_Thread_Descriptor * td = reinterpret_cast<DAF_Thread_Descriptor*>(args); td;) {
                    thr_self = td->self();
                    if (DAF::debug() > 2) {
                        ACE_DEBUG((LM_INFO, ACE_TEXT("DAF (%P | %t) TaskExecutor::cleanup; ")
                            ACE_TEXT("grp_id=%d,thr_count=%d,ThreadID=%d[0x%X],ThreadState=0x%X\n")
                            , task->grp_id()
                            , task->thr_count()
                            , unsigned(thr_self), unsigned(thr_self)
                            , unsigned(td->threadState())));
                    }
                    break;
                }

                // Ensure we don't go negative (i.e. maybe because we are terminating threads)
                if (0 == --task->thr_count_) {
                    task->last_thread_id_ = thr_self;
                }

                task->zero_condition_.broadcast(); break; // Must Be Last as task may go away
            }
        }
    }

    /*********************************************************************************/

    TaskExecutor::ThreadManager TaskExecutor::threadManager_;

    TaskExecutor::TaskExecutor(ACE_Thread_Manager * tmgr) : ACE_Task_Base(tmgr)
        , zero_condition_   (this->lock_)
        , decay_timeout_    (THREAD_DECAY_TIMEOUT)
        , evict_timeout_    (THREAD_EVICT_TIMEOUT)
        , handoff_timeout_  (THREAD_HANDOFF_TIMEOUT)
        , executorAvailable_(true)
        , executorClosing_  (false)
        , executorClosed_   (false)
    {
        // Use the ACE_Thread_Manager singleton if we're running as an
        // active object and the caller didn't supply us with a Thread_Manager.
        if (this->thr_mgr_ == 0) {
            this->thr_mgr_ = &threadManager_;
        }

        this->grp_id(make_grp_id(this));

        time_t decay_timeout(DAF::get_numeric_property<time_t>(DAF_TASKDECAYTIMEOUT, THREAD_DECAY_TIMEOUT / DAF_MSECS_ONE_SECOND, true));
        this->setDecayTimeout(decay_timeout * DAF_MSECS_ONE_SECOND);
        time_t evict_timeout(DAF::get_numeric_property<time_t>(DAF_TASKEVICTTIMEOUT, THREAD_EVICT_TIMEOUT / DAF_MSECS_ONE_SECOND, true));
        this->setEvictTimeout(evict_timeout * DAF_MSECS_ONE_SECOND);
        time_t handoff_timeout(DAF::get_numeric_property<time_t>(DAF_TASKHANDOFFTIMEOUT, time_t(THREAD_HANDOFF_TIMEOUT), true));
        this->setHandoffTimeout(handoff_timeout);
    }

    TaskExecutor::~TaskExecutor(void)
    {
        this->module_closed();
    }

    int
    TaskExecutor::svc(void)
    {
        if (DAF::debug() > 1) {
            ACE_DEBUG((LM_WARNING,
                ACE_TEXT("DAF (%P | %t) WARNING: TaskExecutor; Activated with nothing Running.\n")));
        }
        return 0; // Do Nothing
    }

    int
    TaskExecutor::svc(const DAF::Runnable_ref &cmd)
    {
        return (DAF::is_nil(cmd) ? 0 : cmd->run()); // Run the Command
    }

    bool
    TaskExecutor::isAvailable(void) const
    {
        if (this->executorAvailable_ && DAF::ShutdownHandler::has_shutdown()) {
            this->executorAvailable_ = false;
        }
        return this->executorAvailable_;
    }

    void
    TaskExecutor::setDecayTimeout(time_t timeout_milliseconds) // milliseconds
    {
        this->decay_timeout_ = ace_range(DAF_MSECS_ONE_SECOND, DAF_MSECS_ONE_HOUR, timeout_milliseconds);
    }

    void
    TaskExecutor::setEvictTimeout(time_t timeout_milliseconds) // milliseconds
    {
        this->evict_timeout_ = ace_range(DAF_MSECS_ONE_SECOND, DAF_MSECS_ONE_MINUTE, timeout_milliseconds);
    }

    void
    TaskExecutor::setHandoffTimeout(time_t timeout_milliseconds) // milliseconds
    {
        this->handoff_timeout_ = ace_range(time_t(0), time_t(10), timeout_milliseconds);
    }

    int
    TaskExecutor::execute(size_t  n_threads,
        bool    force_active,
        long    flags,
        long    priority,
        ACE_hthread_t thread_handles[],
        void *  stack[],
        size_t  stack_size[],
        ACE_thread_t thread_ids[],
        const char* thr_name[])
    {
        if (this->isAvailable()) {

            ACE_GUARD_RETURN(ACE_Thread_Mutex, ace_mon, this->lock_, -1);

            if (this->isAvailable()) do { // DCL

                if (force_active ? false : this->isActive()) {
                    break; // Already active.
                }

                this->thr_count_ += n_threads;

                int grp_spawned = -1;

                if (thread_ids) {
                    // thread names were specified
                    grp_spawned = this->thr_mgr_->spawn_n(thread_ids, n_threads,
                        &TaskExecutor::execute_svc,
                        this,
                        flags,
                        priority,
                        this->grp_id(),
                        stack,
                        stack_size,
                        thread_handles,
                        this,
                        thr_name);
                }
                else
                {
                    // Thread Ids were not specified
                    grp_spawned = this->thr_mgr_->spawn_n(n_threads,
                        &TaskExecutor::execute_svc,
                        this,
                        flags,
                        priority,
                        this->grp_id(),
                        this,
                        thread_handles,
                        stack,
                        stack_size,
                        thr_name);
                }

                if (grp_spawned == -1) {
                    this->thr_count_ -= n_threads; break;
                }

#if defined(ACE_TANDEM_T1248_PTHREADS)
                DAF_OS::memcpy(&this->last_thread_id_, 0, sizeof(this->last_thread_id_));
#else
                this->last_thread_id_ = 0;    // Reset to prevent inadvertant match on ID
#endif /* defined (ACE_TANDEM_T1248_PTHREADS) */

                DAF_OS::thr_yield(); return 0; // Let The Thread(s) start up (Still Locked though)

            } while (false);
        }

        return -1;
    }

    int
    TaskExecutor::execute(const DAF::Runnable_ref &cmd)
    {
        if (this->isAvailable()) try {

            if (!DAF::is_nil(cmd)) { // Empty command then we are all done!

                // What happens here is we attempt to offer the Runnable to an existing thread
                // We allow up to 'handoff_timeout_' milliseconds before creating a new thread for the pool
                if (this->taskChannel_.offer(cmd,this->handoff_timeout_) && this->task_handOff(cmd)) {
                    throw "Failed-Thread-HandOff";
                }
            }

            return 0;

        } DAF_CATCH_ALL {
            ACE_DEBUG((LM_ERROR,
                ACE_TEXT("DAF (%P | %t) ERROR: TaskExecutor:")
                ACE_TEXT(" Unable to hand-off executable command 0x%@.\n"), cmd.ptr()));
        }

        return -1;
    }

    int
    TaskExecutor::task_dispatch(DAF::Runnable_ref cmd)
    {
        for (const ACE_Sched_Priority default_prio(DAF_OS::thread_PRIORITY()); this->isAvailable();) {

            if (DAF::is_nil(cmd)) try {
                cmd = this->taskChannel_.poll(this->getDecayTimeout())._retn(); continue;
            } catch (const std::runtime_error &) {
                break;
            }

            DAF_OS::thr_setprio(ACE_Sched_Priority(cmd->runPriority())); // Set the requested priority

            DAF_OS::last_error(0); this->svc(cmd._retn()); // Dispatch The Command

            DAF_OS::thr_setprio(default_prio);  // Reset Priority
        }

        cmd = DAF::Runnable::_nil(); return 0;
    }

    int
    TaskExecutor::task_handOff(const DAF::Runnable_ref &cmd)
    {
        if (this->isAvailable()) do {
            {
                ACE_GUARD_REACTION(ACE_Thread_Mutex, ace_mon, this->lock_, break);

                if (this->isAvailable()) { // DCL

                    ++this->thr_count_;

                    WorkerExTask_ptr tp(new WorkerExTask(this, cmd));

                    int grp_spawned = this->thr_mgr()->spawn_n(1
                        , &TaskExecutor::execute_run
                        , tp // Give the cmd to the thread (it will delete)
                        , (THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED)
                        , ACE_DEFAULT_THREAD_PRIORITY
                        , this->grp_id()
                        , this
                    );

                    if (grp_spawned == -1) {
                        delete tp; --this->thr_count_; break; // Clean up command after failed handoff
                    }

#if defined(ACE_TANDEM_T1248_PTHREADS)
                    DAF_OS::memset(&this->last_thread_id_, 0, sizeof(this->last_thread_id_));
#else
                    this->last_thread_id_ = 0;    // Reset to prevent inadvertant match on ID
#endif /* defined (ACE_TANDEM_T1248_PTHREADS) */
                }
            }

            DAF_OS::thr_yield(); return 0; // Let The Thread start up

        } while (false);

        return -1;
    }

    int
    TaskExecutor::module_closed(void)
    {
        const int TASK_EVICT_RETRY_MAXIMUM = 2;

        this->executorAvailable_ = false; // Say we are no longer available

        if (this->isClosed()) {
            return 0;  // Already Done
        }

        if (this->executorClosing_ ? false : this->executorClosing_ = true) {

            if (DAF::debug()) {
                ACE_DEBUG((LM_DEBUG, ACE_TEXT("DAF (%P | %t) TaskExecutor[%@]; ")
                    ACE_TEXT("Signalled to close; grp_id=%d, thr_count=%d.\n")
                    , this, this->grp_id(), this->thr_count()));
            }

            ACE_Task_Base::module_closed(); this->taskChannel_.interrupt(); // Interrupt the dispatching channel
        }

        /*
        * Pass 1: (evict_retry == 2):
        Wait for upto getEvictTimeout() miliseconds for threads contained
        within this TaskExecutor to autonomously exit.

        * Pass 2: (evict_retry == 1):
        Force cancel all non-cooperating threads and wait for upto
        (getEvictTimeout() / 2) miliseconds for threads contained
        within this TaskExecutor to exit.

        * Pass 3: (evict_retry == 0): with
        Force cancel any non-cooperating threads that were in the process of
        starting in step above and now wait for (upto getEvictTimeout() / 4)
        miliseconds for any remaining threads contained within this TaskExecutor
        to exit.

        * Otherwise: give up and return -1;
        */

        time_t evictTimeout = this->getEvictTimeout(); // Start with the full evict timeout value.

        for (int evict_retry = TASK_EVICT_RETRY_MAXIMUM; (DAF_OS::thr_yield(), this->isActive()); evictTimeout /= 2) {

            try {

                ACE_GUARD_REACTION(ACE_Thread_Mutex, mon, this->lock_, break);

                for (const ACE_Time_Value tv(DAF_OS::gettimeofday(evictTimeout)); this->isActive();) {
                    if (this->zero_condition_.wait(&tv) && errno == ETIME) {
                        if (this->isActive()) { // Are we still active?
                            if (evict_retry--) {
                                throw "Threads-Not-Exiting";  // Throw to terminate_grp (release locks)
                            }
                            this->executorClosing_ = false; // No longer closing
                            ACE_ERROR_RETURN((LM_WARNING, ACE_TEXT("DAF (%P | %t) TaskExecutor[%@]: ")
                                ACE_TEXT("Unable to terminate pool threads - grp_id=%d,thr_count=%d.\n")
                                , this, this->grp_id(), this->thr_count()), -1); // We have a problem
                        }
                    }
                }

                this->zero_condition_.broadcast(); break; // Incase we have > 0 module_closed instances (unlikely?)

            } DAF_CATCH_ALL { // Must be called without locks held
                static_cast<ThreadManager *>(this->thr_mgr())->terminate_grp(this->grp_id());
            }
        }

        this->executorClosed_  = true;
        this->executorClosing_ = false;
        return 0;
    }

    /*********************************************************************************/

    int
    TaskExecutor::WorkerExTask::run(void)
    {
        for (TaskExecutor *tex(dynamic_cast<TaskExecutor*>(this->task_base())); tex;) {
            return tex->task_dispatch(this->cmd_._retn());
        }

        this->cmd_ = DAF::Runnable::_nil(); return -1;
    }

    /*********************************************************************************/

    int
    TaskExecutor::ThreadManager::terminate_grp(int grp_id, int arg)
    {
        if (DAF::debug() > 1) {
            ACE_DEBUG((LM_DEBUG,
                ACE_TEXT("DAF (%P | %t) DEBUG: Force therminate threads in group[%d].\n")
                , grp_id));
        }

        return this->apply_grp(grp_id, ACE_THR_MEMBER_FUNC(&ThreadManager::terminate_thr), arg);
    }

    int
    TaskExecutor::ThreadManager::terminate_thr(ACE_Thread_Descriptor *ace_td, int)
    {
        DAF_Thread_Descriptor * td = static_cast<DAF_Thread_Descriptor *>(ace_td);

        if (td) {

            ACE_thread_t thr_id = td->threadID(); td->at_exit(td->taskBase(), 0, 0);

            try {

                if (this->cancel_thr(td, true)) { // Cancel the thread
                    int error = DAF_OS::last_error();
                    switch (error) {
                    case 0: break;
#if defined(ACE_WIN32)
# if 1
                    case ENOTSUP: break; // Just go with *thr_to_be_removed*.
# else
                    case ENOTSUP: // Original code to terminate thread - BIG Sledge hammer!!
                        if (::TerminateThread(td->threadHandle(), DWORD(0xDEAD))) {
                            break;
                        }
                        error = DAF_OS::last_error(); // Fall though with terminate error
# endif
#endif
                    default:
                        if (DAF::debug() > 1) {
                            ACE_DEBUG((LM_DEBUG, ACE_TEXT("DAF (%P | %t) ERROR - TaskExecutor::ThreadManager:\n")
                                ACE_TEXT("\t - Failed to cancel thread; id=%d[0x%X],error=%d[%s]\n")
                                , unsigned(thr_id)
                                , unsigned(thr_id)
                                , error
                                , DAF::last_error_text(error).c_str()));
                        } break;
                    }
                }

            } DAF_CATCH_ALL {
                /* Ignore any Errors - We Are Terminating */
            }

            this->thr_to_be_removed_.enqueue_tail(td);
            TaskExecutor::cleanup(td->taskBase(), td);
        }

        return 0;
    }

    /*********************************************************************************/

    int SingletonExecute(const DAF::Runnable_ref &command)
    {
        static struct _TaskExecutor : DAF::TaskExecutor {
            _TaskExecutor(void) : DAF::TaskExecutor() {
                this->setDecayTimeout(THREAD_DECAY_TIMEOUT / 2);
            }
        } taskExecutor_;

        return taskExecutor_.execute(command);
    }

}  // namespace DAF
