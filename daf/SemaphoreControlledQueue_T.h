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
#ifndef DAF_SEMAPHORECONTROLLEDQUEUE_T_H
#define DAF_SEMAPHORECONTROLLEDQUEUE_T_H

/**
* ATTRIBUTION: Doug Lee Based On 'Concurrency Patterns in Java'
*
* @file     SemaphoreControlledQueue_T.h
* @author   Derek Dominish
* @author   $LastChangedBy$
* @date     1st September 2011
* @version  $Revision$
* @ingroup
* @namespace DAF
*/

#include "SemaphoreControlledChannel_T.h"

#include <list>

namespace DAF
{
    /** @class SemaphoreControlledQueue
    *@brief Brief \todo{Fill this in}
    *
    * Details \todo{Detailed description}
    */
    template <typename T>
    class SemaphoreControlledQueue : public DAF::SemaphoreControlledChannel<T>
    {
    public:

        /** \todo{Fill this in} */
        SemaphoreControlledQueue(size_t capacity);

    protected:

        /** \todo{Fill this in} */
        virtual int insert(const T&);
        /** \todo{Fill this in} */
        virtual T   extract(void);

    private:


        struct _ChannelQ : std::list<T> {
            _ChannelQ(size_t capacity) { ACE_UNUSED_ARG(capacity); }
        } channelQ_;
    };

    /** @class SemaphoreControlledSlot
     *@brief Brief \todo{Fill this in}
     *
     * Details \todo{Detailed description}
     */
    template <typename T>
    struct SemaphoreControlledSlot : SemaphoreControlledQueue<T> {
        SemaphoreControlledSlot(void) : SemaphoreControlledQueue<T>(1) {}
    };

} // namespace DAF

#if defined (ACE_TEMPLATES_REQUIRE_SOURCE)
# include "SemaphoreControlledQueue_T.cpp"
#endif /* ACE_TEMPLATES_REQUIRE_SOURCE */

#if defined (ACE_TEMPLATES_REQUIRE_PRAGMA)
# pragma implementation ("SemaphoreControlledQueue_T.cpp")
#endif /* ACE_TEMPLATES_REQUIRE_PRAGMA */

#endif // DAF_SEMAPHORECONTROLLEDQUEUE_T_H
