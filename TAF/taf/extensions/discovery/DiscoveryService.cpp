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
#define TAF_DISCOVERYSERVICE_CPP

#include "DiscoveryService.h"

#include <taf/IORQueryRepository.h>

#include <taf/ORBManager.h>
#include <taf/CDRStream.h>

#include <taf/TAFDebug.h>

#include <daf/PropertyManager.h>

#include <ace/Service_Config.h>
#include <ace/Arg_Shifter.h>
#include <ace/SOCK_Dgram.h>
#include <ace/INET_Addr.h>
#include <ace/Reactor.h>

#include <sstream>

ACE_FACTORY_DEFINE(TAFDiscovery, TAFDiscoveryService);
ACE_STATIC_SVC_DEFINE(TAFDiscoveryService,
    TAFDiscoveryService::svc_ident(),
    ACE_SVC_OBJ_T,
    &ACE_SVC_NAME(TAFDiscoveryService),
    (ACE_Service_Type::DELETE_THIS | ACE_Service_Type::DELETE_OBJ),
    false); // Service not initially active

namespace {

    ACE_INET_Addr makePeerAddress(const ACE_INET_Addr &reply_address, u_short reply_port)
    {
        ACE_INET_Addr peer_address(reply_address); peer_address.set_port_number(reply_port);

#if defined (ACE_HAS_IPV6)
        if (this->peer_address_.is_linklocal()) {
            // If this is one of our local linklocal interfaces this is not going
            // to work.
            // Creating a connection using such interface to the client listening
            // at the IPv6 ANY address is not going to work (I'm not quite sure why
            // but it probably has to do with the rather restrictive routing rules
            // for linklocal interfaces).
            // So we see if this is one of our local interfaces and if so create the
            // connection using the IPv6 loopback address instead.
            ACE_INET_Addr  peer_tmp(peer_address);
            peer_tmp.set_port_number(static_cast<u_short>(0));
            ACE_INET_Addr* tmp = 0;
            size_t cnt = 0;
            if (ACE::get_ip_interfaces(cnt, tmp) == 0) {
                for (size_t i = 0; i < cnt; ++i) {
                    if (peer_tmp == tmp[i]) {
                        this->peer_address_.set(reply_port, ACE_IPV6_LOCALHOST); break;
                    }
                }
                delete[] tmp;
            }
        }
#endif /* ACE_HAS_IPV6 */

        return peer_address;
    }
}

namespace TAF
{
    namespace {

        int sendIORReply(const TAF::IORQueryServant &servant, const ACE_INET_Addr &address, u_short flags)
        {
            ACE_UNUSED_ARG(flags);
            const CORBA::Object_ptr obj(servant.in());

            if (CORBA::is_nil(obj)) {
                return -1;
            }

            const taf::IORReply ior_reply = {
                servant.ident().c_str(), CORBA::Object::_duplicate(obj)
            };

            TAF::OutputCDR io_cdr(TAF::MAX_IOR_REPLY_LENGTH);

            size_t io_len = 0;

            char *io_ptr = io_cdr.write_long_placeholder();

            if (io_cdr << ior_reply && io_cdr.replace(ACE_CDR::ULong(io_len = io_cdr.size()), io_ptr)) {

                ACE_Auto_Array_Ptr<char> io_buf(new char[io_len]); // Output Buffer
                if (io_cdr.copy_buffer(io_ptr = io_buf.get(), io_len) == io_len) {

                    for (ACE_SOCK_Dgram dgram; dgram.open(ACE_Addr::sap_any) != -1;) {

                        DAF_OS::sleep(ACE_Time_Value(0, suseconds_t(DAF_OS::rand(500, 5000))));  // Stagger replies

                        const ACE_Time_Value send_timeout(3);

                        if (dgram.send(io_ptr, io_len, address, 0, &send_timeout) != ssize_t(io_len)) {
                            switch (errno) {
                            case ETIME:
                                ACE_DEBUG((LM_ERROR,
                                    ACE_TEXT("TAF(% 04P | % 04t) ERROR: IORReplySender - UDP reply timeout\n"))); break;
                            default:
                                ACE_DEBUG((LM_ERROR,
                                    ACE_TEXT("TAF(% 04P | % 04t) ERROR: IORReplySender - UDP reply failed\n"))); break;
                            }
                        }
                        else if (TAF::debug() > 1) {
                            char addr[BUFSIZ]; if (address.addr_to_string(addr, sizeof(addr))) *addr = 0;
                            ACE_DEBUG((LM_DEBUG,
                                ACE_TEXT("TAF (%04P | %04t) INFO: - CORBA::Object successfully sent to address '%s'\n"), addr));
                        }

                        dgram.close(); return 0;
                    }
                }
            }

            ACE_ERROR_RETURN((LM_ERROR,
                ACE_TEXT("TAF(% 04P | % 04t) ERROR: TAFDiscovery; Unable to reply to request\n")), -1);
        }
    }

    /******************************************************************************************/

    DiscoveryService::DiscoveryService(void) : active_(false)
    {
    }

    DiscoveryService::~DiscoveryService(void)
    {
    }

    const ACE_TCHAR *
    DiscoveryService::svc_ident(void)
    {
        return taf::TAFDISCOVERY_OID;
    }

