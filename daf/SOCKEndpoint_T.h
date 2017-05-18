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
#ifndef DAF_SOCKENDPOINT_T_H
#define DAF_SOCKENDPOINT_T_H

/**
* @file     SOCKEndpoint_T.h
* @author   Derek Dominish
* @author   $LastChangedBy$
* @date     1st June 2013
* @version  $Revision$
* @ingroup
* @namespace DAF
*/

#include <ace/Synch.h>
#include <ace/Synch_Traits.h>
#include <ace/Guard_T.h>

#include <string>

namespace DAF
{
    /** @class SOCKEndpoint
    *@brief Brief \todo{Fill this in}
    *
    * Details \todo{Detailed description}
    */
    template <typename T> class SOCKEndpoint : public T
    {
        ACE_SYNCH_MUTEX sendLock_;

    public:

        /** \todo{Fill this in} */
        virtual ~SOCKEndpoint(void)
        {
            this->close();
        }

        /** \todo{Fill this in} */
        friend DAF::SOCKEndpoint<T>& operator << (DAF::SOCKEndpoint<T> &s, const std::string &frame)
        {
            for (int len = int(frame.size()); len > 0;) {
                ACE_GUARD_RETURN(ACE_SYNCH_MUTEX, daf_mon, s.sendLock_, s); s.send(frame.data(), len); break;
            }
            return s;
        }
    };

} // namespace DAF

#endif // DAF_SOCKENDPOINT_T_H
