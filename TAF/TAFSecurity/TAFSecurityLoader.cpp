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
#define TAF_SECURITYLOADER_CPP

#include "TAFSecurityLoader.h"

#include <ace/OS.h>
#include <ace/Arg_Shifter.h>

#include <tao/PI/ORBInitInfo.h>
#include <tao/ORBInitializer_Registry.h>
#include <tao/Strategies/advanced_resource.h>  // Force Load Advanced Resources

#include <fstream>

#define SSLIOP_IDENT            ACE_TEXT("SSLIOP_Factory")
#define SSLIOP_LIBRARY          ACE_TEXT("TAO_SSLIOP")
#define SSLIOP_FACTORY          ACE_TEXT("_make_TAO_SSLIOP_Protocol_Factory")
#define SSLIOP_ARGS             ACE_TEXT("%s")
#define SSLIOP_DIRECTIVE        ACE_TEXT(ACE_DYNAMIC_SERVICE_DIRECTIVE(SSLIOP_IDENT,SSLIOP_LIBRARY,SSLIOP_FACTORY,SSLIOP_ARGS))
#define SSLIOP_RESOURCE         ACE_TEXT(ACE_STATIC_SERVICE_DIRECTIVE("%s", "-ORBProtocolFactory " SSLIOP_IDENT))

#define ORB_SVCONF              ACE_TEXT("-ORBSvcConf '%s'")
#define SSL_NOPROTECTION        ACE_TEXT("SSLNoProtection") /* This is a special case */

#define SSL_NOPROTECTION_FLAG   ACE_TEXT("-" SSL_NOPROTECTION)
#define TAF_SECURITY_FLAG       ACE_TEXT("-" TAF_SECURITY)

namespace {

    volatile bool _security_active = false;

    // Options supported in $(TAO_ROOT)/orbsvcs/orbsvcs/SSLIOP/SSLIOP_Factory.cpp

#if defined(TAF_HAS_SSLIOP)

    const ACE_TCHAR *ssliop_options[] = {
        "SSLAuthenticate",
        "SSLPrivateKey",
        "SSLCertificate",
        "SSLAcceptTimeout",
        "SSLDHparams",
        "SSLCAfile",
        "SSLCApath",
        "SSLrand",
#if !defined (__Lynx__)
        "SSLServerCipherOrder",
        "SSLCipherList",
#endif
        "SSLVersionList",
        "SSLPassword",
        "SSLCheckHost",
        0  // Signifies End of arguments
    };

#endif

    inline const char * resource_factory_name(void) {
        return ace_svc_desc_TAO_Advanced_Resource_Factory.name_;
    }

    const struct SVC_CONF_FILENAME : std::string {
        SVC_CONF_FILENAME(void)
        {
            std::stringstream ss; std::ends(ss << TAF_SECURITY << static_cast<int>(ACE_OS::getpid()) << ".conf"); this->assign(ss.str());
        }
        const char * operator () (void) const { return this->c_str(); }
    } svc_conf_filename;
}

namespace TAF { // Override isSecurityActive
    bool    isSecurityActive(void)
    {
        return _security_active;
    }
}

namespace TAFSecurity
{
    Loader::Loader(int &argc, ACE_TCHAR *argv[], bool use_property)
    {
        if (TAF::isSecurityActive()) {
            throw CORBA::INITIALIZE();
        }

        try {

            PortableInterceptor::register_orb_initializer(this); // Register this ORBInitializer

#if defined(TAF_HAS_SSLIOP)

            if (argc) for (ACE_Arg_Shifter arg_shifter(argc, argv); arg_shifter.is_anything_left();) {

                if (arg_shifter.is_option_next()) {
                    if (arg_shifter.cur_arg_strncasecmp(TAF_SECURITY_FLAG) == 0) {
                        for (arg_shifter.consume_arg(); arg_shifter.is_parameter_next(); arg_shifter.consume_arg()) {
                            try {
                                if (this->load_file_profile(arg_shifter.get_current()) == 0) {
                                    continue;
                                }
                            } DAF_CATCH_ALL {
                                /* Drop Through to WARNING */
                            }

                            ACE_DEBUG((LM_WARNING, ACE_TEXT("TAF (%P | %t) TAFSecurity::Loader WARNING: ")
                                ACE_TEXT("Unable to load security properties from file argument '%s' - Removed.\n")
                                , arg_shifter.get_current()));
                        }
                        continue;
                    }
                }
                arg_shifter.ignore_arg();
            }

            if (this->load_count() ? false : use_property) {
                if (this->load_file_profile(DAF::get_property(this->config_switch(), use_property))) {
                    throw "Unable-To-Load-Security-Profile";
                }
            }

            if (this->size() == 0) do { // Do we dont have any Security Properties
                if (DAF::get_numeric_property<bool>(TAF_SSLNOPROTECTION, use_property)) {
                    this->set_property(SSL_NOPROTECTION, "1"); break;
                }
                throw "Force-No-Protection";
            } while (false);
#else
            this->set_property(SSL_NOPROTECTION, "1");
#endif
        } catch (const CORBA::Exception &ex) {
            ex._tao_print_exception("TAFSecurity::Loader initialize"); throw;
        } DAF_CATCH_ALL {

            ACE_DEBUG((LM_WARNING, ACE_TEXT("TAFSecurity (%P | %t) WARNING:")
                ACE_TEXT(" Invalid %s configuration; defaulting to %s.\n")
                ,TAF_SECURITY,SSL_NOPROTECTION));

            this->set_property(SSL_NOPROTECTION, "1");
        }
    }

    Loader::~Loader(void)
    {
        std::remove(svc_conf_filename()); // Remove here just incase we did not earlier in post_init
    }

