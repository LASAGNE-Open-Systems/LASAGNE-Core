#ifndef _DAF_TEST_SERVICE_H_
#define _DAF_TEST_SERVICE_H_

#include "TEST_SERVICE_export.h"
#include <ace/Service_Object.h>
#include <ace/Module.h>
#include <ace/Stream.h>
#include "Macros.h"

namespace test
{

class TEST_SERVICE_Export TestGestaltService : public ACE_Task<ACE_SYNCH>
{
public:
  static int init_cnt;
  static int fini_cnt;
  static int suspend_cnt;
  static int resume_cnt;

  int ret_errno;
  int ret_val;

  int svc_init_cnt;
  int svc_fini_cnt;
  int svc_resume_cnt;
  int svc_suspend_cnt;

  time_t svc_init_delay;
  time_t svc_fini_delay;
  time_t svc_resume_delay;
  time_t svc_suspend_delay;

  int debug;

  static const ACE_TCHAR *svc_ident(void);

  TestGestaltService(void);
  virtual ~TestGestaltService(void);

  virtual int init(int argc, ACE_TCHAR *argv[]);
  virtual int fini(void);

  virtual int suspend(void);
  virtual int resume(void);



protected:
  int parse_args(int argc, ACE_TCHAR *argv[]);

};



class TEST_SERVICE_Export TestGestaltModule : public ACE_Module<ACE_SYNCH>
{
public:
  TestGestaltModule(void);
  virtual ~TestGestaltModule(void);
};

class TEST_SERVICE_Export TestGestaltStream : public ACE_Stream<ACE_SYNCH>
{
public:
  TestGestaltStream(void);
  virtual ~TestGestaltStream(void);
};


} //namespace test

typedef test::TestGestaltService test_TestGestaltService;
typedef test::TestGestaltModule test_TestGestaltModule;
typedef test::TestGestaltStream test_TestGestaltStream;

DAF_STREAM_DECLARE(TEST_SERVICE, test_TestGestaltStream);
DAF_MODULE_DECLARE(TEST_SERVICE, test_TestGestaltModule);

ACE_FACTORY_DECLARE(TEST_SERVICE, test_TestGestaltService);
ACE_STATIC_SVC_DECLARE_EXPORT(TEST_SERVICE, test_TestGestaltService);




#endif // _DAF_TEST_SERVICE_H_
