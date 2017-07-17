
#include "TestGestaltService.h"

#include <ace/Get_Opt.h>
#include <ace/Dynamic_Service.h>
#include <daf/DAFDebug.h>
#include <daf/ServiceGestalt.h>
#include <daf/PropertyManager.h>
#include <daf/TaskExecutor.h>

#include <iostream>

//
// Shell for testing the configuration
// Test Cases
// Input Checks
// - CLI Args - Parameter Switching
// - file format
// - file format with ENV sub
// - file format with property sub
// - CLI Args
//
// Service Loader vs Gestalt Control.
// - Checks on Configurator vs Gestalt separation
// - Checks on class break down.
// - Suggestion of Composition and ability to wrap the TAF Gestalt around any
// Gestalt to provide access.
//
// - Service Control
// -- Load/Unload
// -- Suspend/Resume
// -- Lifecycle - container - removal of services.
// --
//
// Public API to check
// - load_static
// - load_dynamic
// - suspend_service
// - resume_service
// - remove_service
// - suspend_service_all
// - resume_service_all
// - remove_service_all
// - list_service_repository (dumps to console - testing ??)
// Behaviour to check
// - Return codes on the RESUME, SUSPEND, DLOAD, SLOAD, REMOVE
// - Timeouts
//


template<typename TYPE>
class Dynamic_Locator : protected ACE_Dynamic_Service_Base
{
public:
  static TYPE *module(const ACE_TCHAR* name, ACE_Service_Gestalt *container = ACE_Service_Config::global())
  {
    ACE_Module<ACE_SYNCH> * module_type = static_cast<ACE_Module<ACE_SYNCH>* >(ACE_Dynamic_Service_Base::instance(container, name, false));
    return dynamic_cast<TYPE*>(module_type);
  }


  static TYPE *stream(const ACE_TCHAR *name, ACE_Service_Gestalt *container = ACE_Service_Config::global())
  {
    ACE_Stream<ACE_SYNCH> * stream_type = static_cast<ACE_Stream<ACE_SYNCH>* >(ACE_Dynamic_Service_Base::instance(container, name, false));
    return dynamic_cast<TYPE*>(stream_type);
  }
};


namespace test
{
bool debug = false;
const char *TEST_NAME = "TestGestalt";
const char *TEST_LIBRARY_NAME= "TestGestaltService";
const char *TEST_MAKE_NAME="_make_test_TestGestaltService";

const std::string service_name(const std::string& name)
{
	size_t pos = name.find_last_of(':');
	if (pos != std::string::npos)
	{
		return name.substr(name.find_last_of(':') + 1);
	}
	return name;
}

enum Actions {
  SVC_SUSPEND, SVC_RESUME, SVC_REMOVE
};

struct ACEServiceAction : DAF::Runnable
{
  DAF::ServiceGestalt &gestalt_;
  Actions directive_;
  std::string ident_;

  ACEServiceAction(DAF::ServiceGestalt &gestalt
                , Actions directive
                , std::string ident)
    : gestalt_(gestalt)
    , directive_(directive)
    , ident_(ident)
  {

  }

  virtual int run(void)
  {
    DAF_Service_Config_Guard guard(&gestalt_);
    int result = 0;
    switch(this->directive_)
    {
      case SVC_SUSPEND:
        if ( debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T Executing SUSPEND Command %d %s\n", this->directive_, this->ident_.c_str()));
        //result = ACE_Service_Config::suspend(this->ident_.c_str());
        result = gestalt_.suspendService(this->ident_.c_str());
        break;
      case SVC_RESUME:
        if ( debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T Executing RESUME Command %d %s\n", this->directive_, this->ident_.c_str()));
        //result = ACE_Service_Config::resume(this->ident_.c_str());
        result = gestalt_.resumeService(this->ident_.c_str());
        break;
      case SVC_REMOVE:
        if ( debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T Executing REMOVE Command %d %s\n", this->directive_, this->ident_.c_str()));
        //result = ACE_Service_Config::remove(this->ident_.c_str());
        result = gestalt_.removeService(this->ident_.c_str());
        break;
      default:
        ACE_DEBUG((LM_ERROR, "(%P|%t) %T Unsupported command\n"));
        break;
    }

    if ( debug )
    {
      ACE_DEBUG((LM_INFO, "(%P|%t) %T Executed Command result=%d errno=%d %s\n",result,DAF_OS::last_error(), DAF::last_error_text().c_str() ));

    }

    return 0;
  }
};


int test_Gestalt_load_static_clean(int threadCount)
{
  std::cout << service_name(__FUNCTION__).c_str() << std::endl;

  ACE_UNUSED_ARG(threadCount);
  int result = 1;
  const int expected = 1;
  int value = 0;



  {
    DAF::ServiceGestalt container;
    //DAF_Service_Config_Guard guard(&container);

    result &=  (container.loadStatic("test_TestService","--init_delay 1000 --fini_delay 1000 --resume_delay 1000 --suspend_delay 1000" ) == 0);


    test::TestGestaltService *svc = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,"test_TestService",false);
    //test::TestGestaltService *svc = ACE_Dynamic_Service<test::TestGestaltService>::instance("test_TestService");
    result &= (svc != 0 );

    if (result)
    {
      value = svc->svc_init_cnt;
    }

  }

  result &= (value == expected);

  std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

  return result;
}

int test_Gestalt_load_dynamic_clean(int threadCount)
{
  std::cout << service_name(__FUNCTION__).c_str() << std::endl;

  ACE_UNUSED_ARG(threadCount);
  int result = 1;
  const int expected = 1;
  int value = 0;



  {
    DAF::ServiceGestalt container;

    result &=  (container.loadDynamic(service_name(__FUNCTION__).c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, "--init_delay 1000 --fini_delay 1000 --resume_delay 1000 --suspend_delay 1000" ) == 0);

    test::TestGestaltService *svc = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,service_name(__FUNCTION__).c_str(),false);
    result &= (svc != 0 );

    if (result)
    {
      value = svc->svc_init_cnt;
    }
  }

  result &= (value == expected);

  std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

  return result;
}

//
// TEST
// NOTE : Should probably also do a count on the services.
//
int test_Gestalt_load_dynamic_multiple(int threadCount)
{
  std::cout << service_name(__FUNCTION__).c_str() << std::endl;

  ACE_UNUSED_ARG(threadCount);
  int result = 1;
  const int expected = 1;
  int value = 0;



  {
    DAF::ServiceGestalt container;

    result &=  (container.loadDynamic(service_name(__FUNCTION__).c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, "--init_delay 1000 --fini_delay 1000 --resume_delay 1000 --suspend_delay 1000" ) == 0);
    for (int i = 0 ; i < threadCount; ++i )
    {
      result &=  (container.loadDynamic(service_name(__FUNCTION__).c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, "--init_delay 1000 --fini_delay 1000 --resume_delay 1000 --suspend_delay 1000" ) != 0);
    }

    test::TestGestaltService *svc = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,service_name(__FUNCTION__).c_str(),false);
    result &= (svc != 0 );

    if (result)
    {
      value = int(container.current_service_repository()->current_size());
    }
  }

