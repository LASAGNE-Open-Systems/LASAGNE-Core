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
#ifndef __TAF_TEST_SERVICE_H__
#define __TAF_TEST_SERVICE_H__

#include "TestService_export.h"
#include "daf/TaskExecutor.h"
#include "ace/Time_Value.h"

namespace test
{

/**
*   A Service for testing that provides a number of common utilities
* - TAFServer shutdown : will send a shutdown call to the TAFServer after a period of time
* - Service-Under-Test : A service tester for init,fini, suspend, resume etc.
*/
class TAF_TEST_SERVICE_Export TestService : public DAF::TaskExecutor
{
public:
    TestService(void);
    virtual ~TestService(void);

    static const char *svc_ident(void);

protected:
    virtual int init(int argc, ACE_TCHAR *argv[]);

    virtual int suspend(void);

    virtual int resume(void);

    virtual int fini(void);

    virtual int info(ACE_TCHAR **info_string, size_t length = 0 ) const;

    virtual int handle_timeout(const ACE_Time_Value &current_time,
                              const void *act = 0);
protected:
    int parse_args(int argc, ACE_TCHAR *argv[]);

private:

    ACE_Time_Value timer_;
    bool shutdown_;
    int debug_;
    long timer_id_;
};

}//namespace test


typedef test::TestService test_TestService;

ACE_FACTORY_DECLARE(TAF_TEST_SERVICE, test_TestService);


#endif //__TAF_TEST_SERVICE_H__
