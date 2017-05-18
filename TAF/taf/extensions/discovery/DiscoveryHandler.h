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
#ifndef TAF_DISCOVERYHANDLER_H
#define TAF_DISCOVERYHANDLER_H

#include "TAFDiscovery_export.h"

#include <taf/TAF.h>
#include <taf/IORQueryServant.h>

#include <ace/CDR_Base.h>
#include <ace/INET_Addr.h>
#include <ace/SOCK_Dgram_Mcast.h>

#include "DiscoveryC.h"

namespace TAF
{
    namespace {
        const size_t MAX_IORQUERY_REPLIES = TAO_DEFAULT_OBJECT_REF_TABLE_SIZE;
        const size_t MIN_IOR_REPLY_LENGTH = ACE_CDR::MAX_ALIGNMENT;
        const size_t MAX_IOR_REPLY_LENGTH = 4096;
    }

    class TAFDiscovery_Export IORQueryReplyHandler
    {
    public:

        IORQueryReplyHandler(void);
        virtual ~IORQueryReplyHandler(void);

        const ACE_INET_Addr &   getReplyAddress(void) const;
        u_short                 getReplyPort(void) const;

        taf::IORReplySeq_var    getIORReply(const ACE_Time_Value &timeout = ACE_Time_Value(TAO_DEFAULT_SERVICE_RESOLUTION_TIMEOUT));

    private:

        ACE_SOCK_Dgram  udpSock_;
        ACE_INET_Addr   replyAddress_;
    };

    class TAFDiscovery_Export DiscoveryHandler : public ACE_SOCK_Dgram_Mcast
    {
        int mcast_ttl_;

    public:

        DiscoveryHandler(int time_to_live = 1);
        DiscoveryHandler(const ACE_INET_Addr&, int time_to_live = 1);
        DiscoveryHandler(const ACE_TCHAR *mcast_addr, int time_to_live = 1);

        virtual ~DiscoveryHandler(void);

        int open_handler(const ACE_TCHAR *mcast_addr, bool reuse_addr = true);
        int open_handler(const ACE_INET_Addr &mcast_addr, const ACE_TCHAR *net_if = 0, bool reuse_addr = true);

        int getTTL(void) const  { return this->mcast_ttl_;  }

        int sendIORQuery(const IORQueryReplyHandler&, const ACE_TCHAR *id);
    };

} // namespace TAF

extern TAFDiscovery_Export const std::string & TAF_DEFAULT_DISCOVERY_ENDPOINT;

typedef class TAF::DiscoveryHandler     TAFDiscoveryHandler;

#endif /* TAF_DISCOVERYHANDLER_H */
