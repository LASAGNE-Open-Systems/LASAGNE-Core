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
#ifndef DAF_CHANNEL_T_H
#define DAF_CHANNEL_T_H

#include "DAF.h"
#include "Monitor.h"

/**
* @file     Channel_T.h
* @author
* @author   $LastChangedBy$
* @date
* @version  $Revision$
* @ingroup
* @namespace DAF
*/

namespace DAF
{
    /** @class Channel
    * @brief An interface for managing the putting and taking of objects.
    *
    * A Channel represents anything that you can put items
    * into and take them out of. As with the Sync interface, both
    * blocking (put(x), take), and timeouts (offer(x, ms), poll(ms)) policies
    * are provided. Using a zero timeout for offer and poll results
    * in a pure balking policy.
    *
    * A given channel implementation might or might not have bounded
    * capacity or other insertion constraints, so in general, you cannot tell if
    * a given put will block. However,Channels that are designed to
    * have an element capacity (and so always block when full)
    * should implement the BoundedChannel subinterface.
    *
    * Channels may hold any kind of item. However,
    * insertion of null is not in general supported. Implementations
    * may (all currently do) throw IllegalArgumentExceptions upon attempts to
    * insert null.
    *
    * By design, the Channel interface does not support any methods to determine
    * the current number of elements being held in the channel, or
    * whether it is empty or full. This decision reflects the fact that in
    * concurrent programming, such methods are so rarely useful
    * that including them invites misuse; at best they could
    * provide a snapshot of current
    * state, that could change immediately after being reported.
    * It is better practice to instead use poll and offer to try
    * to take and put elements without blocking. For example,
    * to empty out the current contents of a channel.
    * However, it is possible to determine whether an item
    * exists in a Channel via <code>peek</code>, which returns
    * but does NOT remove the next item that can be taken (or null
    * if there is no such item). The peek operation has a limited
    * range of applicability, and must be used with care. Unless it
    * is known that a given thread is the only possible consumer
    * of a channel, and that no time-out-based <code>offer</code> operations
    * are ever invoked, there is no guarantee that the item returned
    * by peek will be available for a subsequent take.
    *
    * Also, as a compromise, even though it does not appear in interface,
    * implementation classes that can readily compute the number
    * of elements support a <code>size()</code> method. This allows careful
    * use, for example in queue length monitors, appropriate to the
    * particular implementation constraints and properties.
    *
    * All channels allow multiple producers and/or consumers.
    * They do not support any kind of <em>close</em> method
    * to shut down operation or indicate completion of particular
    * producer or consumer threads.
    *
    * In time-out based methods (poll(ms) and offer(x, ms),
    * time bounds are interpreted in
    * a coarse-grained, best-effort fashion. Since there is no
    * way to escape out of a wait for a synchronized
    * method/block, time bounds can sometimes be exceeded when
    * there is a lot contention for the channel. Additionally,
    * some Channel semantics entail a ``point of
    * no return'' where, once some parts of the operation have completed,
    * others must follow, regardless of time bound.
    *
    * Interruptions are in general handled as early as possible
    * in all methods. Normally, InterruptionExceptions are thrown
    * in put/take and offer(msec)/poll(msec) if interruption
    * is detected upon entry to the method, as well as in any
    * later context surrounding waits.
    *
    * If a put returns normally, an offer returns true, or a put or poll
    * returns non-null, the operation completed successfully.
    * In all other cases, the operation fails cleanly -- the
    * element is not put or taken.
    *
    * As with Sync classes, spinloops are not directly supported,
    * are not particularly recommended for routine use, but are not hard
    * to construct.
    * --> Doug Lee
    *
    * @file     Executor.h
    * @author   Derek Dominish
    * @author   $LastChangedBy$
    * @date     1st September 2011
    * @version  $Revision$
    * @ingroup
    */

    template <typename T> class Channel : public DAF::Monitor
    {
    public:

        /**
        *\todo{Fill this in}
        */
        typedef T   _value_type;

        /**
        *\todo{Fill this in}
        */
        virtual ~Channel(void)
        {
        }

        /**
        * Place item in the channel, possibly waiting indefinitely until
        * it can be accepted. Channels implementing the BoundedChannel
        * subinterface are generally guaranteed to block on puts upon
        * reaching capacity, but other implementations may or may not block.
        */
        virtual int put(const T&) = 0;

        /**
        * Place item in channel only if it can be accepted within
        * ms milliseconds. The time bound is interpreted in
        * a coarse-grained, best-effort fashion.
        */
        virtual int offer(const T&, time_t msecs = 0) = 0;

        /**
        * Return and remove an item from channel,
        * possibly waiting indefinitely until
        * such an item exists.
        */
        virtual T   take(void) = 0;

        /**
        * Return and remove an item from channel only if one is available within
        * ms milliseconds. The time bound is interpreted in a coarse
        * grained, best-effort fashion.
        */
        virtual T   poll(time_t msecs = 0) = 0;


        /// Empty the channel.
        virtual void    clear(void)
        {
            try { for (;;) { this->poll(0); } } DAF_CATCH_ALL {}
        }


        /// Return the maximum number of items the channel can hold
        virtual size_t  capacity(void) const = 0;

        /**
        * Return the number of items currently in the channel.
        * This value may change immediately upon return, and therefore
        * is only an instantanous value.
        */
        virtual size_t  size(void) const = 0;

        /// Return the current empty condition of the channel. This value may change immediately upon return, and therefore is only an instantanous value.
        virtual int     empty(void) const
        {
            return this->size() == 0;
        }
    };
} // namespace DAF

#endif // DAF_CHANNEL_T_H