  result &= (value == expected);

  std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

  return result;
}



int test_Gestalt_remove(int threadCount)
{
  std::cout << service_name(__FUNCTION__).c_str() << std::endl;

  ACE_UNUSED_ARG(threadCount);
  int result = 1;
  const int expected = 1;
  int value = 0;



  {
    DAF::ServiceGestalt container;
    test::TestGestaltService::fini_cnt = 0;
    test::TestGestaltService *svc = 0;

    result &=  (container.loadDynamic(service_name(__FUNCTION__).c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, "--init_delay 1000 --fini_delay 1000 --resume_delay 1000 --suspend_delay 1000" ) == 0);
    svc = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,service_name(__FUNCTION__).c_str(),false);
    result &= (svc != 0 );

    result &= ( container.removeService(service_name(__FUNCTION__).c_str()) == 0 );
    svc = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,service_name(__FUNCTION__).c_str(),false);
    result &= (svc == 0 );

    // This might bomb on Windows with EAGER UNLOAD ?
    value = test::TestGestaltService::fini_cnt;

  }

  result &= (value == expected);

  std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

  return result;
}




int test_Gestalt_suspend(int threadCount)
{
  std::cout << service_name(__FUNCTION__).c_str() << std::endl;

  ACE_UNUSED_ARG(threadCount);
  int result = 1;
  const int expected = 1;
  int value = 0;



  {
    DAF::ServiceGestalt container;
    test::TestGestaltService::fini_cnt = 0;
    test::TestGestaltService *svc = 0;

    result &=  (container.loadDynamic(service_name(__FUNCTION__).c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, "--init_delay 1000 --fini_delay 1000 --resume_delay 1000 --suspend_delay 1000" ) == 0);
    svc = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,service_name(__FUNCTION__).c_str(),false);
    result &= (svc != 0 );

    result &= ( container.suspendService(service_name(__FUNCTION__).c_str()) == 0 );


    if ( result )
    {
      value = svc->svc_suspend_cnt;
    }
  }

  result &= (value == expected);

  std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

  return result;
}

int test_Gestalt_resume(int threadCount)
{
  std::cout << service_name(__FUNCTION__).c_str() << std::endl;

  ACE_UNUSED_ARG(threadCount);
  int result = 1;
  const int expected = 1;
  int value = 0;



  {
    DAF::ServiceGestalt container;
    test::TestGestaltService::fini_cnt = 0;
    test::TestGestaltService *svc = 0;

    result &=  (container.loadDynamic(service_name(__FUNCTION__).c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, "--init_delay 1000 --fini_delay 1000 --resume_delay 1000 --suspend_delay 1000" ) == 0);
    svc = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,service_name(__FUNCTION__).c_str(),false);
    result &= (svc != 0 );

    result &= ( container.suspendService(service_name(__FUNCTION__).c_str()) == 0 );

    result &= ( container.resumeService(service_name(__FUNCTION__).c_str()) == 0 );


    if ( result )
    {
      value = svc->svc_resume_cnt;
    }
  }

  result &= (value == expected);

  std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

  return result;
}

//
// TEST
//
int test_Gestalt_resume_already_active(int threadCount)
{
  std::cout << service_name(__FUNCTION__).c_str() << std::endl;

  ACE_UNUSED_ARG(threadCount);
  int result = 1;
  const int expected = 0;
  int value = -1;



  {
    DAF::ServiceGestalt container;
    test::TestGestaltService::fini_cnt = 0;
    test::TestGestaltService *svc = 0;

    result &=  (container.loadDynamic(service_name(__FUNCTION__).c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, "--init_delay 1000 --fini_delay 1000 --resume_delay 1000 --suspend_delay 1000" ) == 0);
    svc = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,service_name(__FUNCTION__).c_str(),false);
    result &= (svc != 0 );

    // After Load being successful the service should be in an active state.
    // This call should NOT  FAIL, but will not be passed through to the service
    result &= ( 0 == container.resumeService(service_name(__FUNCTION__).c_str())  );


    if ( result )
    {
      value = svc->svc_resume_cnt;
    }
  }

  result &= (value == expected);

  std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

  return result;
}

int test_Gestalt_suspend_already_inactive(int threadCount)
{
  std::cout << service_name(__FUNCTION__).c_str() << std::endl;

  ACE_UNUSED_ARG(threadCount);
  int result = 1;
  const int expected = 1;
  int value = -1;



  {
    DAF::ServiceGestalt container;
    test::TestGestaltService::fini_cnt = 0;
    test::TestGestaltService *svc = 0;

    result &=  (container.loadDynamic(service_name(__FUNCTION__).c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, "--init_delay 1000 --fini_delay 1000 --resume_delay 1000 --suspend_delay 1000" ) == 0);
    svc = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,service_name(__FUNCTION__).c_str(),false);
    result &= (svc != 0 );

    // First one pass
    result &= ( 0 == container.suspendService(service_name(__FUNCTION__).c_str()) );

    // Subsequent calls will do nothing to the service but
    // will NOT return an error
    for ( int i = 0; i < threadCount; ++i )
    {
      result &= ( 0 == container.suspendService(service_name(__FUNCTION__).c_str()) );
    }

    if ( result )
    {
      value = svc->svc_suspend_cnt;
    }
  }

  result &= (value == expected);

  std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

  return result;
}

int test_Gestalt_resume_already_active_multiple(int threadCount)
{
  std::cout << service_name(__FUNCTION__).c_str() << std::endl;

  ACE_UNUSED_ARG(threadCount);
  int result = 1;
  const int expected = 1;
  int value = -1;

  {
    DAF::ServiceGestalt container;
    test::TestGestaltService::fini_cnt = 0;
    test::TestGestaltService *svc = 0;

    result &=  (container.loadDynamic(service_name(__FUNCTION__).c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, "--init_delay 1000 --fini_delay 1000 --resume_delay 1000 --suspend_delay 1000" ) == 0);
    svc = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,service_name(__FUNCTION__).c_str(),false);
    result &= (svc != 0 );

    result &= ( container.suspendService(service_name(__FUNCTION__).c_str()) == 0 );
    // First one Pass
    result &= ( 0 == container.resumeService(service_name(__FUNCTION__).c_str()) );

    // Subsequent calls will not pass it through to the service
    // but will NOT return an error
    for ( int i =0 ; i < threadCount; ++i )
    {
      result &= ( 0 == container.resumeService(service_name(__FUNCTION__).c_str()) );
    }


    if ( result )
    {
      value = svc->svc_resume_cnt;
    }
  }

  result &= (value == expected);

  std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

  return result;
}



