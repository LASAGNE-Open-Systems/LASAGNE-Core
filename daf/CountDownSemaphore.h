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
#ifndef DAF_COUNTDOWNSEMAPHORE_H
#define DAF_COUNTDOWNSEMAPHORE_H

/**
* ATTRIBUTION: Doug Lee Based On 'Concurrency Patterns in Java'
*
* @file     CountDownSemaphore.h
* @author   Derek Dominish
* @author   $LastChangedBy$
* @date     1st September 2011
* @version  $Revision$
* @ingroup  \todo{which group?}
* @namespace DAF
*/

#include "Monitor.h"

namespace DAF
{
    /** @class CountDownSemaphore
     *
     * @brief A CountDownSemaphore can serve as a simple one-shot
     * barrier that is initialized with a given count value.
     *
     * Each release decrements the count.
     * All acquires block until the count reaches zero. Upon reaching
     * zero all current acquires are unblocked and all
     * subsequent acquires pass without blocking. This is a one-shot
     * phenomenon -- the count cannot be reset.
     * If you need a version that resets the count, consider
     * using a CyclicBarrier.
     */
    class DAF_Export CountDownSemaphore : DAF::Monitor
    {
        volatile int count_;

    public:

        /** \todo{Fill this in}   */
        CountDownSemaphore(int count);

        /** \todo{Fill this in}   */
        int   count(void) const
        {
            return this->count_;
        }

        /** \todo{Fill this in}   */
        virtual int acquire(void);
        /** \todo{Fill this in}   */
        virtual int attempt(time_t msecs);

        /** \todo{Fill this in}   */
        virtual int release(void);
        /** \todo{Fill this in}   */
        virtual int release(int n);
    };
}  // namespace DAF

#endif // DAF_COUNTDOWNSEMAPHORE_H
