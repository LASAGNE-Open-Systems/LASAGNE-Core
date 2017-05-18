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
#include "DDSSvcTest.h"
#include "ace/Get_Opt.h"
#include "dds/DDSPubSub.h"
#include "daf/ServiceGestalt.h"


ACE_FACTORY_DEFINE(DDS_INIT_TEST, test_DDSInitTestService);


namespace //anonymous
{
  const int DEFAULT_TIMER_VALUE = 10;//seconds

  void print_usage(void)
  {
    ACE_DEBUG((LM_INFO, "%s usage:\n\t--help, -h\t: print usage\n\t--svc, -s\t: Init ODDS inside Service Routine\n\t--debug, -z\t: Debug ON\n", test::DDSInitTestService::svc_ident() ));
  }


}//namespace anonymous


namespace test
{




  const char *DDSInitTestService::svc_ident(void)
  {
    return "test_DDSInitTestService";
  }


  DDSInitTestService::DDSInitTestService(void)
  : DAF::TaskExecutor()

  , service_(false)
  , debug_(0)
  {

  }

  DDSInitTestService::~DDSInitTestService(void)
  {

  }

  int
  DDSInitTestService::info(ACE_TCHAR **info_string, size_t length) const
  {
    static const char *info_desc =
    {
      "A Test Utility Service."
    };

    return (info_string ? (*info_string = ACE::strnnew(info_desc, length), 0) : -1);
  }

  int
  DDSInitTestService::init(int argc, ACE_TCHAR *argv[])
  {
    if ( this->parse_args(argc, argv) != 0 ) ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%t) %s Failed to parse args\n"), svc_ident()), -1);

    this->loaded_gestalt_ = ACE_Service_Config::current();


    if ( !this->service_ )
    {
      ACE_DEBUG((LM_INFO, "(%P|%t) %T Loading OpenDDS via INIT method \n"));
      ACE_DEBUG((LM_INFO, "(%P|%t) %T current %@ global %@\n", ACE_Service_Config::current(), ACE_Service_Config::global()));
      DAF::print_gestalt();

      TAFDDS::DDS_DomainParticipant p;
      if ( p.init(TAFDDS::DEFAULT_DOMAIN) != DDS::RETCODE_OK) ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%T) %T Failed to init Domain Participant\n")),-1);

      DAF::print_gestalt();
    }

    this->execute(1);

    return 0;
  }

  int
  DDSInitTestService::fini(void)
  {



    return 0;
  }


  int
  DDSInitTestService::resume(void)
  {


    return 0;
  }

  int
  DDSInitTestService::suspend(void)
  {


    return 0;

  }

  int
  DDSInitTestService::svc(void)
  {

    ACE_DEBUG((LM_INFO, "(%P|%T) %T Loading OpenDDS via SVC routine\n"));


    // Adding this guard will switch to the original TAFServer Gestalt.
    // But as its service code we are removing it to manifest our bug for FRAM-122
    //DAF_Service_Config_Guard svc_guard(this->loaded_gestalt_);

    ACE_DEBUG((LM_INFO, "(%P|%t) %T current %@ global %@\n", ACE_Service_Config::current(), ACE_Service_Config::global()));


    TAFDDS::DDS_DomainParticipant p;
    if ( p.init(TAFDDS::DEFAULT_DOMAIN) != DDS::RETCODE_OK) ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("(%P|%T) %T Failed to init Domain Participant\n")),-1);

    DAF::print_gestalt();

    //
    // The remainder is just to make sure the DDS stack works and is not part of the issue
    //
    // TODO
    return 0;
  }


  int
  DDSInitTestService::parse_args(int argc, ACE_TCHAR *argv[])
  {
    ACE_Get_Opt cli_opt(argc, argv, "hszt:", 0);

    cli_opt.long_option("help", 'h', ACE_Get_Opt::NO_ARG);
    cli_opt.long_option("svc", 's', ACE_Get_Opt::NO_ARG);
    cli_opt.long_option("debug", 'z', ACE_Get_Opt::NO_ARG);

    int option = -1;
    while ( (option = cli_opt()) != -1 ) switch(option) {
      case 'h': print_usage(); return -1;
      case 's': this->service_ = true; break;
      case 'z': this->debug_ = 1; break;

      break;
    }

    if ( this->debug_ )
    {
      ACE_DEBUG((LM_INFO, ACE_TEXT("%s Args:\n\tservice %d\n"), svc_ident(), this->service_));
    }

    return 0;
  }

}//namespace test
