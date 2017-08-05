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
#define TAF_TAFSECURITY_CPP

#include "TAFSecurity.h"

#include <daf/DAF.h>
#include <daf/PropertyManager.h>

#include <tao/PortableServer/PortableServer.h>

#if defined(TAF_HAS_SSLIOP)
# include <openssl/x509.h>
#endif

namespace TAFSecurity {

    namespace SL2 {

        /************************** Security Level 2 ************************************************************/

        const SecurityManager_var   securityManager(const CORBA::ORB_ptr orb)
        {
            if (TAF::isSecurityActive()) {
                CORBA::Object_var security_manager_obj(orb->resolve_initial_references(SECURITY_L2_MANAGER));
                if (security_manager_obj) for (SecurityManager_var security_manager(SecurityManager::_narrow(security_manager_obj.in())); security_manager;) {
                    return security_manager._retn();
                }
            }
            throw CORBA::INV_OBJREF();
        }

        const AccessDecision_var    accessDecision(const CORBA::ORB_ptr orb)
        {
            for (AccessDecision_var access_decision(securityManager(orb)->access_decision()); access_decision;) {
                return access_decision._retn();
            }
            throw CORBA::INV_OBJREF();
        }

    } // namespace SL2

    namespace SL3 {

        /************************** Security Level 3 ************************************************************/

        const SecurityManager_var       securityManager(const CORBA::ORB_ptr orb)
        {
            if (TAF::isSecurityActive()) {
                CORBA::Object_var security_manager_obj(orb->resolve_initial_references(SECURITY_L3_MANAGER));
                if (security_manager_obj) {
                    for (SecurityManager_var security_manager(SecurityManager::_narrow(security_manager_obj.in())); security_manager;) {
                        return security_manager._retn();
                    }
                }
            }
            throw CORBA::INV_OBJREF();
        }

        const SecurityCurrent_var       securityCurrent(const CORBA::ORB_ptr orb)
        {
            if (TAF::isSecurityActive()) {
                CORBA::Object_var security_current_obj(orb->resolve_initial_references(SECURITY_L3_CURRENT));
                if (security_current_obj) {
                    for (SecurityCurrent_var security_current(SecurityCurrent::_narrow(security_current_obj.in())); security_current;) {
                        return security_current._retn();
                    }
                }
            }
            throw CORBA::INV_OBJREF();
        }

        const CredentialsCurator_var    credentialsCurator(const CORBA::ORB_ptr orb)
        {
            if (TAF::isSecurityActive()) {
                CORBA::Object_var credentials_curator_obj(orb->resolve_initial_references(SECURITY_L3_CURATOR));
                if (credentials_curator_obj) {
                    for (CredentialsCurator_var credentials_curator(CredentialsCurator::_narrow(credentials_curator_obj.in())); credentials_curator;) {
                        return credentials_curator._retn();
                    }
                }
            }
            throw CORBA::INV_OBJREF();
        }

        const ClientCredentials_var     clientCredentials(const CORBA::ORB_ptr orb)
        {
            for (ClientCredentials_var client_credentials(securityCurrent(orb)->client_credentials()); client_credentials;) {
                return client_credentials._retn();
            }
            throw CORBA::INV_OBJREF();
        }

    } // namespace SL3

    /****************************************************************************************************/
#if defined(TAF_HAS_SSLIOP)
    const SSLIOP::Current_var           ssliop_current(const CORBA::ORB_ptr orb)
    {
        if (TAF::isSecurityActive()) {
            CORBA::Object_var ssliop_current_obj(orb->resolve_initial_references(SECURITY_SSLIOP_CURRENT));
            if (ssliop_current_obj) {
                for (SSLIOP::Current_var ssliop_current(SSLIOP::Current::_narrow(ssliop_current_obj.in())); ssliop_current;) {
                    return ssliop_current._retn();
                }
            }
        }
        throw CORBA::INV_OBJREF();
    }
#endif
    /****************************************************************************************************/

    AccessDecision::AccessDecision(const CORBA::ORB_ptr orb)
        : TAO_SL2_AccessDecision_ref(TAO_SL2_AccessDecision::_narrow(TAFSecurity::SL2::accessDecision(orb).in()))
        , orb_(CORBA::ORB::_duplicate(orb))
    {
        if (CORBA::is_nil(this->in())) {
            throw CORBA::INV_OBJREF();
        }
    }

    int
    AccessDecision::set_servant_access(PortableServer::Servant ps, bool allow_insecure_access)
    {
        if (ps) for (PortableServer::POA_var poa(ps->_default_POA()); poa;) try {

            CORBA::String_var               orb_id = this->orb_->id();
            CORBA::OctetSeq_var             poa_id = poa->id();
            PortableServer::ObjectId_var    obj_id = poa->servant_to_id(ps);

            (*this)->add_object(orb_id.in(), poa_id.in(), obj_id.in(), allow_insecure_access);

            return 0;

        } DAF_CATCH_ALL { /* Fall through to return error */ }

        return -1;
    }

    /****************************************************************************************************/
#if defined(TAF_HAS_SSLIOP)
    SSLCertificate::SSLCertificate(const CORBA::ORB_ptr orb)
        : SSLIOP::ASN_1_Cert_var(TAFSecurity::ssliop_current(orb)->get_peer_certificate())
        , certificate_((*this)->get_buffer())
    {
        if (CORBA::is_nil(this->certificate_)) {
            throw CORBA::INV_OBJREF();
        }
    }

    const std::string
    SSLCertificate::get_issuer_name(void) const
    {
        char buf[BUFSIZ];

        // Convert the DER encoded X.509 certificate into OpenSSL's internal format.
        X509 *peer = ::d2i_X509(0, &(const_cast<SSLCertificate*>(this)->certificate_), (*this)->length());

        ::X509_NAME_oneline(::X509_get_issuer_name(peer), buf, sizeof(buf));

        ::X509_free(peer);

        return std::string(buf);
    }
#endif

} // namespace TAFSecurity
