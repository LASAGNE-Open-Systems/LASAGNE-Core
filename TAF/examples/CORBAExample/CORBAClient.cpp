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

#include "ace/ARGV.h"
#include "ace/Log_Msg.h"
#include "ace/Min_Max.h"
#include "ace/Get_Opt.h"
#include "ace/High_Res_Timer.h"
#include "ace/Service_Config.h"
#include "ace/Reactor.h"

#include "taf/ORBManager.h"

#include "taf/ObjectStubRef_T.h"

#include "daf/ShutdownHandler.h"

#include <string>

#define CLIENT_USES_ORB_THREADPOOL  1
#define CLIENT_USES_IORFILE         0

#include "CORBAServiceC.cpp"
#include "CORBAServiceS.cpp"

#include "CORBAService.h"

#if defined(TAF_HAS_SIOP) && (TAF_HAS_SIOP != 0)
# include "SIOP/SIOP.h"
#endif

// Derek's Desktop
// -ORBObjRefStyle URL -ORBInitRef MySimpleServer=corbaloc::dsto-aar11972:8765/MySimpleServer
// -ORBObjRefStyle URL -ORBInitRef MySimpleServer=corbaloc::131.185.25.12:8765/MySimpleServer
// -ORBObjRefStyle URL -ORBInitRef MySimpleServer=corbaloc:iiop:1.2@dsto-aar11972:8765/MySimpleServer

// Derek's Laptop
// -ORBObjRefStyle URL -ORBInitRef MySimpleServer=corbaloc::131.185.25.23:8765/MySimpleServer
// -ORBObjRefStyle URL -ORBInitRef MySimpleServer=corbaloc::aod-dsto100358:8765/MySimpleServer
// -ORBObjRefStyle URL -ORBInitRef MySimpleServer=corbaloc:iiop:1.2@aod-dsto100358:8765/MySimpleServer

struct HRTimer : ACE_High_Res_Timer {
    std::string msg_;
    HRTimer(const char *msg = "") : msg_(msg)   { this->start();    }
    ~HRTimer(void) { this->stop(); this->print_ave(msg_.c_str(),1); }
};

static int parse_args(int argc, char *argv[])
{
    ACE_UNUSED_ARG(argc);
    ACE_UNUSED_ARG(argv);
    return 0;
}

struct SimpleCallback_impl : POA_taf_xmpl::SimpleCallback
{
    virtual void  callback_op(const char *text)
    {
         ACE_DEBUG ((LM_DEBUG, "(%04P|%04t)<-- CLIENT::callback_op(%C):\n", text));
    }
};

static taf_xmpl::SimpleServer_ptr locate_SimpleServer(::CORBA::ORB_ptr orb)
{
    static size_t retry(1);

    taf_xmpl::SimpleServer_var objRef;

    for(;;) try {
#if defined(CLIENT_USES_IORFILE) && (CLIENT_USES_IORFILE != 0)
        CORBA::Object_var server_proxy(orb->string_to_object(std::string("file://").append(_ior_file).c_str()));
#else
        CORBA::Object_var server_proxy(orb->resolve_initial_references(TAF_XMPL::CORBAService::svc_ident()));
#endif
        if (CORBA::is_nil(objRef = taf_xmpl::SimpleServer::_narrow(server_proxy))) {
            throw CORBA::TRANSIENT();
        } else break;
    } catch (const CORBA::Exception&) {
        ACE_DEBUG((LM_ERROR, "CLIENT: SimpleServer Object reference is invalid - Retry(%d).\n",retry++)); DAF_OS::sleep(2);
    }

    return objRef._retn();
}

static bool _shutdown = false;

int main(int argc, char *argv[])
{
    struct _Shutdown : DAF::ShutdownHandler {
        virtual int handle_shutdown(int sig = 0) {
            _shutdown = true; return DAF::ShutdownHandler::handle_shutdown(sig);
        }
    } shutdown_;

    if (parse_args(argc, argv) == 0) try {

        TAF::ORBManager orb(argc, argv); orb.run(2);

        //***********************
        TAF::ObjectStubRef<SimpleCallback_impl>   callback(new SimpleCallback_impl());

        CORBA::String_var   cb_ref_ior(orb->object_to_string(callback));

        ACE_DEBUG((LM_DEBUG, "CLIENT:Callback Activated as:\n  <%C>\n", cb_ref_ior.in()));

        taf_xmpl::SimpleServer_var server_objRef(taf_xmpl::SimpleServer::_nil());

        for (unsigned i = 0; !_shutdown; i++) {
            while (CORBA::is_nil(server_objRef)) {
                server_objRef = ::locate_SimpleServer(orb);
            }

            ACE_DEBUG ((LM_DEBUG, "\n********* CLIENT::LOOP START [%03d] ************************************\n",i));

//            server_objRef->register_callback(callback);

            try {
                HRTimer _("********* CLIENT::LOOP END ****************** -");

                server_objRef->test_val(CORBA::Long(i));

                CORBA::Long val = server_objRef->test_val();

                ACE_DEBUG ((LM_DEBUG, "(%04P|%04t)<-> %d == CLIENT::test_val(%d):\n", val, i));

                taf_xmpl::Structure in_struct; in_struct.i = i;

                CORBA::String_var name(CORBA::string_dup("My name is DEREK!"));

                taf_xmpl::Structure_var out_struct;

                CORBA::Long r = server_objRef->test_method(i, in_struct, out_struct.out(), name.inout());

                ACE_DEBUG ((LM_DEBUG,"(%04P|%04t)<-> %d == CLIENT::test_method():<- out_struct->i = %d, name = <%C>\n", r, out_struct->i, name.in()));

                ACE_DEBUG ((LM_DEBUG,"(%04P|%04t)--> CLIENT::test_callback():\n"));
                server_objRef->test_callback(callback, "Derek Rocks!");

                ACE_DEBUG ((LM_DEBUG,"(%04P|%04t)--> CLIENT::test_oneway():\n"));
                server_objRef->test_oneway(CORBA::String_var(CORBA::string_dup("Test ONEWAY String!")).in());

                try {
                    server_objRef->raise_system_exception();
                } catch (const CORBA::NO_PERMISSION &ex) {
                    ex._tao_print_exception("<-> CLIENT::expected CORBA System Exception caught!");
                }
                try {
                    server_objRef->raise_user_exception();
                } catch (const taf_xmpl::test_exception &ex) {
                    ex._tao_print_exception("<-> CLIENT::expected User Exception caught!");
                }
            } catch(const CORBA::Exception &ex) {
                ex._tao_print_exception("<?> CLIENT: Server Failed - begin retry for Server IOR");
                server_objRef = taf_xmpl::SimpleServer::_nil();
            }

            DAF_OS::sleep(1);
        }
    } catch (const CORBA::Exception& ex) {
        ex._tao_print_exception ("CLIENT: UNEXPECTED exception caught - "); return -1;
    } else {
        ACE_ERROR_RETURN((LM_DEBUG, ACE_TEXT("(%04P|%04t) ERROR: CORBAClient is unable to pars arguments.\n")),-1);
    }

    return 0;
}
