
#include "TestGestaltService.h"
#include <ace/Service_Config.h>
#include <ace/Get_Opt.h>
#include <daf/OS.h>
#include <daf/DAFDebug.h>

ACE_FACTORY_DEFINE(TEST_SERVICE, test_TestGestaltService);
ACE_STATIC_SVC_DEFINE(test_TestGestaltService,
    test_TestGestaltService::svc_ident(),
    ACE_SVC_OBJ_T,
    &ACE_SVC_NAME(test_TestGestaltService),
    (ACE_Service_Type::DELETE_THIS | ACE_Service_Type::DELETE_OBJ),
    false); // Service not initially active

DAF_STREAM_DEFINE(TEST_SERVICE, test_TestGestaltStream);
DAF_MODULE_DEFINE(TEST_SERVICE, test_TestGestaltModule);

namespace test
{

int TestGestaltService::init_cnt = 0;
int TestGestaltService::fini_cnt = 0;
int TestGestaltService::resume_cnt = 0;
int TestGestaltService::suspend_cnt = 0;

void print_usage(void)
{
  ACE_DEBUG((LM_INFO, "TestGestaltService\n\t--help -h : Look at the code!!\n"));

}

const ACE_TCHAR *TestGestaltService::svc_ident(void)
{
  return "test_TestGestaltService";
}

TestGestaltService::TestGestaltService(void)
  : ret_errno(0)
  , ret_val(0)
  , svc_init_cnt(0)
  , svc_fini_cnt(0)
  , svc_resume_cnt(0)
  , svc_suspend_cnt(0)
  , svc_init_delay(0)
  , svc_fini_delay(0)
  , svc_resume_delay(0)
  , svc_suspend_delay(0)
  , debug(0)
{

}

TestGestaltService::~TestGestaltService(void)
{

}

int
TestGestaltService::init(int argc, ACE_TCHAR *argv[])
{
  init_cnt++;
  this->svc_init_cnt++;

  if ( this->parse_args(argc, argv) != 0 )
  {
    return -1;
  }
  if ( this->debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T TestGestaltService %s called delay=%d errno=%d rc=%d \n", __FUNCTION__, this->svc_init_delay, this->ret_errno, this->ret_val));


  ACE_Time_Value delay;
  delay.msec(int(this->svc_init_delay));
  ACE_OS::sleep(delay);

  DAF_OS::last_error(this->ret_errno);
  return this->ret_val;
}

int
TestGestaltService::fini(void)
{
  if ( this->debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T TestGestaltService %s called delay=%d errno=%d rc=%d \n", __FUNCTION__, this->svc_fini_delay, this->ret_errno, this->ret_val));
  fini_cnt++;
  this->svc_fini_cnt++;

  ACE_Time_Value delay;
  delay.msec(int(this->svc_fini_delay));
  ACE_OS::sleep(delay);
  if ( this->debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T TestGestaltService %s exiting \n", __FUNCTION__));

  DAF_OS::last_error(this->ret_errno);
  return this->ret_val;
}

int
TestGestaltService::suspend(void)
{
  if ( this->debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T TestGestaltService %s called delay=%d errno=%d rc=%d \n", __FUNCTION__, this->svc_suspend_delay, this->ret_errno, this->ret_val));
  suspend_cnt++;
  this->svc_suspend_cnt++;

  ACE_Time_Value delay;
  delay.msec(int(this->svc_suspend_delay));
  ACE_OS::sleep(delay);
  DAF_OS::last_error(this->ret_errno);
  if ( this->debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T TestGestaltService %s exit \n", __FUNCTION__));
  return this->ret_val;
}


int
TestGestaltService::resume(void)
{
  if ( this->debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T TestGestaltService %s called delay=%d errno=%d rc=%d \n", __FUNCTION__, this->svc_resume_delay, this->ret_errno, this->ret_val));
  resume_cnt++;
  this->svc_resume_cnt++;

  ACE_Time_Value delay;
  delay.msec(int(this->svc_resume_delay));
  ACE_OS::sleep(delay);
  DAF_OS::last_error(this->ret_errno);
  return this->ret_val;

}

int
TestGestaltService::parse_args(int argc, ACE_TCHAR *argv[])
{
  ACE_Get_Opt cli_opt(argc, argv, "hi:f:s:r:e:v:z", 0);
  cli_opt.long_option("help", 'h', ACE_Get_Opt::NO_ARG);
  cli_opt.long_option("init_delay", 'i', ACE_Get_Opt::ARG_REQUIRED);
  cli_opt.long_option("fini_delay", 'f', ACE_Get_Opt::ARG_REQUIRED);
  cli_opt.long_option("suspend_delay", 's', ACE_Get_Opt::ARG_REQUIRED);
  cli_opt.long_option("resume_delay", 'r', ACE_Get_Opt::ARG_REQUIRED);
  cli_opt.long_option("init_errno", 'e', ACE_Get_Opt::ARG_REQUIRED);
  cli_opt.long_option("init_return", 'v', ACE_Get_Opt::ARG_REQUIRED);
  cli_opt.long_option("debug", 'z', ACE_Get_Opt::NO_ARG);


  int option = -1;
  while ( (option = cli_opt()) != -1 ) switch(option) {
      case 'h': print_usage(); return -1;
      case 'i': this->svc_init_delay = DAF_OS::atoi(cli_opt.opt_arg()); break;
      case 'f': this->svc_fini_delay = DAF_OS::atoi(cli_opt.opt_arg()); break;
      case 'r': this->svc_resume_delay = DAF_OS::atoi(cli_opt.opt_arg()); break;
      case 's': this->svc_suspend_delay = DAF_OS::atoi(cli_opt.opt_arg()); break;
      case 'e': this->ret_errno = DAF_OS::atoi(cli_opt.opt_arg()); break;
      case 'v': this->ret_val = DAF_OS::atoi(cli_opt.opt_arg()); break;
      case 'z': this->debug = 10; break;

  }

  if ( this->debug )
  {
      ACE_DEBUG((LM_INFO, "(%P|%t) TestGestaltService\n\tinit_delay :%d\n\tfini_delay :%d\n\tresume_delay:%d\n\tsuspend_delay:%d\n"
                        , this->svc_init_delay
                        , this->svc_fini_delay
                        , this->svc_resume_delay
                        , this->svc_suspend_delay));
  }
  return 0;
}

TestGestaltModule::TestGestaltModule(void)

 : ACE_Module<ACE_SYNCH>("TestGestaltModule", new TestGestaltService, new TestGestaltService)
{
  if ( DAF::debug() )
  {
    ACE_DEBUG((LM_INFO, "(%P|%t) %T TestGestaltModule ctor\n"));
  }
}

TestGestaltModule::~TestGestaltModule(void)
{
  if ( DAF::debug() )
  {
    ACE_DEBUG((LM_INFO, "(%P|%t) %T TestGestaltModule dtor\n"));
  }
  //this->reader()->module_closed();
  //this->writer()->module_closed();

}

TestGestaltStream::TestGestaltStream(void)
{
  if ( DAF::debug() )
  {
    ACE_DEBUG((LM_INFO, "(%P|%t) %T TestGestaltStream ctor\n"));
  }
}

TestGestaltStream::~TestGestaltStream(void)
{
  if ( DAF::debug() )
  {
    ACE_DEBUG((LM_INFO, "(%P|%t) %T TestGestaltStream dtor\n"));
  }
}

}//namespace test