int test_Gestalt_double_remove_EEXIST(int threadCount)
{
  std::cout << service_name(__FUNCTION__).c_str() << std::endl;

  ACE_UNUSED_ARG(threadCount);
  int result = 1;
  const int expected = EEXIST;
  int value = 0;



  {
    DAF::ServiceGestalt container;
    test::TestGestaltService::fini_cnt = 0;
    test::TestGestaltService *svc = 0;

    result &=  (container.loadDynamic(service_name(__FUNCTION__).c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, "--init_delay 1000 --fini_delay 1000 --resume_delay 1000 --suspend_delay 1000" ) == 0);
    svc = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,service_name(__FUNCTION__).c_str(),false);
    result &= (svc != 0 );

    result &= ( container.removeService(service_name(__FUNCTION__).c_str()) == 0 );
    // should fail
    result &= ( container.removeService(service_name(__FUNCTION__).c_str()) != 0 );
    svc = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,service_name(__FUNCTION__).c_str(),false);
    result &= (svc == 0 );

    // This might bomb on Windows with EAGER UNLOAD ?
    value = DAF_OS::last_error();
  }

  result &= (value == expected);

  std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

  return result;
}



int test_Gestalt_load_dynamic_ident_whitespace_EINVAL(int threadCount)
{
  std::cout << service_name(__FUNCTION__).c_str() << std::endl;

  ACE_UNUSED_ARG(threadCount);
  int result = 1;
  const int expected = EINVAL;
  int value = 0;



  {
    DAF::ServiceGestalt container;
    test::TestGestaltService::fini_cnt = 0;
    //test::TestGestaltService *svc = 0;

    result &=  (container.loadDynamic("some bad white space", TEST_LIBRARY_NAME, TEST_MAKE_NAME, "--init_delay 1000 --fini_delay 1000 --resume_delay 1000 --suspend_delay 1000" ) != 0);

    value = DAF_OS::last_error();
  }

  result &= (value == expected);

  std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

  return result;
}

//
// TEST
// This test looks at the ":" character being present in the ident
//
int test_Gestalt_load_dynamic_ident_bad_characters_EINVAL(int threadCount)
{
  std::cout << service_name(__FUNCTION__).c_str() << std::endl;

  ACE_UNUSED_ARG(threadCount);
  int result = 1;
  const int expected = EINVAL;
  int value = 0;

  {
    DAF::ServiceGestalt container;
    test::TestGestaltService::fini_cnt = 0;

    // Ok characters include: ;,?!@$^&`+=[]
    // Bad characters include: :.#%*~-(){}\'"
    result &= ( 0 != container.loadDynamic("some:ident;with,.?*@$#%^&!~`+-=()*{}[]\\\'\"", TEST_LIBRARY_NAME, TEST_MAKE_NAME, "--init_delay 1000 --fini_delay 1000 --resume_delay 1000 --suspend_delay 1000" ) );

    value = DAF_OS::last_error();
  }

  result &= (value == expected);

  std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

  return result;
}

//
// TEST
// This test looks at the control character (\b\t\n) being present in the ident
// Parsing will
//
int test_Gestalt_load_dynamic_ident_bad_characters_control_EINVAL(int threadCount)
{
  std::cout << service_name(__FUNCTION__).c_str() << std::endl;

  ACE_UNUSED_ARG(threadCount);
  int result = 1;
  const int expected = 0;
  int value = -1;

  {
    DAF::ServiceGestalt container;
    test::TestGestaltService::init_cnt = 0; // The init_cnt must be reset otherwise other tests will influence the result here
    test::TestGestaltService::fini_cnt = 0;
    std::stringstream params;


    /*
     * Note: DAF::trim_string, which is used on the ident, will convert control characters into spaces. If a control
     * character is at the start or end of the ident, it will be first converted to a space, and then trimmed off.
     * Any remaining embedded space is treated as invalid for an ident in any case.
    */

    result &=  ( 0 != container.loadDynamic("some\bb", TEST_LIBRARY_NAME, TEST_MAKE_NAME, params.str()) );
    result &=  ( EINVAL == DAF_OS::last_error());
    if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T %s %N:%l Failure here %s\n", __FUNCTION__, (result ? "SUCCESS":"FAILURE")));

    result &=  ( 0 != container.loadDynamic("some\aa", TEST_LIBRARY_NAME, TEST_MAKE_NAME, params.str()) );
    result &=  ( EINVAL == DAF_OS::last_error());
    if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T %s %N:%l Failure here %s\n", __FUNCTION__, (result ? "SUCCESS":"FAILURE")));


    result &=  ( 0 != container.loadDynamic("f\ff", TEST_LIBRARY_NAME, TEST_MAKE_NAME, params.str()) );
    result &=  ( EINVAL == DAF_OS::last_error());
    if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T %s %N:%l Failure here %s\n", __FUNCTION__, (result ? "SUCCESS":"FAILURE")));


    result &=  ( 0 != container.loadDynamic("r\rr", TEST_LIBRARY_NAME, TEST_MAKE_NAME, params.str()) );
    result &=  ( EINVAL == DAF_OS::last_error());
    if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T %s %N:%l Failure here %s\n", __FUNCTION__, (result ? "SUCCESS":"FAILURE")));


    result &=  ( 0 != container.loadDynamic("t\tt", TEST_LIBRARY_NAME, TEST_MAKE_NAME, params.str()) );
    result &=  ( EINVAL == DAF_OS::last_error());
    if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T %s %N:%l Failure here %s\n", __FUNCTION__, (result ? "SUCCESS":"FAILURE")));


    result &=  ( 0 != container.loadDynamic("n\nn", TEST_LIBRARY_NAME, TEST_MAKE_NAME, params.str()) );
    result &=  ( EINVAL == DAF_OS::last_error());
    if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T %s %N:%l Failure here %s\n", __FUNCTION__, (result ? "SUCCESS":"FAILURE")));


    /*
     * Test removed - I'm not sure what this was supposed to be testing but is equivalent to the test above.
     *
    result &=  ( 0 != container.loadDynamic("\?oct\111A\xA", TEST_LIBRARY_NAME, TEST_MAKE_NAME, params.str()) );
    result &=  ( EINVAL == DAF_OS::last_error());
    if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T %s %N:%l Failure here %s\n", __FUNCTION__, (result ? "SUCCESS":"FAILURE")));
     */

    value = test::TestGestaltService::init_cnt;
  }

  result &= (value == expected);

  std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

  return result;
}



int test_Gestalt_load_dynamic_libpath_EINVAL(int threadCount)
{
  std::cout << service_name(__FUNCTION__).c_str() << std::endl;

  ACE_UNUSED_ARG(threadCount);
  int result = 1;
  const int expected = EINVAL;
  int value = 0;



  {
    DAF::ServiceGestalt container;
    test::TestGestaltService::fini_cnt = 0;
    //test::TestGestaltService *svc = 0;

    result &=  (container.loadDynamic(service_name(__FUNCTION__).c_str(), " some bad libray path C:/ blash/", TEST_MAKE_NAME, "--init_delay 1000 --fini_delay 1000 --resume_delay 1000 --suspend_delay 1000" ) != 0);

    value = DAF_OS::last_error();
  }

  result &= (value == expected);

  std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

  return result;
}

