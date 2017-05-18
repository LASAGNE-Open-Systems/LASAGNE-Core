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
#ifndef DAF_EVENT_HANDLER_H
#define DAF_EVENT_HANDLER_H

#include "RefCount.h"

#include <ace/Event_Handler.h>

/**
* @file     Event_Handler.h
* @author   Derek Dominish
* @author   $LastChangedBy$
* @date     1st September 2011
* @version  $Revision$
* @ingroup
* @namespace DAF
*/

namespace DAF
{
    // Template specializations for ACE_Event_Handler
    // which uses its own API with add_reference() and remove_reference().
    template <> inline ACE_Event_Handler *
    ObjectRefTraits<ACE_Event_Handler>::duplicate(const ACE_Event_Handler *p)
    {
        if (p) { const_cast<ACE_Event_Handler*>(p)->add_reference(); } return const_cast<ACE_Event_Handler*>(p);
    }

    template <> inline void
    ObjectRefTraits<ACE_Event_Handler>::release(ACE_Event_Handler *&p)
    {
        if (p) { p->remove_reference(); p = ObjectRefTraits<ACE_Event_Handler>::nil(); }
    }

    // Forward Declarations
    class Event_Handler;

    template <> inline void
    ObjectRefTraits<Event_Handler>::release(Event_Handler *&p);

    template <> inline Event_Handler *
    ObjectRefTraits<Event_Handler>::duplicate(const Event_Handler *p);

    /**
     * @class Event_Handler
     *
     * @brief DAF::EventHandler with Reference Counting.
     *
     * ACE_Event_Handler by default does not ENABLE reference counting.
     * The DAF::Event_Handler enabled reference counting by default and
     * includes template specializations of the ObjectRefTraits to allow
     * seamless integration with the DAF reference counting code base.
     *
     * \sa ACE_Event_Handler::Reference_Counting_Policy, DAF::ObjectRefTraits, DAF::RefCount
     *
     */
    class Event_Handler : public ACE_Event_Handler
    {
    public:
        /** \todo{Fill this in} */
        Event_Handler(ACE_Reactor *reactor = 0,
                      int priority = ACE_Event_Handler::LO_PRIORITY)
        : ACE_Event_Handler(reactor,priority)
        {
            this->reference_counting_policy().value(ACE_Event_Handler::Reference_Counting_Policy::ENABLED);
        }

        DAF_DEFINE_REFCOUNTABLE(Event_Handler);
    };

    DAF_DECLARE_REFCOUNTABLE(Event_Handler);

    template <> inline void
    ObjectRefTraits<Event_Handler>::release(Event_Handler *&p)
    {
        if (p) {
            p->remove_reference();
            p = ObjectRefTraits<Event_Handler>::nil();
        }
    }

    template <> inline Event_Handler *
    ObjectRefTraits<Event_Handler>::duplicate(const Event_Handler *p)
    {
        if (p) {
            const_cast<Event_Handler*>(p)->add_reference();
        }
        return const_cast<Event_Handler*>(p);
    }
} // namespace DAF

typedef DAF::Event_Handler  DAF_Event_Handler;

#endif
