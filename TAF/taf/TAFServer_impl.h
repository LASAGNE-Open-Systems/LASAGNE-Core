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
    ///Forward Declaration
    typedef class CORBAInterfaceHandler_T<POA_taf::TAFServer>   TAFServerInterfaceHandler;

    /** @class TAFServer_impl
    * @brief Brief \todo{Add Brief}
    *
    * details \todo{Details}
    */
    class TAF_Export TAFServer_impl : virtual public TAFServerInterfaceHandler, virtual public ACE_Service_Object
        , virtual public TAFGestaltServiceImpl
        , virtual public TAFPropertyServerImpl
    {
        int parse_args(int argc, ACE_TCHAR *argv[]);

    public:

        /** \todo{Fill this in} */
        TAFServer_impl(void);
        virtual ~TAFServer_impl(void);

        /** \todo{Fill this in} */
        virtual taf::EntityDescriptor *entity_descriptor(void);

        /** \todo{Fill this in} */
        virtual char*   svc_name(void);

        /** \todo{Fill this in} */
        static const char*  svc_ident(void)
        {
            return taf::TAFSERVER_OID;
        }

    public:  /* IDL Interface Methods */

        /** \todo{Fill this in} */
        virtual void    sendConsoleMsg(const char *msg);
        /** \todo{Fill this in} */
        virtual void    shutdown(void);         // Handle Shutdown Request

        /** \todo{Fill this in} */
        virtual char *  getHostName(void);
        /** \todo{Fill this in} */
        virtual char *  getRepoQOS(CORBA::Object_ptr obj, CORBA::Long_out time);

    protected:  /* Service Interface */

        /** \todo{Fill this in} */
        virtual int init(int argc, ACE_TCHAR *argv[]);
        /** \todo{Fill this in} */
        virtual int suspend(void);
        /** \todo{Fill this in} */
        virtual int resume(void);
        /** \todo{Fill this in} */
        virtual int fini(void);
        /** \todo{Fill this in} */
        virtual int info(ACE_TCHAR **info_string, size_t length = 0) const;

    protected:

        virtual const char * config_switch(void) const
        {
            return TAF_SERVICES;  // Switch for ServiceGestaltLoader
        }
    };
} // namespace TAF

typedef class TAF::TAFServer_impl   TAFServerImpl;

#endif // TAF_TAFSERVER_IMPL_H
