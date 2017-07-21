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
#define TAF_EXTENSIONS_CPP

#include "TAFExtensions.h"

#if defined(TAF_HAS_DISCOVERY)
# include <taf/extensions/discovery/DiscoveryService.h>
#endif

#include <taf/TAF.h>

#include <daf/PropertyManager.h>

#include <tao/ORB_Core.h>
#include <tao/PI/ORBInitInfo.h>
#include <tao/ORBInitializer_Registry.h>

#include <tao/PI/PI.h>
#include <tao/LocalObject.h>

#include <ace/Service_Config.h>

namespace  {  // Anonymous

    static class TAFExtensionsInitializer : virtual public PortableInterceptor::ORBInitializer, virtual public CORBA::LocalObject
    {
        bool pre_init_, post_init_;

    public:

        TAFExtensionsInitializer(void);

        virtual void pre_init(PortableInterceptor::ORBInitInfo_ptr info);
        virtual void post_init(PortableInterceptor::ORBInitInfo_ptr info);

    } tafExtensionsInitializer; DAF_UNUSED_STATIC(tafExtensionsInitializer);

    TAFExtensionsInitializer::TAFExtensionsInitializer(void) : pre_init_(false), post_init_(false)
    {
        PortableInterceptor::register_orb_initializer(this); // Register this ORBInitializer
    }

    void
    TAFExtensionsInitializer::pre_init(PortableInterceptor::ORBInitInfo_ptr info)
    {
        if (this->pre_init_ ? false : (this->pre_init_ = true)) {
            TAO_ORBInitInfo_var tao_info = TAO_ORBInitInfo::_narrow(info);

            if (CORBA::is_nil(tao_info.in())) {
                throw CORBA::INTERNAL();
            }
        }
    }

    void
    TAFExtensionsInitializer::post_init(PortableInterceptor::ORBInitInfo_ptr info)
    {
        if (this->post_init_ ? false : (this->post_init_ = true)) {

            TAO_ORBInitInfo_var tao_info = TAO_ORBInitInfo::_narrow(info);

            if (CORBA::is_nil(tao_info.in())) {
                throw CORBA::INTERNAL();
            }

            const std::string svc_args(DAF::get_property(TAF_SERVICE_ARGS, false)); // Get The Current Service Arguments

            {   // Scope the Gestalt Guard

                ACE_Service_Config_Guard svc_guard(tao_info->orb_core()->configuration()); ACE_UNUSED_ARG(svc_guard);

#if defined(TAF_HAS_DISCOVERY)
                ACE_Service_Config::process_directive(ace_svc_desc_TAFDiscoveryService);
                if (ACE_Service_Config::initialize(TAFDiscoveryService::svc_ident(), svc_args.c_str())) {
                    ACE_DEBUG((LM_WARNING,
                        ACE_TEXT("TAFResources (%P | %t) WARNING: Unable to initialize %s.\n"),
                        TAFDiscoveryService::svc_ident()));
                }
#endif
            }
        }
    }

}  // Anonymous

int TAFExtensions_Export TAF::ExtensionsInitializer(void)
{
    return 0; // Force Linkage and DLL Load
}
