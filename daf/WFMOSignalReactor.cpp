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
#define DAF_WFMOSIGNALREACTOR_CPP

#include "daf/DAF.h"
#include "daf/DAFDebug.h"
#include "daf/WFMOSignalReactor.h"

#if defined(ACE_WIN32)
#include <ace/WFMO_Reactor.h>
#else
#include <ace/TP_Reactor.h>
#endif

#include <ace/Log_Msg.h>

namespace  //anonymous
{
#if defined (ACE_WIN32)
    struct WFMO_Reactor : ACE_WFMO_Reactor {
        virtual int expire_timers(void) {
            for (ACE_Timer_Queue *tq = this->timer_queue(); tq;) {
                return (this->deactivated() ? 0 : tq->expire()); // Allow any thread to dispatch timers
            }
            return 0;
        }
    };
#else
    typedef class ACE_TP_Reactor WFMO_Reactor;
#endif
} //namespace anonymous

namespace DAF
{
    WFMOSignalReactor::WFMOSignalReactor(void)
        : ACE_Reactor(new WFMO_Reactor(), true)
    {
    }

    WFMOSignalReactor::~WFMOSignalReactor(void)
    {
        this->module_closed(); this->wait();
    }

    int
    WFMOSignalReactor::run(size_t thr_count, bool wait_completion)
    {
        if (this->initialized()) {
            if (this->execute(thr_count = ace_range(size_t(1), size_t(100), thr_count ), true) == -1) {
                ACE_ERROR_RETURN((LM_ERROR,
                    ACE_TEXT("DAF (%04P | %04t) ERROR: Failed to activate SignalReactor threads.\n")), -1);
            } else if (DAF::debug() > 1) {
                ACE_DEBUG((LM_INFO,
                    ACE_TEXT("DAF (%04P | %04t) INFO: SignalReactor started with %u threads.\n"), thr_count));
            }
        } else {
            ACE_ERROR_RETURN((LM_ERROR,
                ACE_TEXT("DAF (%04P | %04t) ERROR: SignalReactor - Not Initialized.\n")), -1);
        }

        return wait_completion ? this->wait() : 0;
    }

    int
    WFMOSignalReactor::svc(void)
    {
        while (this->isAvailable()) {
            return this->run_reactor_event_loop();
        }
        return -1;
    }

    int
    WFMOSignalReactor::close(u_long flags)
    {
        if (flags) {
            if (this->implementation()->deactivated() || this->end_reactor_event_loop() == 0) {
                DAF::TaskExecutor::close(flags);
            }
            return this->implementation()->close();
        }

        return DAF::TaskExecutor::close(flags);
    }

    int
    WFMOSignalReactor::fini(void)
    {
        return this->module_closed();
    }

    /**************************************************************************/

    WFMOSignalReactorSingleton::WFMOSignalReactorSingleton(void) : WFMOSignalReactor()
    {
        this->run(DEFAULT_THREAD_COUNT, false);
    }

} // namespace DAF
