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
#ifndef DAF_DIRECTEXECUTOR_H
#define DAF_DIRECTEXECUTOR_H

/**

*
* @file     DirectExecutor.h
* @author   Derek Dominish
* @author   $LastChangedBy$
* @date     1st September 2011
* @version  $Revision$
* @ingroup
* @namespace DAF
*/

#include "Executor.h"

namespace DAF
{
    /**
     * \struct DirectExecutor
     * \brief An implementation of executor that invokes the run method
     * of the supplied command then returns.
     *
     * This class has no thread management associated with it.
     *
     * ATTRIBUTION: Doug Lee Based On 'Concurrency Patterns in Java'
     * \ingroup executor
     */
    struct DAF_Export DirectExecutor : DAF::Executor {

        virtual int execute(const DAF::Runnable_ref &command) {
            return (DAF::is_nil(command) ? 0 : command->run());
        }
    };
} // namespace DAF

#endif
