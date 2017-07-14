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
#include "TestService.h"
#include "ace/Dynamic_Service.h"
#include "ace/Reactor.h"
#include "taf/ORBManager.h"
#include "taf/TAFServer_impl.h"
#include "taf/TAFServerC.h"
#include "ace/Get_Opt.h"


ACE_FACTORY_DEFINE(TAF_TEST_SERVICE, test_TestService);


namespace //anonymous
{
    const int DEFAULT_TIMER_VALUE = 10;//seconds

    void print_usage(void)
    {
        ACE_DEBUG((LM_INFO, "%s usage:\n\t--help, -h\t: print usage\n\t--shutdown, -s\t: Shutdown TAFServer\n\t--secs,-t <sec>\t: Timeout for Shutdown <sec> seconds\n\t--debug, -z\t: Debug ON\n", test::TestService::svc_ident() ));
    }


}//namespace anonymous


namespace test
{


    struct TAFShutdownRunner : DAF::Runnable
    {
        TAF::TAFServer_impl *server;
        TAFShutdownRunner( TAF::TAFServer_impl *server_in)
            : server(server_in)
        {

        }

        int run(void)
        {
            if (server) server->shutdown();
            return 0;
        }
    };

    const char *TestService::svc_ident(void)
    {
        return "test_TestService";
    }


    TestService::TestService(void)
        : DAF::TaskExecutor()
        , timer_(DEFAULT_TIMER_VALUE,0)
        , shutdown_(false)
        , debug_(0)
    {

    }

    TestService::~TestService(void)
    {

    }

    int
    TestService::info(ACE_TCHAR **info_string, size_t length) const
    {
        static const char *info_desc =
        {
            "A Test Utility Service."
        };

        return (info_string ? (*info_string = ACE::strnnew(info_desc, length), 0) : -1);
    }

    int
    TestService::init(int argc, ACE_TCHAR *argv[])
    {
        if ( this->parse_args(argc, argv) != 0 ) ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) %s Failed to parse args\n"), svc_ident()), -1);


        if ( this->shutdown_ )
        {
            // Install Reactor Timeout
           this->timer_id_ = ACE_Reactor::instance()->schedule_timer(this, this, this->timer_);
        }


        return 0;
    }

    int
    TestService::fini(void)
    {

        if ( this->shutdown_ )
        {
            if ( ACE_Reactor::instance()->cancel_timer(this->timer_id_) )
            {
                // Log ?
            }
        }

        return 0;
    }


    int
    TestService::resume(void)
    {
        // what behaviour should we have here ?
        // cancel the Timer ?
        if ( this->shutdown_ )
        {
            if ( ACE_Reactor::instance()->cancel_timer(this->timer_id_) != 1 )
            {
                if ( this->debug_ ) ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %s Failed to suspend operation\n"), svc_ident()));
            }
        }

        return 0;
    }

    int
    TestService::suspend(void)
    {
        if (this->shutdown_ )
        {
            // TODO : ideally it would be good to keep track of how far our timer has gone.
            // But for the time being we will just re-schedule
            this->timer_id_ = ACE_Reactor::instance()->schedule_timer(this, this, this->timer_);
            if ( this->timer_id_ == -1 )
            {
                if ( this->debug_ ) ACE_DEBUG((LM_ERROR, ACE_TEXT("(%P|%t) %s Failed to resume operation (errno %d)\n"), svc_ident(), errno));
            }
        }

        return 0;

    }

    int
    TestService::handle_timeout(const ACE_Time_Value &, const void *)
    {
        // Find TAFServer
        // - We could do this via IOR Discovery Service, but it does give local TAFServers
        //   This might be good as a future feature if we want to kill multiple remote TAFServers
        // - We could do a Service lookup in the Global Gestalt

        //TODO : work out a ref-count way of doing this?
        TAF::TAFServer_impl *server = 0;
        {
            DAF_Service_Config_Guard svc_guard; ACE_UNUSED_ARG(svc_guard);
            server = ACE_Dynamic_Service<TAF::TAFServer_impl>::instance(taf::TAFSERVER_OID);
        }

        if (!server) {
            ACE_ERROR_RETURN((LM_ERROR,
                ACE_TEXT("(%P|%t) %s Failed to find TAFServer via Gestalt\n"), svc_ident()), -1);
        }

        std::stringstream stream;
        stream << svc_ident() << " called shutdown";
        server->sendConsoleMsg(stream.str().c_str());

        // Spawn a runnable onto the Singleton Executor. It we call it inline
        // the TAFServer will rip our service away from underneath us, which may create problems.
        // The Singleton Executor should be long living in the
        // ACE_Object_Manager;
        DAF::SingletonExecute(new TAFShutdownRunner(server));

        return -1;
    }

    int
    TestService::parse_args(int argc, ACE_TCHAR *argv[])
    {
        ACE_Get_Opt cli_opt(argc, argv, "hszt:", 0);

        cli_opt.long_option("help", 'h', ACE_Get_Opt::NO_ARG);
        cli_opt.long_option("shutdown", 's', ACE_Get_Opt::NO_ARG);
        cli_opt.long_option("debug", 'z', ACE_Get_Opt::NO_ARG);
        cli_opt.long_option("secs", 't', ACE_Get_Opt::ARG_REQUIRED);


        int option = -1;
        while ( (option = cli_opt()) != -1 ) switch(option) {
            case 'h': print_usage(); return -1;
            case 's': this->shutdown_ = true; break;
            case 'z': this->debug_ = 1; break;
            case 't': this->timer_ = ACE_Time_Value(DAF_OS::atoi(cli_opt.opt_arg()));
                      break;
        }

        if ( this->debug_ )
        {
            ACE_DEBUG((LM_INFO, ACE_TEXT("%s Args:\n\tShutdown %d\n\tTimeout %d\n"), svc_ident(), this->shutdown_, this->timer_.sec()));
        }

        return 0;
    }

}//namespace test
