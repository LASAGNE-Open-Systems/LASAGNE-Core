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
#include "daf/SignalHandler.h"
#include "daf/ShutdownHandler.h"
#include "daf/WFMOSignalReactor.h"

#include <ace/High_Res_Timer.h>

#include <iostream>

#define SIGNAL_HANDLER_MAX  31 /* Prime Number */
#define SIGNAL_THREADS_MAX  5

static ACE_Atomic_Op<ACE_SYNCH_MUTEX, int> thread_id(1);

class MySignaller : public ACE_Task_Base
{
    volatile bool active_;

    bool    is_active(void) const
    {
        return this->active_;
    }

    class MySignalHandler : public DAF::SignalHandler {
        int ident_;
        ACE_High_Res_Timer  timer_;
        ACE_Semaphore       lock_;
    public:
        int signal(void);

        MySignalHandler(int ident) : ident_(ident) {}
        virtual int handle_event(int sig, siginfo_t*, ucontext_t*);
    };

    DAF::SignalHandler_ref handler_[SIGNAL_HANDLER_MAX];

public:

    MySignaller(void) : active_(true)  { this->init(0,0); }

    ~MySignaller(void) { this->fini(); }

protected:

    virtual int close(u_long flags);

protected:

    virtual int init(int argc, ACE_TCHAR *argv[]);

    virtual int fini(void);

private:

    virtual int svc(void);
};

int
MySignaller::MySignalHandler::signal(void)
{
    lock_.acquire();
    this->timer_.start();
    return DAF::SignalHandler::signal();
}

int
MySignaller::MySignalHandler::handle_event(int sig, siginfo_t*, ucontext_t*)
{
    this->timer_.stop();
    {
        char s[128]; DAF_OS::sprintf(s, "(%u | %04u) Handler= %02d, Signal=%d "
            , static_cast<int>(DAF_OS::getpid())
            , static_cast<int>(DAF_OS::thread_ID())
            , this->ident_
            , sig);

        this->timer_.print_ave(s,1);
    }
    lock_.release();
    return 0;

    //ACE_ERROR_RETURN((LM_INFO,
    //    ACE_TEXT("(%P | %04t) Handler= %02d, Signal=%d,Time=%D\n")
    //    , this->ident_,sig),0);
}

int
MySignaller::init(int argc, ACE_TCHAR *argv[])
{
    ACE_UNUSED_ARG(argc); ACE_UNUSED_ARG(argv);
    for(int i = 0; i < SIGNAL_HANDLER_MAX; i++) {
        this->handler_[i] = new MySignalHandler(i);
    }
    return this->activate(THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED, SIGNAL_THREADS_MAX, true);
}

int
MySignaller::fini(void)
{
    return this->module_closed();
}

int
MySignaller::close(u_long flags)
{
    if (flags) {
        this->active_ = false; this->wait();
    }
    return ACE_Task_Base::close(flags);
}

int
MySignaller::svc(void)
{
    int th_ident = thread_id++;

    for(int id = th_ident; this->is_active(); id += th_ident) {
        this->handler_[id %= SIGNAL_HANDLER_MAX]->signal(); DAF_OS::sleep(ACE_Time_Value(0,1000));
    }

    return 0;
}

int main(int argc, char *argv[])
{
    ACE_UNUSED_ARG(argc); ACE_UNUSED_ARG(argv);

    DAF::ShutdownHandler shutdown_;

    MySignaller signaller; ACE_UNUSED_ARG(signaller);

    return shutdown_.wait_shutdown();
}