    int
    Loader::init(std::string &orb_params)
    {
#if defined(TAF_HAS_SSLIOP)

        std::string ssliop_args;

        if (DAF::debug() > 2 || this->get_numeric_property<bool>("verbose", false, false)) {
            if (ssliop_args.length()) {
                ssliop_args.append(1, ' ');
            }
            ssliop_args.append("-verbose");
        }

        do {

            if (this->get_numeric_property<bool>(SSL_NOPROTECTION, false, false)) {
                if (ssliop_args.length()) {
                    ssliop_args.append(1, ' ');
                }
                ssliop_args.append(SSL_NOPROTECTION_FLAG); break; // Say No Protection
            }

            // "SSLNoProtection" is not set, so process any other security properties (from index 1)
            for (int i = 1; ssliop_options[i]; i++) try {

                const std::string ssliop_arg(this->get_property(ssliop_options[i], false));

                std::stringstream ssliop_opt; ssliop_opt << ssliop_options[i] << ' ' << ssliop_arg;

                if (ssliop_args.length()) {
                    ssliop_args.append(1, ' ');
                }

                ssliop_args.append(1, '-').append(DAF::trim_string(ssliop_opt.str()));

            } catch (const DAF::NotFoundException &) { /* option Not Set */ }

        } while (false);

        this->clear(); // Clear out the properties

        for (std::ofstream ofile(svc_conf_filename(), std::ios_base::out); ofile;) {

            char directive[512];

            ACE_OS::sprintf(directive, SSLIOP_DIRECTIVE, DAF::trim_string(ssliop_args).c_str());
            ofile << directive << std::endl;
            ACE_OS::sprintf(directive, SSLIOP_RESOURCE, resource_factory_name());
            ofile << directive << std::endl;
            ACE_OS::sprintf(directive, ORB_SVCONF, svc_conf_filename());
            if (orb_params.length()) { orb_params.append(1,' '); } orb_params.append(directive);

            ofile << std::flush; return 0;
        }

        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("TAFSecurityLoader (%P | %t) ERROR:")
            ACE_TEXT(" Unable to create SSLIOP ORBSvcConf loader file \"%s\"\n")
            , svc_conf_filename()), -1);
#else
        ACE_UNUSED_ARG(orb_params); return 0;
#endif
    }

    void
    Loader::pre_init(PortableInterceptor::ORBInitInfo_ptr info)
    {
        TAO_ORBInitInfo_var tao_info = TAO_ORBInitInfo::_narrow(info);

        if (CORBA::is_nil(tao_info.in()) || TAF::isSecurityActive()) {
            throw CORBA::INTERNAL();
        }

        //const ACE_CString resource_factory(TAO_ORB_Core_Static_Resources::instance()->resource_factory_name_);
        //for (ACE_Service_Object *svc = ACE_Dynamic_Service<ACE_Service_Object>::instance(tao_info->orb_core()->configuration(), resource_factory.c_str()); svc;) {
        //    ACE_ARGV args("-ORBProtocolFactory " SSLIOP_IDENT);
        //    if (svc->init(args.argc(), args.argv())) {
        //        ACE_DEBUG((LM_WARNING,
        //            ACE_TEXT("TAFSecurityLoader (%P | %t) WARNING: Unable to initialize with %s.\n"),
        //            args.buf()));
        //    }
        //    break;
        //}
    }

    void
    Loader::post_init(PortableInterceptor::ORBInitInfo_ptr info)
    {
        TAO_ORBInitInfo_var tao_info = TAO_ORBInitInfo::_narrow(info);

        if (CORBA::is_nil(tao_info.in()) || TAF::isSecurityActive()) {
            throw CORBA::INTERNAL();
        }

        do try {

            CORBA::Object_var sl2sm_obj = tao_info->resolve_initial_references(SECURITY_L2_MANAGER);
            SecurityLevel2::SecurityManager_var sl2sm = SecurityLevel2::SecurityManager::_narrow(sl2sm_obj.in());
            if (CORBA::is_nil(sl2sm.in())) {
                break;
            }

            CORBA::Object_var sl2ad_obj = sl2sm->access_decision();
            TAO_SL2_AccessDecision::_var_type sl2ad = TAO_SL2_AccessDecision::_narrow(sl2ad_obj.in());
            if (CORBA::is_nil(sl2ad.in())) {
                break;
            }

            bool default_decision = DAF::get_numeric_property<bool>(TAF_DEFAULTALLOWANCE, false, false);

            sl2ad->default_decision(default_decision);

            if (DAF::debug() > 1) {
                ACE_DEBUG((LM_INFO, ACE_TEXT("TAFSecurity (%P | %t) INFO: Default allowance decision set to %s.\n")
                    , (default_decision ? "true" : "false")));
            }

#if ACE_GTEQ_VERSION(6,3,3)

            bool collocated_decision = DAF::get_numeric_property<bool>(TAF_COLLOCATEDALLOWANCE, default_decision, false);

            sl2ad->default_collocated_decision(collocated_decision);

            if (DAF::debug() > 1) {
                ACE_DEBUG((LM_INFO, ACE_TEXT("TAFSecurity (%P | %t) INFO: Collocated allowance decision set to %s.\n")
                    , (collocated_decision ? "true" : "false")));
            }

#endif

            std::remove(svc_conf_filename()); _security_active = true; return;

        } DAF_CATCH_ALL {
        } while (false);

        _security_active = false;

        ACE_DEBUG((LM_ERROR, ACE_TEXT("TAFSecurityLoader (%P | %t) ERROR:")
            ACE_TEXT(" Unable to set allowance decisions.\n")));
    }

} // namespace TAFSecurity
