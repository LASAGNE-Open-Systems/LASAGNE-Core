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

#define TAF_DISCOVERYHANDLER_CPP

#include "DiscoveryHandler.h"

#include <taf/ORBManager.h>
#include <taf/CDRStream.h>

#include <tao/orbconf.h>

namespace {
    const struct DefaultDiscoveryEndpoint : std::string {
        DefaultDiscoveryEndpoint(const char *address, unsigned port) {
            char s[64]; DAF_OS::sprintf(s, ACE_TEXT("%s:%u"), address, port); this->assign(s);
        }
    } DEFAULT_DISCOVERY_ENDPOINT(TAF_DEFAULT_DISCOVERY_ADDRESS, TAF_DEFAULT_DISCOVERY_PORT);
}

TAFDiscovery_Export const std::string & TAF_DEFAULT_DISCOVERY_ENDPOINT(DEFAULT_DISCOVERY_ENDPOINT);

namespace TAF
{
    IORQueryReplyHandler::IORQueryReplyHandler(void)
    {
        // Bind listener to any port and then find out what the port was.
#if defined (ACE_HAS_IPV6)
        if (this->udpSock_.open(ACE_Addr::sap_any, 0, AF_INET6) == -1)  {
#else /* ACE_HAS_IPV6 */
        if (this->udpSock_.open(ACE_Addr::sap_any) == -1) {
#endif /* !ACE_HAS_IPV6 */
            ACE_DEBUG((LM_ERROR, ACE_TEXT ("ERROR: TAF::IORQuery")
                ACE_TEXT(" - Unable to open multicast acceptor.\n")));
        } else if (this->udpSock_.get_local_addr(this->replyAddress_) == -1) {
            ACE_DEBUG((LM_ERROR, ACE_TEXT ("ERROR: TAF::IORQuery")
                ACE_TEXT(" - Unable to get the local multicast address.\n")));
        } else return;

        throw CORBA::BAD_OPERATION();
    }

    IORQueryReplyHandler::~IORQueryReplyHandler(void)
    {
        this->udpSock_.close();
    }

    const ACE_INET_Addr&
    IORQueryReplyHandler::getReplyAddress(void) const
    {
        return this->replyAddress_;
    }

    u_short
    IORQueryReplyHandler::getReplyPort(void) const
    {
        return this->getReplyAddress().get_port_number();
    }

    taf::IORReplySeq_var
    IORQueryReplyHandler::getIORReply(const ACE_Time_Value &timeout)
    {
        taf::IORReplySeq_var ior_seq(new taf::IORReplySeq(MAX_IORQUERY_REPLIES));

        const ACE_Time_Value time_limit(DAF_OS::gettimeofday() + timeout);

        for (CORBA::ULong i = 0; i < CORBA::ULong(MAX_IORQUERY_REPLIES);) {

            ior_seq->length(i); // Reset to current index

            const ACE_Time_Value current_time(DAF_OS::gettimeofday());
            if (current_time >= time_limit) {
                break;
            }

            ACE_CDR::ULong  io_header = 0;
            ACE_INET_Addr   io_address;

            const ACE_Time_Value peek_timeout(time_limit - current_time);

            if (this->udpSock_.recv(&io_header, sizeof(io_header), io_address, MSG_PEEK, &peek_timeout) != sizeof(io_header)) {
                if (errno == ETIME) break; else continue;
            }

            size_t io_len = size_t(ACE_UINT16_MAX & ACE_NTOHL(io_header));

            if (ace_range(TAF::MIN_IOR_REPLY_LENGTH, TAF::MAX_IOR_REPLY_LENGTH, io_len) == io_len) { // Sanity Check

                ACE_Auto_Array_Ptr<char> io_buf(new char[io_len]); // Read Buffer

                char *io_ptr = io_buf.get();

                if (size_t(this->udpSock_.recv(io_ptr, io_len, io_address, 0, &timeout)) != io_len) {
                    if (errno == ETIME) break; else continue;
                }

                TAF::InputCDR io_cdr(io_ptr, io_len);

                ACE_CDR::UShort io_flags, io_length;

                if (io_cdr >> io_flags && io_cdr >> io_length) {
                    if (io_len == size_t(io_length)) {
                        ior_seq->length(i + 1); if (io_cdr >> (*ior_seq)[i]) {
                            i++; // Increment index
                        }
                    }
                }

            }
        }

        return ior_seq._retn();
    }

    /*************************************************************************************/

    DiscoveryHandler::~DiscoveryHandler(void)
    {
        this->close();
    }

    DiscoveryHandler::DiscoveryHandler(int ttl)
      : mcast_ttl_(ttl)
    {
    }

    DiscoveryHandler::DiscoveryHandler(const ACE_TCHAR *mcast_addr, int ttl)
      : mcast_ttl_(ttl)
    {
        if (this->open_handler(mcast_addr)) {
            throw CORBA::BAD_OPERATION();
        }
    }

    DiscoveryHandler::DiscoveryHandler(const ACE_INET_Addr &mcast_addr, int ttl)
      : mcast_ttl_(ttl)
    {
        if (this->open_handler(mcast_addr)) {
            throw CORBA::BAD_OPERATION();
        }
    }

    int
    DiscoveryHandler::open_handler(const ACE_TCHAR *mcast_addr, bool reuse_addr)
    {
        std::string mcast_address(DAF::trim_string(mcast_addr)), mcast_nic;

        int pos = int(mcast_address.find_first_of('@')); if (pos >= 0) {
            std::string real_address(mcast_address.substr(0, pos++));
            mcast_nic.assign(mcast_address.substr(pos));
            if ((pos = int(mcast_nic.find_first_of(':'))) > 0) {
                real_address.append(mcast_nic.substr(pos)); mcast_nic.erase(pos);
            }
            mcast_address.assign(real_address);
        }

        for (ACE_INET_Addr addr; addr.set(mcast_address.c_str()) == 0;) {
            return this->open_handler(addr, mcast_nic.c_str(), reuse_addr);
        }

        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: TAF::IORQueryHandler")
            ACE_TEXT(" - Invalid address '%s' for activation.\n")
            , mcast_address.c_str()), -1);
    }

    int
    DiscoveryHandler::open_handler(const ACE_INET_Addr &mcast_addr, const char *net_if, bool reuse_addr)
    {
        do {
            if (net_if && DAF_OS::strlen(net_if) > 0) {
                if (ACE_SOCK_Dgram_Mcast::join(mcast_addr, reuse_addr, net_if) == 0) {
                    break;
                }
            } else if (ACE_SOCK_Dgram_Mcast::join(mcast_addr, reuse_addr) == 0) {
                break;
            }

            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: TAF::IORQueryHandler")
                ACE_TEXT(" - Unable to join the Datagram!\n"))
                , -1);

        } while (false);

        if (this->set_option(IP_MULTICAST_TTL, static_cast<char>(this->getTTL()))) {
            ACE_DEBUG((LM_WARNING,
                ACE_TEXT ("WARNING: TAF::IORQueryHandler")
                ACE_TEXT (" - Unable to initialise the Datagramming ttl=%d for network interface!\n")
                , this->getTTL()));
        }

        return 0;
    }

    int
    DiscoveryHandler::sendIORQuery(const IORQueryReplyHandler &rh, const ACE_TCHAR *svc_ident)
    {
        if (svc_ident && DAF_OS::strlen(svc_ident) > 0) {

            const taf::IORQuery mcast_query = {
                CORBA::UShort(rh.getReplyPort()), CORBA::UShort(0), svc_ident
            };

            try {

                TAF::OutputCDR os;

                char *pflen = os.write_long_placeholder(); // Reserve Frame Length

                size_t iov_len = 0;

                if (os << mcast_query && os.replace(ACE_CDR::ULong(iov_len = os.size()), pflen)) {

                    ACE_Auto_Array_Ptr<char> iov_buf(new char[iov_len + ACE_CDR::MAX_ALIGNMENT]);
                    char *iov_ptr = ACE_ptr_align_binary(iov_buf.get(), ACE_CDR::MAX_ALIGNMENT);

                    size_t cpy_len = os.copy_buffer(iov_ptr, iov_len);

                    if (cpy_len == iov_len) {
                        if (size_t(this->send(iov_ptr, iov_len)) == iov_len) { // Send the multicast.
                            return 0;
                        }
                    }
                }

            } DAF_CATCH_ALL { /* Ignore Any Error */ }

            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: TAF::IORQueryHandler")
                ACE_TEXT(" - error sending IORQuery\n"))
                , -1);
        }

        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("ERROR: TAF::IORQueryHandler")
            ACE_TEXT(" - ID is not specified correctly!\n"))
            , -1);
    }
} // namespace TAF
