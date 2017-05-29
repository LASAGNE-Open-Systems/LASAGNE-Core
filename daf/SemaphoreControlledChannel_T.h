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
#ifndef DAF_SEMAPHORECONTROLLEDCHANNEL_T_H
#define DAF_SEMAPHORECONTROLLEDCHANNEL_T_H

/**
* ATTRIBUTION: Doug Lee Based On 'Concurrency Patterns in Java'
*
* @file     SemaphoreControlledChannel_T.h
* @author   Derek Dominish
* @author   $LastChangedBy$
* @date     1st September 2011
* @version  $Revision$
* @ingroup
* @namespace DAF
*/

#include "Channel_T.h"
#include "Semaphore.h"

namespace DAF
{
    /** @class SemaphoreControlledChannel
    *@brief Brief \todo{Fill this in}
    *
    * Details \todo{Detailed description}
    */
    template <typename T>
    class SemaphoreControlledChannel : public DAF::Channel<T>
    {
        DAF::Semaphore  putGuard_, takeGuard_;

        virtual int insert(const T&) = 0;
        virtual T   extract(void) = 0;

    public:
        /** \todo{Fill this in} */
        SemaphoreControlledChannel(size_t capacity);

        /** \todo{Fill this in} */
        virtual int put(const T&);
        /** \todo{Fill this in} */
        virtual int offer(const T&, time_t msecs = 0);
        /** \todo{Fill this in} */
        virtual T   take(void);
        /** \todo{Fill this in} */
        virtual T   poll(time_t msecs = 0);


        /** \todo{Fill this in} */
        virtual size_t  capacity(void) const;
        /** \todo{Fill this in} */
        virtual size_t  size(void) const;

        /** \todo{Fill this in} */
        virtual int interrupt(void);
  };
} // namespace DAF

#if defined (ACE_TEMPLATES_REQUIRE_SOURCE)
# include "SemaphoreControlledChannel_T.cpp"
#endif /* ACE_TEMPLATES_REQUIRE_SOURCE */

#if defined (ACE_TEMPLATES_REQUIRE_PRAGMA)
# pragma implementation ("SemaphoreControlledChannel_T.cpp")
#endif /* ACE_TEMPLATES_REQUIRE_PRAGMA */

#endif // DAF_SEMAPHORECONTROLLEDCHANNEL_T_H
