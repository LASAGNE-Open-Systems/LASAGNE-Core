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
#ifndef __DDS_INIT_TEST_SERVICE_H__
#define __DDS_INIT_TEST_SERVICE_H__

#include "DDSInitTest_export.h"
#include "daf/TaskExecutor.h"


namespace test
{

  /**
   * FRAM-122 Trying to test for the existence of problems with ODDS loading on the svc
   * routine
   */
  class DDS_INIT_TEST_Export DDSInitTestService : public DAF::TaskExecutor
  {
  public:
    DDSInitTestService(void);
    virtual ~DDSInitTestService(void);

    static const char *svc_ident(void);

  protected:
    virtual int init(int argc, ACE_TCHAR *argv[]);

    virtual int suspend(void);

    virtual int resume(void);

    virtual int fini(void);

    virtual int info(ACE_TCHAR **info_string, size_t length = 0 ) const;

    virtual int svc(void);

  protected:
    int parse_args(int argc, ACE_TCHAR *argv[]);

  private:

    bool service_;
    int debug_;
    long timer_id_;
    ACE_Service_Gestalt *loaded_gestalt_;
  };

}//namespace test


typedef test::DDSInitTestService test_DDSInitTestService;

ACE_FACTORY_DECLARE(DDS_INIT_TEST, test_DDSInitTestService);


#endif //__TAF_TEST_SERVICE_H__
