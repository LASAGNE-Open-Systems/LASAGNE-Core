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
#define DAF_DAFINIT_CPP

#include <ace/OS.h>
#include <ace/Init_ACE.h>
#include <ace/TP_Reactor.h>
#include <ace/Object_Manager.h>
#include <ace/High_Res_Timer.h>

/*
    BEWARE:  Changed filename to _DAFInit.cpp to ensure first in linkage order under VC
    The anonymous namespace below defines a number of Global Variable "Loaders".
    Some items to be aware of are:
    - Runtime Global Variable creation and initialisation is dependent on the dynamic Loader's
      order of the Shared Library load.
    - Runtime Global Variable creation and initialisation within a shared library is dependent on the Compiler/Linker's object
      linking order. Some are alphabetical (VC), some are order of declaration, some are configurable.
    - The order of declaration of Global Variables inside OS.cpp is important in terms of dependency between
      global variables. ie ACE::init -> DAF_Loader (Reactor) -> ....
    - Any future development that makes use of global variables should consider whether
      they should be placed inside OS.cpp to maintain the correct link and load order for DAF. Or consider some other form
      of initialisation. For example, static declaration within a function or using a singleton, which are
      initialised on first usage.
*/
namespace { //anonymous

/*
    If there is no ACE_MAIN then this ACE_OS_Object_Manager will be the instance used by the
    ACE_Object_Manager singleton, otherwise the instance created through ACE_MAIN will be used.
    This allows for the default ACE configuration behaviour of a dynamic ACE_Object Manager.
*/

    struct ACELoader : ACE_OS_Object_Manager
    {
        ACELoader(void)    { ACE::init(); }
        ~ACELoader(void)   { ACE::fini(); }
    };

    /* Initialize the ACE Layer and replace the global Reactor with the ACE_TP_Reactor varient */

    const struct DAFInit : ACELoader
    {
        DAFInit(void) // Set up process wide state.
        {
            ACE_TEST_ASSERT(ACE_Reactor::instance(new ACE_Reactor(new ACE_TP_Reactor(), true), true) == 0);

#if !defined(ACE_WIN32)

            sigset_t RT_signals; sigemptyset(&RT_signals);

            for (int si = ACE_SIGRTMIN; si <= ACE_SIGRTMAX; si++) {
                sigaddset(&RT_signals, si);
            }

            if (ACE_OS::pthread_sigmask(SIG_BLOCK, &RT_signals, 0)) {
                ACELIB_ERROR((LM_ERROR, ACE_TEXT ("Error:(%P | %t):%p\n"), ACE_TEXT ("pthread_sigmask")));
            }

#elif !defined(ACE_HAS_WINCE) // ACE_WIN32 defined

            // ::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOALIGNMENTFAULTEXCEPT | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);

            /*
                SEM_FAILCRITICALERRORS
                The system does not display the critical-error-handler message box. Instead, the system sends the error to the calling process.

                SEM_NOALIGNMENTFAULTEXCEPT

                The system automatically fixes memory alignment faults and makes them invisible to the application.
                It does this for the calling process and any descendant processes.This feature is only supported by certain processor architectures.
                For more information, see the Remarks section.

                After this value is set for a process, subsequent attempts to clear the value are ignored.

                SEM_NOGPFAULTERRORBOX
                The system does not display the Windows Error Reporting dialog.

                SEM_NOOPENFILEERRORBOX
                The system does not display a message box when it fails to find a file.Instead, the error is returned to the calling process.
            */

#endif

            /* Initialize the OS Random Number generator */
            ACE_OS::srand(u_int(ACE_OS::gettimeofday().usec()));

            /* Calibrate the High_Res_Timer */
            ACE_High_Res_Timer::calibrate(250000);
        }

    } DAFInit_;

} //namespace anonymous