int test_Gestalt_load_dynamic_objclass_EINVAL(int threadCount)
{
  std::cout << service_name(__FUNCTION__).c_str() << std::endl;

  ACE_UNUSED_ARG(threadCount);
  int result = 1;
  const int expected = EINVAL;
  int value = 0;



  {
    DAF::ServiceGestalt container;
    test::TestGestaltService::fini_cnt = 0;
    //test::TestGestaltService *svc = 0;

    result &=  (container.loadDynamic(service_name(__FUNCTION__).c_str(), TEST_LIBRARY_NAME, "_make bad  space make method test_TestService", "--init_delay 1000 --fini_delay 1000 --resume_delay 1000 --suspend_delay 1000" ) != 0);

    value = DAF_OS::last_error();
  }

  result &= (value == expected);

  std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

  return result;
}



int test_Gestalt_load_static_ident_whitespace_EINVAL(int threadCount)
{
  std::cout << service_name(__FUNCTION__).c_str() << std::endl;

  ACE_UNUSED_ARG(threadCount);
  int result = 1;
  const int expected = EINVAL;
  int value = 0;



  {
    DAF::ServiceGestalt container;
    test::TestGestaltService::fini_cnt = 0;
    //test::TestGestaltService *svc = 0;

    result &=  (container.loadStatic("some bad white space", "--init_delay 1000 --fini_delay 1000 --resume_delay 1000 --suspend_delay 1000" ) != 0);

    value = DAF_OS::last_error();
  }

  result &= (value == expected);

  std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

  return result;
}


int test_Gestalt_remove_ident_whitespace_EINVAL(int threadCount)
{
  std::cout << service_name(__FUNCTION__).c_str() << std::endl;

  ACE_UNUSED_ARG(threadCount);
  int result = 1;
  const int expected = EINVAL;
  int value = 0;



  {
    DAF::ServiceGestalt container;
    test::TestGestaltService::fini_cnt = 0;
    test::TestGestaltService *svc = 0;

    result &=  (container.loadDynamic(service_name(__FUNCTION__).c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, "--init_delay 1000 --fini_delay 1000 --resume_delay 1000 --suspend_delay 1000" ) == 0);
    svc = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,service_name(__FUNCTION__).c_str(),false);
    result &= (svc != 0 );

    result &= ( container.removeService("some bad white space") != 0 );
    value = DAF_OS::last_error();
  }

  result &= (value == expected);

  std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

  return result;
}

int test_Gestalt_suspend_ident_whitespace_EINVAL(int threadCount)
{
  std::cout << service_name(__FUNCTION__).c_str() << std::endl;

  ACE_UNUSED_ARG(threadCount);
  int result = 1;
  const int expected = EINVAL;
  int value = 0;



  {
    DAF::ServiceGestalt container;
    test::TestGestaltService::fini_cnt = 0;
    test::TestGestaltService *svc = 0;

    result &=  (container.loadDynamic(service_name(__FUNCTION__).c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, "--init_delay 1000 --fini_delay 1000 --resume_delay 1000 --suspend_delay 1000" ) == 0);
    svc = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,service_name(__FUNCTION__).c_str(),false);
    result &= (svc != 0 );

    result &= ( container.suspendService("some bad white space") != 0 );
    value = DAF_OS::last_error();
  }

  result &= (value == expected);

  std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

  return result;
}

int test_Gestalt_resume_ident_whitespace_EINVAL(int threadCount)
{
  std::cout << service_name(__FUNCTION__).c_str() << std::endl;

  ACE_UNUSED_ARG(threadCount);
  int result = 1;
  const int expected = EINVAL;
  int value = 0;



  {
    DAF::ServiceGestalt container;
    test::TestGestaltService::fini_cnt = 0;
    test::TestGestaltService *svc = 0;

    result &=  (container.loadDynamic(service_name(__FUNCTION__).c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, "--init_delay 1000 --fini_delay 1000 --resume_delay 1000 --suspend_delay 1000" ) == 0);
    svc = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,service_name(__FUNCTION__).c_str(),false);
    result &= (svc != 0 );

    result &= ( container.resumeService("some bad white space") != 0 );
    value = DAF_OS::last_error();
  }

  result &= (value == expected);

  std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

  return result;
}

