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

#ifndef TAF_CORBACLIENTSERVICE_H
#define TAF_CORBACLIENTSERVICE_H

#include "CORBAClientService_export.h"

#include "CORBAServiceC.h"
#include "daf/TaskExecutor.h"

namespace TAF
{

/**
 * CORBAClientService
 *
 * A Client that is wrapped up as a Service for loading in the TAF Server
 */
class CORBAClientService_Export CORBAClientService : public DAF::TaskExecutor
{
public:
    CORBAClientService(void);
    virtual ~CORBAClientService(void);

    static const ACE_TCHAR *svc_ident(void)
    {
        return ACE_TEXT("CORBAClientService");
    }

protected:

    virtual int init(int argc, ACE_TCHAR* argv[]);

    virtual int fini(void);

    virtual int info(ACE_TCHAR **info_string, size_t length = 0) const;

    virtual int svc(void);

private:

    int parse_args(int argc, ACE_TCHAR *argv[]);

    int ok_;
    int threads_;
};



}//namesapce TAF

typedef TAF::CORBAClientService TAF_CORBAClientService;

ACE_FACTORY_DECLARE(CORBAClientService, TAF_CORBAClientService);


#endif //TAF_CORBACLIENTSERVICE_H
