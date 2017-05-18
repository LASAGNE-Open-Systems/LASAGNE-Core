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
#ifndef TAF_TAFSERVER_H
#define TAF_TAFSERVER_H

#include "TAF.h"
#include "ORBManager.h"

class TAF_Export TAFServer : public TAF::ORBManager
{
public:

    TAFServer(void);
    TAFServer(int argc, ACE_TCHAR *argv[]);     // Used to instantiate

    virtual ~TAFServer(void);

    enum {
        TAFSERVER_LOAD_TIMEOUT = time_t(60)     // Default to 60 Seconds
    };

    static const char * svc_ident(void)         // Used to name service in Gestalt
    {
        return ACE_TEXT("TAFServerLoader");
    }

    virtual int run(bool wait_for_completion);

protected:  /* Service Interface */

    virtual int init(int argc, ACE_TCHAR *argv[]);
    virtual int fini(void);
    virtual int info(ACE_TCHAR **info_string, size_t length = 0) const;

protected:

    virtual int close(u_long flags);
};

extern "C" {
    typedef int(*TAFSERVER_INIT_FUNCTION)(int argc, char *argv[]);
    TAF_Export int _init_TAFServer(int argc, char *argv[]);
    typedef int(*TAFSERVER_WAIT_FUNCTION)(unsigned);
    TAF_Export int _wait_TAFServer(unsigned);
    typedef int(*TAFSERVER_FINI_FUNCTION)(void);
    TAF_Export int _fini_TAFServer(void);
}

#endif  // TAF_TAFSERVER_H