    int
    DiscoveryService::init(int argc, ACE_TCHAR *argv[])
    {
        ACE_UNUSED_ARG(argc); ACE_UNUSED_ARG(argv);

        if (TAF::isDiscoveryEnabled()) {

            const std::string address(DAF::get_property(TAF_DISCOVERYENDPOINT, TAF_DEFAULT_DISCOVERY_ENDPOINT, true));

            try {
                if (this->handler_.open_handler(address.c_str()) == 0) {
                    ThePropertyRepository()->set_property(TAF_DISCOVERYENDPOINT, address);
                    if (ACE_Reactor::instance()->register_handler(this, ACE_Event_Handler::READ_MASK) == 0) {
                        this->active_ = true; return 0;
                    }
                }
            } DAF_CATCH_ALL{ /* FAll Through to Error */ }

                ACE_DEBUG((LM_WARNING,
                    ACE_TEXT("IORQueryService (%P | %t) WARNING: ")
                    ACE_TEXT("Unable to initialize discovery at address '%s' - Service Unloaded.\n")
                    , address.c_str()));
        }

        return -1; // Force Unload of service
    }

    int
    DiscoveryService::suspend(void)
    {
        ACE_NOTSUP_RETURN(-1);
    }

    int
    DiscoveryService::resume(void)
    {
        ACE_NOTSUP_RETURN(-1);
    }

    int
    DiscoveryService::fini(void)
    {
        if (this->isActive()) {
            this->active_ = false;
            if (this->reactor()) {
                this->reactor()->remove_handler(this, (ACE_Event_Handler::READ_MASK | ACE_Event_Handler::DONT_CALL));
            }
        }
        return 0;
    }

    int
    DiscoveryService::info(ACE_TCHAR **info_string, size_t length) const
    {
        static const char *info_desc =
        {
            "A service that implements IORQuery Discovery for the TAF framework."
        };

        return (info_string ? (*info_string = ACE::strnnew(info_desc, length), 0) : -1);
    }

    ACE_HANDLE
    DiscoveryService::get_handle(void) const
    {
        return this->handler_.get_handle();
    }

    int
    DiscoveryService::handle_input(ACE_HANDLE handle)
    {
        if (this->isActive()) {
            if (handle != this->get_handle()) {
                return 0; // Not for Us
            }
        } else return -1; // Unhook from Reactor

        ACE_CDR::ULong  io_header = 0;
        ACE_INET_Addr   io_address;

        // Take a peek at the header to find out how long is the service name string we should receive.
        if (this->handler_.recv(&io_header, sizeof(io_header), io_address, MSG_PEEK) == sizeof(io_header)) {
            size_t io_len = size_t(ACE_UINT16_MAX & ACE_NTOHL(io_header)); // Mask for Read Length

            // All OK - Receive full client multicast request.
            if (ace_range(MIN_IOR_REPLY_LENGTH, MAX_IOR_REPLY_LENGTH, io_len) == io_len) {

                ACE_Auto_Array_Ptr<char> io_buff(new char[io_len + ACE_CDR::MAX_ALIGNMENT]);
                char *io_ptr = ACE_ptr_align_binary(io_buff.get(), ACE_CDR::MAX_ALIGNMENT);

                // Read the iovec and send back our IOR response if we recognize it.
                if (size_t(this->handler_.recv(io_ptr, io_len, io_address)) == io_len) {

                    ACE_CDR::ULong iov_len; taf::IORQuery ior_query;

                    TAF::InputCDR cdr(io_ptr, io_len); do {

                        if (cdr >> iov_len && io_len == size_t(iov_len)) {
                            if (cdr >> ior_query) {
                                break; // All Good
                            }
                        }

                        ACE_ERROR_RETURN((LM_ERROR, // Must Return 0 to stop reactor disconnecting
                            ACE_TEXT("TAF (%P | %t) READ-ERROR IORQueryService::handle_event.\n")), 0);

                    } while (false);

                    const std::string ident(ior_query.svc_ident.in());

                    if (ident.length()) do {

                        const ACE_INET_Addr reply_address(makePeerAddress(io_address, u_short(ior_query.svc_port)));
                        {
                            TAF::IORServantRepository *queryRepository = TheIORQueryRepository();

                            ACE_GUARD_REACTION(DAF_SYNCH_MUTEX, taf_mon, *queryRepository, break);

                            for (TAF::IORServantRepository::iterator it(queryRepository->begin()); it != queryRepository->end();) {
                                if (this->isActive()) try {
                                    if (it->is_ident(ident)) { // Hand Off For UDP Send
                                        if (sendIORReply(*it, reply_address, u_short(ior_query.svc_flags))) {
                                            throw "Discovery-Failed-Send-Reply";
                                        }
                                    }
                                    it++;
                                } DAF_CATCH_ALL {
                                    it = TheIORQueryRepository()->erase(it);
                                } else break;
                            }
                        }

                        if (TAF::debug() > 2) {
                            ACE_DEBUG((LM_INFO, ACE_TEXT("IORQuery Response sent to '%s:%d'\n")
                                , reply_address.get_host_name(), int(reply_address.get_port_number())));
                        }

                    } while (false);

                    return this->isActive() ? 0 : -1; // May Unhook From Reactor
                }
            }
        }

        ACE_ERROR_RETURN((LM_ERROR,
            ACE_TEXT("TAF (%P | %t) READ-ERROR IORQueryService::handle_event.\n")), 0);
    }

} // namespace TAF
