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
#ifndef TAF_TAFSECURITY_H
#define TAF_TAFSECURITY_H

/********************* NOTE **************************
 CANNOT Link (or use) TAF.dll -> Circular Dependencies
******************************************************/

#include "TAFSecurity_export.h"

#include <daf/DAF.h>
#include <daf/ObjectRef_T.h>

#include <tao/ORB.h>
#include <tao/ORB_Core.h>
#include <tao/PortableServer/Servant_Base.h>

#include <orbsvcs/SecurityLevel2C.h>
#include <orbsvcs/SecurityLevel3C.h>

#include <orbsvcs/orbsvcs/SSLIOPC.h>

#define SECURITY_CURRENT        ACE_TEXT("SecurityCurrent")

#define SECURITY_L2_MANAGER     ACE_TEXT("SecurityLevel2:SecurityManager")

#define SECURITY_L3_CURRENT     ACE_TEXT("SecurityLevel3:SecurityCurrent")
#define SECURITY_L3_CURATOR     ACE_TEXT("SecurityLevel3:CredentialsCurator")
#define SECURITY_L3_MANAGER     ACE_TEXT("SecurityLevel3:SecurityManager")

#define SECURITY_SSLIOP_CURRENT ACE_TEXT("SSLIOPCurrent")

typedef class ::TAO::SL2::AccessDecision            TAO_SL2_AccessDecision;
typedef DAF::ObjectRef<TAO_SL2_AccessDecision>      TAO_SL2_AccessDecision_ref;

namespace TAF {
    TAFSecurity_Export  bool isSecurityActive(void);
}

namespace TAFSecurity {

    namespace SL2 { using namespace SecurityLevel2;
        TAFSecurity_Export const SecurityManager_var    securityManager(const CORBA::ORB_ptr orb = TAO_ORB_Core_instance()->orb());
        TAFSecurity_Export const AccessDecision_var     accessDecision(const CORBA::ORB_ptr orb = TAO_ORB_Core_instance()->orb());
    }

    namespace SL3 { using namespace SecurityLevel3;
        TAFSecurity_Export const SecurityManager_var    securityManager(const CORBA::ORB_ptr orb = TAO_ORB_Core_instance()->orb());
        TAFSecurity_Export const SecurityCurrent_var    securityCurrent(const CORBA::ORB_ptr orb = TAO_ORB_Core_instance()->orb());
        TAFSecurity_Export const CredentialsCurator_var credentialsCurator(const CORBA::ORB_ptr orb = TAO_ORB_Core_instance()->orb());
        TAFSecurity_Export const ClientCredentials_var  clientCredentials(const CORBA::ORB_ptr orb = TAO_ORB_Core_instance()->orb());
    }

/* Have to type these to DAF::ObjectRef<T> (enhanced _var) so we can inherit within helper classes */

    class TAFSecurity_Export AccessDecision : public TAO_SL2_AccessDecision_ref
    {
        CORBA::ORB_var  orb_;

    public:

        AccessDecision(const CORBA::ORB_ptr orb = TAO_ORB_Core_instance()->orb());

        int set_servant_access(PortableServer::Servant, bool allow_insecure_access = false);
    };

#if defined(TAF_HAS_SSLIOP)
    class TAFSecurity_Export SSLCertificate : SSLIOP::ASN_1_Cert_var
    {
        const CORBA::Octet *certificate_;

    public:

        SSLCertificate(const CORBA::ORB_ptr orb = TAO_ORB_Core_instance()->orb());

        const std::string   get_issuer_name(void) const;
    };
#endif

} // namespace TAF

typedef class TAFSecurity::AccessDecision   TAFSecurityAccessDecision;

#endif // TAF_SECURITY_H
