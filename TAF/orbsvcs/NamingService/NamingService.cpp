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
#define TAF_NAMINGSERVICE_CPP

#include "NamingService.h"

#include "taf/ORBManager.h"
#include "taf/IORQueryRepository.h"

#include <ace/ARGV.h>

#include <orbsvcs/Naming/Naming_Server.h>
#include <orbsvcs/Log_Macros.h>
#include <orbsvcs/Daemon_Utilities.h>

ACE_FACTORY_DEFINE(NamingService, TAF_NamingService);
ACE_STATIC_SVC_DEFINE(TAF_NamingService
    , TAF_NamingService::svc_ident()
    , ACE_SVC_OBJ_T
    , &ACE_SVC_NAME(TAF_NamingService)
    , (ACE_Service_Type::DELETE_THIS | ACE_Service_Type::DELETE_OBJ)
    , true // Service not initially active
    );

ACE_DLL_UNLOAD_POLICY(NamingService, ACE_DLL_UNLOAD_POLICY_LAZY);

namespace { // Ananomous
    struct NamingIORBinder : TAF::IORBinderSequence {
        NamingIORBinder(void) : TAF::IORBinderSequence() {
            this->add_binding(new TAF::RegisterIORQueryBinder());
        }
    };
}

namespace TAF
{
    NamingService::NamingService(void) : NamingInterfaceHandler(NamingIORBinder())
    {
        this->multicast_ = true; // Always turn on the multicast discovery
    }

    NamingService::~NamingService(void)
    {
    }

    /* CosNaming::NamingContext IDL Methods */

    void
    NamingService::bind(const CosNaming::Name &n, CORBA::Object_ptr obj)
    {
        (*this)->bind(n, obj);
    }

    void
    NamingService::rebind(const CosNaming::Name &n, CORBA::Object_ptr obj)
    {
        (*this)->rebind(n, obj);
    }

    void
    NamingService::bind_context(const CosNaming::Name &n, CosNaming::NamingContext_ptr cxt)
    {
        (*this)->bind_context(n, cxt);
    }

    void
    NamingService::rebind_context(const CosNaming::Name &n, CosNaming::NamingContext_ptr cxt)
    {
        (*this)->rebind_context(n, cxt);
    }

    CORBA::Object_ptr
    NamingService::resolve(const CosNaming::Name &n)
    {
        return (*this)->resolve(n);
    }

    void
    NamingService::unbind(const CosNaming::Name &n)
    {
        (*this)->unbind(n);
    }

    CosNaming::NamingContext_ptr
    NamingService::new_context(void)
    {
        return (*this)->new_context();
    }

    CosNaming::NamingContext_ptr
    NamingService::bind_new_context(const CosNaming::Name &n)
    {
        return (*this)->bind_new_context(n);
    }

    void
    NamingService::destroy(void)
    {
        (*this)->destroy();
    }

    void
    NamingService::list(CORBA::ULong how_many, CosNaming::BindingList_out bl, CosNaming::BindingIterator_out bi)
    {
        (*this)->list(how_many, bl, bi);
    }

    int
    NamingService::init(int argc, ACE_TCHAR* argv[])
    {
        const std::string args_param(DAF::parse_args(argc, argv));

#if 0 // For Commentry only
            ACE_TEXT ("-d ") // debug output
            ACE_TEXT ("-o <ior_output_file> ")
            ACE_TEXT ("-p <pid_file_name> ")
            ACE_TEXT ("-s <context_size> ")
            ACE_TEXT ("-b <base_address> ")
            ACE_TEXT ("-u <persistence dir name> ")
            ACE_TEXT ("-m <1=enable multicast(default), 0=disable multicast ")
#if !defined (CORBA_E_MICRO)
            ACE_TEXT ("-f <persistence_file_name> ")
#endif /* CORBA_E_MICRO */
#if (TAO_HAS_MINIMUM_POA == 0) && !defined (CORBA_E_MICRO)
            ACE_TEXT ("-u <storable_persistence_directory (not used with -f)> ")
            ACE_TEXT ("-r <redundant_persistence_directory> ");
#endif /* TAO_HAS_MINIMUM_POA && !CORBA_E_MICRO */
            ACE_TEXT ("-z <relative round trip timeout> ")
#endif
        ACE_ARGV args(true); args.add(NamingService::svc_ident()); args.add(args_param.c_str());
        try { // Initialize the state of the TAO_Naming_Service object
            ACE_Service_Config_Guard svc_guard(ACE_Service_Config::global()); ACE_UNUSED_ARG(svc_guard);
            if (this->init_with_orb(args.argc(), args.argv(), TheTAFOrb()) == 0) {
                this->init_bind(NamingService::svc_ident()); return 0;
            }
        } catch (const CORBA::Exception &ex) {
            ex._tao_print_exception("ERROR: NamingService POA Creation failed.");
        } DAF_CATCH_ALL {
        }

        return -1;
    }

    int
    NamingService::suspend(void)
    {
        DAF_OS::last_error(ENOTSUP); return -1;
    }

    int
    NamingService::resume(void)
    {
        DAF_OS::last_error(ENOTSUP); return -1;
    }

    int
    NamingService::fini(void)
    {
        int fini_val = this->fini_bind();

        try {
            ACE_Service_Config_Guard svc_guard(ACE_Service_Config::global()); ACE_UNUSED_ARG(svc_guard); return TAO_Naming_Server::fini();
        } DAF_CATCH_ALL {}

        return fini_val;
    }

    int
    NamingService::info(ACE_TCHAR **info_string, size_t length) const
    {
        static const char *info_desc =
        {
            "A service wrapper implementation to the CORBA Naming Service."
        };

        return (info_string ? (*info_string = ACE::strnnew(info_desc, length), 0) : -1);
    }
}