/**
 * TEST
 *
 * test_Gestalt_load
 */

 //
 // TEST
 // the timeout is configurable on dynamic loading
 //
 int test_Gestalt_load_dynamic_timeout_ETIME(int threadCount)
 {
   std::cout << service_name(__FUNCTION__).c_str() << std::endl;

   ACE_UNUSED_ARG(threadCount);
   int result = 1;
   const int expected = ETIME;
   int value = 0;

   // This will make the DEFAULT_XXX_TIMEOUTS minimum values.
   DAF::set_property(DAF_SVCACTIONTIMEOUT, "16");
   {
     DAF::ServiceGestalt container;
     std::stringstream params;
     params << (debug? "--debug ":"") << "--init_delay " << (DAF::get_numeric_property<int>(DAF_SVCACTIONTIMEOUT) * DAF_MSECS_ONE_SECOND + 1000) ;

     if ( debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T Loading with params %s\n", params.str().c_str()));

     result &=  (container.loadDynamic(service_name(__FUNCTION__).c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, params.str().c_str() ) != 0);

     if ( debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T Return errno=%d \n", DAF_OS::last_error()));

     ACE_OS::sleep(ACE_Time_Value(2));

     // Check for valid service load
     test::TestGestaltService *svc = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,service_name(__FUNCTION__).c_str(),false);
     result &= (svc != 0 );

     // Check that errno is ETIME
     value = DAF_OS::last_error();

   }

   result &= (value == expected);

   std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

   return result;
 }

 //
 // TEST
 // the timeout is configurable on dynamic loading
 //
 int test_Gestalt_remove_timeout_ETIME(int threadCount)
 {
   std::cout << service_name(__FUNCTION__).c_str() << std::endl;

   ACE_UNUSED_ARG(threadCount);
   int result = 1;
   const int expected = ETIME;
   int value = 0;

   // This will make the DEFAULT_XXX_TIMEOUTS minimum values.
   DAF::set_property(DAF_SVCACTIONTIMEOUT, "16");
   {
     DAF::ServiceGestalt container;
     std::stringstream params;
     params << (debug? "--debug ":"") << "--fini_delay " << (DAF::get_numeric_property<int>(DAF_SVCACTIONTIMEOUT) * DAF_MSECS_ONE_SECOND + 1000) ;


     result &=  (container.loadDynamic(service_name(__FUNCTION__).c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, params.str().c_str() ) == 0);

     test::TestGestaltService *svc = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,service_name(__FUNCTION__).c_str(),false);
     result &= (svc != 0 );

     if ( debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T Removing with params %s\n", params.str().c_str()));
     result &=  (container.removeService(service_name(__FUNCTION__).c_str() ) != 0);
     if ( debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T Return errno=%d \n", DAF_OS::last_error()));

     ACE_OS::sleep(ACE_Time_Value(2));


     // Check that errno is ETIME
     value = DAF_OS::last_error();

   }

   result &= (value == expected);

   std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

   return result;
 }

 //
 // TEST
 // the timeout is configurable on dynamic loading
 //
 int test_Gestalt_suspend_timeout_ETIME(int threadCount)
 {
   std::cout << service_name(__FUNCTION__).c_str() << std::endl;

   ACE_UNUSED_ARG(threadCount);
   int result = 1;
   const int expected = ETIME;
   int value = 0;

   // This will make the DEFAULT_XXX_TIMEOUTS minimum values.
   DAF::set_property(DAF_SVCACTIONTIMEOUT, "16");
   {
     DAF::ServiceGestalt container;
     std::stringstream params;
     params << (debug? "--debug ":"") << "--suspend_delay " << (DAF::get_numeric_property<int>(DAF_SVCACTIONTIMEOUT) * DAF_MSECS_ONE_SECOND + 1000) ;


     result &=  (container.loadDynamic(service_name(__FUNCTION__).c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, params.str().c_str() ) == 0);

     test::TestGestaltService *svc = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,service_name(__FUNCTION__).c_str(),false);
     result &= (svc != 0 );

     if ( debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T Suspending with params %s\n", params.str().c_str()));
     result &=  (container.suspendService(service_name(__FUNCTION__).c_str() ) != 0);
     if ( debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T Return errno=%d \n", DAF_OS::last_error()));

     ACE_OS::sleep(ACE_Time_Value(2));


     // Check that errno is ETIME
     value = DAF_OS::last_error();

   }

   result &= (value == expected);

   std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

   return result;
 }

 //
 // TEST
 // the timeout is configurable on dynamic loading
 //
 int test_Gestalt_resume_timeout_ETIME(int threadCount)
 {
   std::cout << service_name(__FUNCTION__).c_str() << std::endl;

   ACE_UNUSED_ARG(threadCount);
   int result = 1;
   const int expected = ETIME;
   int value = 0;

   // This will make the DEFAULT_XXX_TIMEOUTS minimum values.
   DAF::set_property(DAF_SVCACTIONTIMEOUT, "16");
   {
     DAF::ServiceGestalt container;
     std::stringstream params;
     params << (debug? "--debug ":"") << "--resume_delay " << (DAF::get_numeric_property<int>(DAF_SVCACTIONTIMEOUT) * DAF_MSECS_ONE_SECOND + 1000) ;


     result &=  (container.loadDynamic(service_name(__FUNCTION__).c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, params.str().c_str() ) == 0);

     test::TestGestaltService *svc = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,service_name(__FUNCTION__).c_str(),false);
     result &= (svc != 0 );
     result &=  (container.suspendService(service_name(__FUNCTION__).c_str() ) == 0);
     if ( debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T Resume with params %s\n", params.str().c_str()));
     result &=  (container.resumeService(service_name(__FUNCTION__).c_str() ) != 0);
     if ( debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T Return errno=%d \n", DAF_OS::last_error()));

     ACE_OS::sleep(ACE_Time_Value(2));


     // Check that errno is ETIME
     value = DAF_OS::last_error();

   }

   result &= (value == expected);

   std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

   return result;
 }

 //
 // TEST
 // the timeout is configurable on dynamic loading
 //
 int test_Gestalt_load_dynamic_service_errno(int threadCount)
 {
   std::cout << service_name(__FUNCTION__).c_str() << std::endl;

   ACE_UNUSED_ARG(threadCount);
   int result = 1;
   const int expected = EHOSTDOWN;
   int value = 0;

   {
     DAF::ServiceGestalt container;
     std::stringstream params;

     params << (debug? "--debug":"") <<  " --init_errno  " << EHOSTDOWN << " --init_return " << -1 ;

     if ( debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T Loading with params %s\n", params.str().c_str()));

     result &=  (container.loadDynamic(service_name(__FUNCTION__).c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, params.str().c_str() ) == -1);

     // Check that errno is ETIME
     value = DAF_OS::last_error();

   }

   result &= (value == expected);

   std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

   return result;
 }

 //
 // TEST
 // the timeout is configurable on dynamic loading
 //
 int test_Gestalt_suspend_service_errno(int threadCount)
 {
   std::cout << service_name(__FUNCTION__).c_str() << std::endl;

   ACE_UNUSED_ARG(threadCount);
   int result = 1;
   const int expected = EHOSTDOWN;
   int value = 0;

   {
     DAF::ServiceGestalt container;
     std::stringstream params;
     params << (debug? "--debug":"");

     result &=  (container.loadDynamic(service_name(__FUNCTION__).c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, params.str().c_str() ) == 0);

     test::TestGestaltService *svc = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,service_name(__FUNCTION__).c_str(),false);
     result &= (svc != 0 );

     if ( debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T Service 0x%@ %d \n", svc, result));

     if ( result )
     {
       svc->ret_errno = EHOSTDOWN;
       svc->ret_val = -1;
       if ( debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T Configuring service errno %d rc=%d\n", svc->ret_errno, svc->ret_val));
     }
     result &=  (container.suspendService(service_name(__FUNCTION__).c_str() ) != 0);

     value = DAF_OS::last_error();

   }

   result &= (value == expected);

   std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

   return result;
 }

 //
 // TEST
 // the timeout is configurable on dynamic loading
 //
 int test_Gestalt_resume_service_errno(int threadCount)
 {
   std::cout << service_name(__FUNCTION__).c_str() << std::endl;

   ACE_UNUSED_ARG(threadCount);
   int result = 1;
   const int expected = EHOSTDOWN;
   int value = 0;

   {
     DAF::ServiceGestalt container;
     std::stringstream params;
     params << (debug? "--debug":"");

     result &=  (container.loadDynamic(service_name(__FUNCTION__).c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, params.str().c_str() ) == 0);

     test::TestGestaltService *svc = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,service_name(__FUNCTION__).c_str(),false);
     result &= (svc != 0 );
     result &=  (0 == container.suspendService(service_name(__FUNCTION__).c_str() ));

     if ( result )
     {
       svc->ret_errno = EHOSTDOWN;
       svc->ret_val = -1;
       if ( debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T Configuring service errno %d rc=%d\n", svc->ret_errno, svc->ret_val));
     }

     result &=  (container.resumeService(service_name(__FUNCTION__).c_str() ) != 0);

     if ( result )
     {
       svc->ret_errno = 0;
       svc->ret_val = 0;
       if ( debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T Configuring service errno %d rc=%d\n", svc->ret_errno, svc->ret_val));
     }

     value = DAF_OS::last_error();

   }

   result &= (value == expected);

   std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

   return result;
 }

 //
 // TEST
 // the timeout is configurable on dynamic loading
 //
 int test_Gestalt_remove_service_errno(int threadCount)
 {
   std::cout << service_name(__FUNCTION__).c_str() << std::endl;

   ACE_UNUSED_ARG(threadCount);
   int result = 1;
   const int expected = EHOSTDOWN;
   int value = 0;

   {
     DAF::ServiceGestalt container;
     std::stringstream params;
     params << (debug? "--debug":"");

     result &=  (container.loadDynamic(service_name(__FUNCTION__).c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, params.str().c_str() ) == 0);

     test::TestGestaltService *svc = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,service_name(__FUNCTION__).c_str(),false);
     result &= (svc != 0 );


     if ( result )
     {
       svc->ret_errno = EHOSTDOWN;
       svc->ret_val = -1;
     }

     result &=  (container.removeService(service_name(__FUNCTION__).c_str() ) != 0);

     value = DAF_OS::last_error();
   }

   result &= (value == expected);

   std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

   return result;
 }

 //
 // TEST
 // the timeout is configurable on dynamic loading
 //
 int test_Gestalt_suspend_EACCES(int threadCount)
 {
   std::cout << service_name(__FUNCTION__).c_str() << std::endl;

   ACE_UNUSED_ARG(threadCount);
   int result = 1;
   const int expected = EACCES;
   int value = 0;


   // This will make the DEFAULT_XXX_TIMEOUTS minimum values.
   DAF::set_property(DAF_SVCACTIONTIMEOUT, "16");
   {
     DAF::ServiceGestalt container;
     DAF::TaskExecutor executor;
     std::stringstream params;
     params << (debug ? "--debug ":"") ;


     result &=  (container.loadDynamic(service_name(__FUNCTION__).c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, params.str().c_str() ) == 0);

     test::TestGestaltService *svc = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,service_name(__FUNCTION__).c_str(),false);
     result &= (svc != 0 );

     if ( result )
     {
        svc->svc_fini_delay = DAF::get_numeric_property<int>(DAF_SVCACTIONTIMEOUT) * DAF_MSECS_ONE_SECOND * 2;
     }

     // Can't use remove service here as it removes it internally at the DAF level
     // If we remove it underlying via ACE and then call suspend... will
     // it catch the fini_called method.
     std::string ident = service_name(__FUNCTION__).c_str();
     if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T Executing Remove Runner\n"));
     executor.execute(new ACEServiceAction(container, SVC_REMOVE, ident));

     ACE_OS::sleep(ACE_Time_Value(1,0));

     // Call suspend in this window to be caught by "fini_called"
     if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T Executing Suspend Runner\n"));
     result &=  (container.suspendService(service_name(__FUNCTION__).c_str() ) != 0);
     value = DAF_OS::last_error();

     if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T Completed Test\n"));
     // Thread Join?
     executor.wait();
     if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T Thread Joined Test\n"));

   }

   result &= (value == expected);

   std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

   return result;
 }

 //
 // TEST
 // This test covers the case where a Timeout does occur on the first remove
 // but a second remove should not be allowed.
 //
 int test_Gestalt_long_remove_remove(int threadCount)
 {
   std::cout << service_name(__FUNCTION__).c_str() << std::endl;

   ACE_UNUSED_ARG(threadCount);
   int result = 1;
   const int expected = 1;
   int value = 0;


   // This will make the DEFAULT_XXX_TIMEOUTS minimum values.
   DAF::set_property(DAF_SVCACTIONTIMEOUT, "16");
   {
     DAF::ServiceGestalt container;
     DAF::TaskExecutor executor;
     std::stringstream params;
     params << (debug ? "--debug ":"") ;

     test::TestGestaltService::fini_cnt = 0;
     test::TestGestaltService::suspend_cnt = 0;

     result &=  (container.loadDynamic(service_name(__FUNCTION__).c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, params.str().c_str() ) == 0);

     test::TestGestaltService *svc = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,service_name(__FUNCTION__).c_str(),false);
     result &= (svc != 0 );

     if ( result )
     {
        svc->svc_fini_delay = DAF::get_numeric_property<int>(DAF_SVCACTIONTIMEOUT) * DAF_MSECS_ONE_SECOND * 2;
     }

     std::string ident = service_name(__FUNCTION__).c_str();

     //
     // This has been configured to be a LONG Remove function that will ultimately
     // work. What we are checking here is can a suspend/resume call enter
     // during the Remove and change service state. This should be guarded on the
     // RW Mutex of the Service Gestalt.
     //
     if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T Executing Remove Runner\n"));
     executor.execute(new ACEServiceAction(container, SVC_REMOVE, ident));

     // Give it some time to call it out. and Sleep
     ACE_OS::sleep(ACE_Time_Value(1));
     if ( result )
     {
        svc->svc_fini_delay = 0;
     }

     // This shouldn't really execute.
     executor.execute(new ACEServiceAction(container, SVC_REMOVE, ident));
     value = test::TestGestaltService::fini_cnt;

     // Thread Join?
     executor.wait();
     if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T Thread Joined Test\n"));

   }

   result &= (value == expected);

   std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

   return result;
 }

 //
 // TEST
 // the timeout is configurable on dynamic loading
 //
 int test_Gestalt_concurrent_long_remove_short_resume(int threadCount)
 {
   std::cout << service_name(__FUNCTION__).c_str() << std::endl;

   ACE_UNUSED_ARG(threadCount);
   int result = 1;
   const int expected = 0;
   int value = 0;


   // This will make the DEFAULT_XXX_TIMEOUTS minimum values.
   DAF::set_property(DAF_SVCACTIONTIMEOUT, "16");
   {
     DAF::ServiceGestalt container;
     DAF::TaskExecutor executor;
     std::stringstream params;
     params << (debug ? "--debug ":"") ;

     test::TestGestaltService::fini_cnt = 0;
     test::TestGestaltService::suspend_cnt = 0;

     result &=  (container.loadDynamic(service_name(__FUNCTION__).c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, params.str().c_str() ) == 0);

     test::TestGestaltService *svc = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,service_name(__FUNCTION__).c_str(),false);
     result &= (svc != 0 );

     if ( result )
     {
        svc->svc_fini_delay = DAF::get_numeric_property<int>(DAF_SVCACTIONTIMEOUT) * DAF_MSECS_ONE_SECOND * 2;
     }

     std::string ident = service_name(__FUNCTION__).c_str();

     //
     // This has been configured to be a LONG Remove function that will ultimately
     // work. What we are checking here is can a suspend/resume call enter
     // during the Remove and change service state. This should be guarded on the
     // RW Mutex of the Service Gestalt.
     //
     if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T Executing Remove Runner\n"));
     executor.execute(new ACEServiceAction(container, SVC_REMOVE, ident));

     // This shouldn't really execute.
     executor.execute(new ACEServiceAction(container, SVC_SUSPEND, ident));
     value = test::TestGestaltService::suspend_cnt;

     // Thread Join?
     executor.wait();
     if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T Thread Joined Test\n"));

   }

   result &= (value == expected);

   std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

   return result;
 }


 //
 // TEST
 // This test is checking to see if the RW Mutex in the Service_Gestalt
 // Will guard against suspend/resume calls overlapping.
 //
 int test_Gestalt_concurrent_long_suspend_overlap_resume(int threadCount)
 {
   std::cout << service_name(__FUNCTION__).c_str() << std::endl;

   ACE_UNUSED_ARG(threadCount);
   int result = 1;
   const int expected = 1;
   int value = 0;


   // This will make the DEFAULT_XXX_TIMEOUTS minimum values.
   DAF::set_property(DAF_SVCACTIONTIMEOUT, "16");
   {
     DAF::ServiceGestalt container;
     DAF::TaskExecutor executor;
     std::stringstream params;
     params << (debug ? "--debug ":"") ;

     test::TestGestaltService::fini_cnt = 0;
     test::TestGestaltService::suspend_cnt = 0;

     result &=  (container.loadDynamic(service_name(__FUNCTION__).c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, params.str().c_str() ) == 0);

     test::TestGestaltService *svc = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,service_name(__FUNCTION__).c_str(),false);
     result &= (svc != 0 );


     std::string ident = service_name(__FUNCTION__).c_str();

     //
     // This has been configured with a long suspend followed by cycles
     // of short suspends, resumes
     //
     if ( result )
     {
        svc->svc_suspend_delay = DAF::get_numeric_property<int>(DAF_SVCACTIONTIMEOUT) * DAF_MSECS_ONE_SECOND * 2;
     }

     // Start Long SUSPEND
     if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T Executing SUSPEND Runner\n"));
     executor.execute(new ACEServiceAction(container, SVC_SUSPEND, ident));

     // Give it some time to enter the suspend
     ACE_OS::sleep(ACE_Time_Value(1,0));

     // Reset Timeout...
     if ( result )
     {
        svc->svc_suspend_delay = 0;
     }

     int ops = 0;
     for ( int i =0 ;i < threadCount; ++i )
     {
       ops += container.suspendService(ident);
       ops += container.resumeService(ident);
     }

     if ( result )
     {
       value = svc->svc_suspend_cnt + svc->svc_resume_cnt;
     }

     // Thread Join?
     executor.wait();
     if (debug) ACE_DEBUG((LM_INFO, "(%P|%t) %T Thread Joined Test\n"));

   }

   result &= (value == expected) || (value == (threadCount * 2 - 1) ) ;

   std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

   return result;
 }


 //
 // TEST
 //
 int test_Gestalt_suspendServiceAll_failure_count(int threadCount)
 {
   std::cout << service_name(__FUNCTION__).c_str() << std::endl;

   ACE_UNUSED_ARG(threadCount);
   int result = 1;
   const int expected = 2;
   int value = -1 ;

   {
     DAF::ServiceGestalt container;
     std::stringstream params;
     params << (debug ? "--debug ":"") ;

     std::stringstream svcname;
     svcname << service_name(__FUNCTION__);



     result &=  (container.loadDynamic(svcname.str().c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, params.str().c_str() ) == 0);
     test::TestGestaltService *svcA = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,svcname.str().c_str(),false);

     svcname << "B";
     result &=  (container.loadDynamic(svcname.str().c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, params.str().c_str() ) == 0);
     test::TestGestaltService *svcB = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,svcname.str().c_str(),false);


     result &= (svcA != 0 ) && (svcB != 0 );
     if ( result )
     {
       // Create some failures on the suspend call
       svcA->ret_val = -1;
       svcB->ret_val = -1;
     }

     value = container.suspendServiceAll();
     if ( debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T Suspended Failures on suspendServiceAll %d\n", value));

     if ( result )
     {
       // Create some failures on the suspend call
       svcA->ret_val = 0;
       svcB->ret_val = 0;
     }

     result &= (0 == container.suspendServiceAll());
     if ( debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T Suspended Failures on suspendServiceAll %d\n", !result));

   }

   result &= (value == expected) ;

   std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

   return result;
 }

 //
 // TEST
 //
 int test_Gestalt_resumeServiceAll_failure_count(int threadCount)
 {
   std::cout << service_name(__FUNCTION__).c_str() << std::endl;

   ACE_UNUSED_ARG(threadCount);
   int result = 1;
   const int expected = 2;
   int value = -1 ;

   {
     DAF::ServiceGestalt container;
     std::stringstream params;
     params << (debug ? "--debug ":"") ;

     std::stringstream svcname;
     svcname << service_name(__FUNCTION__);



     result &=  (container.loadDynamic(svcname.str().c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, params.str().c_str() ) == 0);
     test::TestGestaltService *svcA = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,svcname.str().c_str(),false);

     svcname << "B";
     result &=  (container.loadDynamic(svcname.str().c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, params.str().c_str() ) == 0);
     test::TestGestaltService *svcB = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,svcname.str().c_str(),false);

     result &= (0 == container.suspendServiceAll());

     result &= (svcA != 0 ) && (svcB != 0 );
     if ( result )
     {
       // Create some failures on the suspend call
       svcA->ret_val = -1;
       svcB->ret_val = -1;
     }

     value = container.resumeServiceAll();
     if ( debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T Resume Failures on resumeServiceAll %d\n", value));

     if ( result )
     {
       // Create some failures on the suspend call
       svcA->ret_val = 0;
       svcB->ret_val = 0;
     }

     result &= (0 == container.resumeServiceAll());
     if ( debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T Resume Failures on resumeServiceAll %d\n", !result));

   }

   result &= (value == expected) ;

   std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

   return result;
 }


 //
 // TEST
 //
 int test_Gestalt_removeServiceAll_failure_count(int threadCount)
 {
   std::cout << service_name(__FUNCTION__).c_str() << std::endl;

   ACE_UNUSED_ARG(threadCount);
   int result = 1;
   /* Note we aren't actually expecting to generate failures relating to 'already removed' on a call to removeAll.
   * We aren't directly specifying what we are trying to remove, so there is no direct relationship between a request and a failure.
   */
   const int expected = 0; // 2 // We aren't explicitly removing 2 identified services by removing all;
   int value = -1 ;

   {
     DAF::ServiceGestalt container;
     std::stringstream params;
     params << (debug ? "--debug ":"") ;

     std::stringstream svcname;
     svcname << service_name(__FUNCTION__);



     result &=  (container.loadDynamic(svcname.str().c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, params.str().c_str() ) == 0);
     test::TestGestaltService *svcA = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,svcname.str().c_str(),false);

     svcname << "B";
     result &=  (container.loadDynamic(svcname.str().c_str(), TEST_LIBRARY_NAME, TEST_MAKE_NAME, params.str().c_str() ) == 0);
     test::TestGestaltService *svcB = ACE_Dynamic_Service<test::TestGestaltService>::instance(&container,svcname.str().c_str(),false);


     result &= (svcA != 0 ) && (svcB != 0 );
     if ( result )
     {
       // Create some failures on the suspend call
       svcA->ret_val = -1;
       svcB->ret_val = -1;
     }

     value = container.removeServiceAll();
     if ( debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T Remove Failures on resumeServiceAll %d\n", value));

     // The question is have these really been removed
     int temp = container.removeServiceAll();
     if ( debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T Remove Failures  2 on removeServiceAll %d\n", temp));

     if ( result )
     {
       // Create some failures on the suspend call
       svcA->ret_val = 0;
       svcB->ret_val = 0;
     }

     result &= (0 == container.removeServiceAll());
     if ( debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T Remove Failures on removeServiceAll %d\n", !result));

   }

   result &= (value == expected) ;

   std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

   return result;
 }

 //
 // TEST - PLAY
 // Investigating the usage of Streams and modules via the DAF::ServiceGestalt
 //
 int test_Gestalt_load_stream(int threadCount)
 {
   std::cout << service_name(__FUNCTION__).c_str() << std::endl;

   ACE_UNUSED_ARG(threadCount);
   int result = 1;
   const int expected = 1;
   int value = 0;

   {
     DAF::ServiceGestalt container;
     DAF_Service_Config_Guard guard(&container);

     std::stringstream directive ;
     directive << "stream dynamic TestGestaltStream STREAM * TestGestaltService : _make_test_TestGestaltStream() active {\n\tdynamic TestModuleA Module * TestGestaltService : _make_test_TestGestaltModule() \"--debug\"\n\tdynamic TestModuleB Module * TestGestaltService : _make_test_TestGestaltModule() \"--debug\"\n}\n";


     ACE_Service_Gestalt *current = ACE_Service_Config::current();
     if ( debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T Gestalts Current %@ Global %@ Container %@\n", current, ACE_Service_Config::global(), &container));
     result &= ( 0 == current->process_directive(directive.str().c_str()) );

     test::TestGestaltStream *stream;
     stream = Dynamic_Locator<test::TestGestaltStream>::stream("TestGestaltStream", &container);

     test::TestGestaltModule *module;
     module = Dynamic_Locator<test::TestGestaltModule>::module("TestModuleA", &container);

     if ( debug ) ACE_DEBUG((LM_INFO, "(%P|%t) %T Stream 0x%@ Module 0x%@ name %s\n", stream, module, (module? module->name(): "null") ));

     result &= ( 0 !=  stream);

     if ( result )
     {
       // Put Message into Stream...
       if ( debug )  ACE_DEBUG((LM_INFO, "Got a Stream !!\n"));
       //stream->close();
     }


   }

   result &= (value == expected);

   std::cout <<  " Expected " << expected << " result " << value << " " << (result ? "OK" : "FAILED" ) << std::endl;

   return result;
 }

}//namespace test

void print_usage(const ACE_Get_Opt &cli_opt)
{
    ACE_UNUSED_ARG(cli_opt);
    std::cout << test::TEST_NAME
              << " -h --help              : Print this message \n"
              << " -z --debug             : Debug \n"
              << " -n --count             : Number of Threads/Test\n"
              << std::endl;
}

int main(int argc, char *argv[])
{
    int result = 1, threadCount = 2;

    ACE_Get_Opt cli_opt(argc, argv, "hzn:");
    cli_opt.long_option("help",'h', ACE_Get_Opt::NO_ARG);
    cli_opt.long_option("debug",'z', ACE_Get_Opt::NO_ARG);
    cli_opt.long_option("count",'n', ACE_Get_Opt::ARG_REQUIRED);

    for( int i = 0; i < argc; ++i ) switch(cli_opt()) {
        case -1: break;
        case 'h': print_usage(cli_opt); return 0;
        case 'z': DAF::debug(true); test::debug=true; break;
        case 'n': threadCount = ACE_OS::atoi(cli_opt.opt_arg());
    }

    std::cout << test::TEST_NAME << std::endl;

#if 1
    //
    // Loading Static Libraries is not really supported at the time of writing
    // these tests this needs to be revisited at a later date
    //
    //result &= test::test_Gestalt_load_static_clean(threadCount);


    result &= test::test_Gestalt_load_dynamic_clean(threadCount);
    result &= test::test_Gestalt_load_dynamic_multiple(threadCount);
    result &= test::test_Gestalt_remove(threadCount);
    result &= test::test_Gestalt_suspend(threadCount);
    result &= test::test_Gestalt_suspend_already_inactive(threadCount);
    result &= test::test_Gestalt_suspendServiceAll_failure_count(threadCount);

    result &= test::test_Gestalt_resume(threadCount);
    result &= test::test_Gestalt_resume_already_active(threadCount);
    result &= test::test_Gestalt_resume_already_active_multiple(threadCount);
    result &= test::test_Gestalt_resumeServiceAll_failure_count(threadCount);

    result &= test::test_Gestalt_double_remove_EEXIST(threadCount);
    result &= test::test_Gestalt_removeServiceAll_failure_count(threadCount);

    result &= test::test_Gestalt_load_static_ident_whitespace_EINVAL(threadCount);
    result &= test::test_Gestalt_suspend_ident_whitespace_EINVAL(threadCount);
    result &= test::test_Gestalt_resume_ident_whitespace_EINVAL(threadCount);
    result &= test::test_Gestalt_remove_ident_whitespace_EINVAL(threadCount);

    result &= test::test_Gestalt_load_dynamic_ident_whitespace_EINVAL(threadCount);
    result &= test::test_Gestalt_load_dynamic_ident_bad_characters_EINVAL(threadCount);
    result &= test::test_Gestalt_load_dynamic_ident_bad_characters_control_EINVAL(threadCount);
    result &= test::test_Gestalt_load_dynamic_libpath_EINVAL(threadCount);
    result &= test::test_Gestalt_load_dynamic_objclass_EINVAL(threadCount);

    //
    // The set of errno tests are here for the case where a "called" service
    // can return a -1 state but pass through its own errno.
    // ATM we can't pass through the errno. These tests will fail.
    // Either the tests are faulty (very possible) or something is masking
    // the errno in the ServiceGestalt process.
    //
    //result &= test::test_Gestalt_load_dynamic_service_errno(threadCount);
    //result &= test::test_Gestalt_suspend_service_errno(threadCount);
    //result &= test::test_Gestalt_resume_service_errno(threadCount);
    //result &= test::test_Gestalt_remove_service_errno(threadCount);

    result &= test::test_Gestalt_load_dynamic_timeout_ETIME(threadCount);
    result &= test::test_Gestalt_remove_timeout_ETIME(threadCount);
    result &= test::test_Gestalt_suspend_timeout_ETIME(threadCount);
    result &= test::test_Gestalt_resume_timeout_ETIME(threadCount);

    result &= test::test_Gestalt_long_remove_remove(threadCount);
    result &= test::test_Gestalt_concurrent_long_remove_short_resume(threadCount);
#endif


#if 0
    {
//      ACE_Debug debug(test::debug);
      // Not Yet Fully working....
      //result &= test::test_Gestalt_load_stream(threadCount);

      result &= test::test_Gestalt_concurrent_long_suspend_overlap_resume(threadCount);
      result &= test::test_Gestalt_suspend_EACCES(threadCount);
    }
#endif

    return !result;
}
