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
#ifndef TAF_TAFSERVER_IMPL_H
#define TAF_TAFSERVER_IMPL_H

#include "TAF.h"

#include "CORBAInterfaceHandler_T.h"

#include "GestaltService_impl.h"
#include "PropertyServer_impl.h"

#include "TAFServerS.h"

#include <ace/Service_Object.h>

namespace TAF
{
    typedef class CORBAInterfaceHandler_T<POA_taf::TAFServer>   TAFServerInterfaceHandler;

    class TAF_Export TAFServer_impl : virtual public TAFServerInterfaceHandler, virtual public ACE_Service_Object
        , virtual public TAFGestaltServiceImpl
        , virtual public TAFPropertyServerImpl
    {
        int parse_args(int argc, ACE_TCHAR *argv[]);

    public:

        TAFServer_impl(void);
        virtual ~TAFServer_impl(void);

        virtual taf::EntityDescriptor *entity_descriptor(void);

        virtual char*   svc_name(void);

        static const char*  svc_ident(void)
        {
            return taf::TAFSERVER_OID;
        }

    public:  /* IDL Interface Methods */

        virtual void    sendConsoleMsg(const char *msg);
        virtual void    shutdown(void);         // Handle Shutdown Request

        virtual char *  getHostName(void);
        virtual char *  getRepoQOS(CORBA::Object_ptr obj, CORBA::Long_out time);

    protected:  /* Service Interface */

        virtual int init(int argc, ACE_TCHAR *argv[]);
        virtual int suspend(void);
        virtual int resume(void);
        virtual int fini(void);
        virtual int info(ACE_TCHAR **info_string, size_t length = 0) const;
    };
} // namespace TAF

typedef class TAF::TAFServer_impl   TAFServerImpl;

#endif // TAF_TAFSERVER_IMPL_H
