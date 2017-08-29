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

#ifndef DAF_WFMOSIGNALREACTOR_H
#define DAF_WFMOSIGNALREACTOR_H


#include "DAF.h"
#include "TaskExecutor.h"

#include <ace/Reactor.h>
#include <ace/Singleton.h>

namespace DAF
{
    /** @class WFMOSignalReactor
    * @brief Brief \todo{Fill this in}
    *
    * Details \todo{Detailed description}
    */
    class DAF_Export WFMOSignalReactor : public ACE_Reactor
        , public DAF::TaskExecutor
    {
        using ACE_Reactor::close; // Scope this to private - Ambiguity with ACE_Task_Base

    public:

        /** \todo{Fill this in} */
        enum {
            DEFAULT_THREAD_COUNT = 5
        };

        /** \todo{Fill this in} */
        WFMOSignalReactor(void);
        /** \todo{Fill this in} */
        virtual ~WFMOSignalReactor(void);

        /** \todo{Fill this in} */
        virtual int run(size_t thr_count, bool wait_completion = false);

    protected:

        /** \todo{Fill this in} */
        virtual int close(u_long flags);

        /** \todo{Fill this in} */
        virtual int svc(void);

    protected:

        /// Terminates object when dynamic unlinking occurs.
        virtual int fini(void);
    };

    /** @struct WFMOSignalReactorSingleton
    * @brief Brief \todo{Fill this in}
    *
    * Details \todo{Detailed description}
    */
    struct DAF_Export WFMOSignalReactorSingleton : WFMOSignalReactor
    {
        /** \todo{Fill this in} */
        WFMOSignalReactorSingleton(void);

        /** \todo{Fill this in} */
        const ACE_TCHAR *dll_name(void) const
        {
            return DAF_DLL_NAME;
        }

        /** \todo{Fill this in} */
        const ACE_TCHAR *name(void) const
        {
            return typeid(*this).name();
        }
    };

} // namespace DAF

typedef DAF::WFMOSignalReactorSingleton DAF_WFMOSignalReactorSingleton;
typedef ACE_DLL_Singleton_T<DAF_WFMOSignalReactorSingleton, ACE_SYNCH_MUTEX>    SingletonWFMOSignalReactor;

// This is needed to get only one of these defined across a set of DLLs and EXE
DAF_SINGLETON_DECLARE(ACE_DLL_Singleton_T, DAF_WFMOSignalReactorSingleton, ACE_SYNCH_MUTEX);

#endif // DAF_WFMOSIGNALREACTOR_H
