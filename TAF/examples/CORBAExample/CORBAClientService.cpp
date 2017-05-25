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

#define TAF_CORBACLIENTSERVICE_CPP

#include "CORBAClientService.h"

#include "CORBAServiceS.h"
#include "CORBAService.h"

#include "ace/Get_Opt.h"
#include "taf/ObjectStubRef_T.h"
#include "taf/ORBManager.h"


ACE_FACTORY_DEFINE(CORBAClientService, TAF_CORBAClientService);

namespace TAF
{

namespace //anonymous
{
    class ClientCallback_impl : virtual public POA_taf_xmpl::SimpleCallback
    {
    public:
        virtual void callback_op(const char *text)
        {
            ACE_DEBUG((LM_DEBUG, ACE_TEXT("(%P|%t) <-- CLIENT::callback_op(%C)\n"), text));
        }
    };

    const int max_number_attempts = 10;

}//namespace anonymous

    CORBAClientService::CORBAClientService(void)
        : DAF::TaskExecutor()
        , ok_(1)
        , threads_(1)
    {

    }

    CORBAClientService::~CORBAClientService(void)
    {

    }

    int
    CORBAClientService::info(ACE_TCHAR **info_string, size_t length) const
    {
        static const char *info_desc =
        {
            "CORBAClientService is a simple client service to example the TAF framework services."
        };

        return (info_string ? (*info_string = ACE::strnnew(info_desc, length), 0) : -1);
    }


    int
    CORBAClientService::init(int argc, ACE_TCHAR *argv[])
    {
        if ( this->parse_args(argc, argv) == -1 )
        {
            ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) CORBAClientService - Failed to Parse Arguments\n")),-1);
        }

        // Spawn our Client Thread
        ACE_DEBUG((LM_INFO, ACE_TEXT("CORBAClientService - Starting %d Clients\n"), this->threads_));
        this->execute(this->threads_);

        return 0;
    }

    int
    CORBAClientService::fini(void)
    {
        this->ok_ = 0;

        this->wait();

        return 0;
    }

    int
    CORBAClientService::svc(void)
    {
        ACE_DEBUG((LM_INFO, "\n\t*********CORBAClientService %t************\n"));
        TAF::ObjectStubRef<ClientCallback_impl> callback(new ClientCallback_impl());

        taf_xmpl::SimpleServer_var server(taf_xmpl::SimpleServer::_nil());
        CORBA::Long test_value = 123;

        while(this->ok_)
        {

            for ( int i =0 ; CORBA::is_nil(server) && i < max_number_attempts && this->ok_; ++i) try           {
                CORBA::Object_var server_proxy(TAFResolveInitialReferences(taf_xmpl::MY_SERVICE_OID));

                if (CORBA::is_nil(server_proxy) )
                {
                    server_proxy = TAFResolveInitialReferences(TAF_XMPL::CORBAService::svc_ident());
                }

                if (CORBA::is_nil(server = taf_xmpl::SimpleServer::_narrow(server_proxy)))
                {
                    throw CORBA::TRANSIENT();
                }
                else
                {
                    break;
                }
            } catch (const CORBA::Exception& ) {
                ACE_DEBUG((LM_ERROR, "CLIENT SVC: SimpleServer Object reference is invalid - Retry(%d).\n", i));
                DAF_OS::sleep(2);
            }

            ACE_DEBUG((LM_INFO, "\n\t*****CLIENT SERVICE (%04t) %T*****\n"));
            try {
                server->test_val(test_value);

                CORBA::Long result = server->test_val();

                ACE_DEBUG((LM_INFO, "(%P|%04t) %T <-> %d == CLIENT::test_val(%d) %C\n", test_value, result, (test_value == result ? "SUCCESS" : "FAILURE")));

            } catch(const CORBA::Exception &ex) {
                ex._tao_print_exception("CLIENT SVC: UNEXPECTED exception -");
                server = taf_xmpl::SimpleServer::_nil();
            }

            DAF_OS::sleep(2);
        }

        return 0;
    }

    int
    CORBAClientService::parse_args(int argc, ACE_TCHAR *argv[])
    {
        ACE_Get_Opt cli_opt(argc, argv, "hn:z", 0);
        cli_opt.long_option("help",'h', ACE_Get_Opt::NO_ARG);
        cli_opt.long_option("threads" , 'n', ACE_Get_Opt::ARG_REQUIRED);
        cli_opt.long_option("debug",'z', ACE_Get_Opt::NO_ARG);

        int option = -1;
        while ( (option = cli_opt()) != -1 ) switch(option) {
            case 'h': ACE_DEBUG((LM_INFO, "Print the Help\n")); return -1;
            case 'n': this->threads_ = DAF_OS::atoi(cli_opt.opt_arg()); break;
            case 'z': break;
        }

        return 0;
    }

}//namespace TAF
