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

#define TAF_XMPL_CORBASERVICE_CPP

#include "CORBAService.h"

#include <taf/ORBManager.h>
#include <taf/IORBinder.h>

#include <ace/ARGV.h>
#include <ace/Get_Opt.h>

ACE_FACTORY_DEFINE(CORBAService, TAF_XMPL_CORBAService);
ACE_STATIC_SVC_DEFINE(TAF_XMPL_CORBAService
    , TAF_XMPL_CORBAService::svc_ident()
    , ACE_SVC_OBJ_T
    , &ACE_SVC_NAME(TAF_XMPL_CORBAService)
    , (ACE_Service_Type::DELETE_THIS | ACE_Service_Type::DELETE_OBJ)
    , false // Service not initially active
    );

namespace TAF_XMPL
{
    namespace {
        std::string _ior_file(std::string(taf_xmpl::MY_SERVICE_OID).append(".ior"));

        struct SimpleIORFileBinder : TAF::IORFileBinder {
            virtual std::string make_bind_name(const std::string &name) const
            {
                ACE_UNUSED_ARG(name); return _ior_file;
            }
        };
    }

    CORBAService::CORBAService(void) : vlong_(0)
    {
    }

    CORBAService::~CORBAService(void)
    {
    }

    int
    CORBAService::parse_args(int argc, char *argv[])
    {
        if (argc) this->svc_args_.assign(DAF::parse_args(argc, argv));

        for (ACE_Get_Opt get_opts(argc, argv, "ho:");;) switch(get_opts()) {
            case -1 :   return 0; // Indicates sucessful parsing of the command line
            case 'o':   _ior_file.assign(get_opts.opt_arg()); break;
            case 'h':   ACE_DEBUG((LM_DEBUG, "usage: %s -o <ior filename>.\n", argv[0])); break;
            default :   break; // Ignore
        }

        ACE_NOTREACHED (return -1;) // Should Never Get Here (Keep Compiler happy)
    }

    /// Initializes object when dynamic linking occurs.
    int
    CORBAService::init(int argc, ACE_TCHAR *argv[])
    {
        if (this->parse_args(argc, argv) == 0) try {

            this->addIORBinding(new SimpleIORFileBinder());

            if (this->init_bind(this->svc_ident()) == 0) {

                ACE_ERROR_RETURN((LM_DEBUG, ACE_TEXT("(%04P|%04t) SERVER Activated as:\n<%s>\n")
                    , TAFObjectToString(this->stub_reference()).in()), 0);
            }

        } DAF_CATCH_ALL { /* FAll Through to Error */ }

        ACE_ERROR_RETURN((LM_ERROR,
            ACE_TEXT("ERROR: MySimpleServer:- Failed to initialize Service successfully\n")), -1);
    }

    /// Terminates object when dynamic unlinking occurs.
    int
    CORBAService::fini(void)
    {
        this->fini_bind(); return 0;
    }

    /// Returns information on a service object.
    int
    CORBAService::info(ACE_TCHAR **info_string, size_t length) const
    {
        static const char *info_desc =
        {
            "The CORBAService is a simple passive service to example the TAF framework services."
        };

        return (info_string ? (*info_string = ACE::strnnew(info_desc, length), 0) : -1);
    }

    void
    CORBAService::register_callback(taf_xmpl::SimpleCallback_ptr)
    {
        ACE_DEBUG ((LM_DEBUG,"(%04P|%04t) SERVER::register_callback NOT Implemented.\n"));
    }

    CORBA::Long
    CORBAService::test_method(CORBA::Long x, const taf_xmpl::Structure& in_struct, taf_xmpl::Structure_out out_struct, char *&name)
    {
        out_struct = new taf_xmpl::Structure(in_struct);  out_struct->i += (x + 100);

        ACE_DEBUG ((LM_DEBUG,"(%04P|%04t) SERVER::test_method():-> x = %d, i = %d, name = <%C>\n", x, in_struct.i, name));

        name = CORBA::string_dup(std::string(name).append(" - Yes it is!!").c_str());

        return this->vlong_;
    }

    void
    CORBAService::test_callback(taf_xmpl::SimpleCallback_ptr callback_ptr, const char *text)
    {
        callback_ptr->callback_op(text);
    }

    void
    CORBAService::test_oneway(const char *name)
    {
        ACE_DEBUG ((LM_DEBUG, "(%04P|%04t) SERVER::test_oneway():-> name = <%C>\n", name));
    }

    CORBA::Long
    CORBAService::test_val(void)
    {
        return vlong_;
    }

    void
    CORBAService::test_val(CORBA::Long test_val)
    {
        ACE_DEBUG ((LM_DEBUG, "(%04P|%04t) SERVER::test_val(%d):->\n", (vlong_ = test_val)));
    }

    void
    CORBAService::raise_user_exception(void)
    {
        throw taf_xmpl::test_exception(33, "reactor meltdown", "kaput");
    }

    void
    CORBAService::raise_system_exception(void)
    {
        throw CORBA::NO_PERMISSION();
    }
}
