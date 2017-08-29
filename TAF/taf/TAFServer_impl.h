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

#include <daf/TaskExecutor.h>

namespace TAF
{
    ///Forward Declaration
    typedef class CORBAInterfaceHandler_T<POA_taf::TAFServer>   TAFServerInterfaceHandler;

    /** @class TAFServer_impl
    * @brief The TAFServer is the primary process level interface to the service container
    * itself is a service contained within the primary (global) gestalt specifically inserted 
    * after the infrastructure middleware for dependant unload purposes
    */
    class TAF_Export TAFServer_impl : virtual public TAFServerInterfaceHandler, public DAF::TaskExecutor
        , virtual public TAFGestaltServiceImpl
        , virtual public TAFPropertyServerImpl
    {
        using DAF::TaskExecutor::execute; // Make this private

    public:

        /** Default Constructor */
        TAFServer_impl(void);
        virtual ~TAFServer_impl(void);

        /** Provide a descriptor for this process interface */
        virtual taf::EntityDescriptor *entity_descriptor(void);

        /** Provide a formal name for this instance */
        virtual char*   svc_name(void);

        /** Provide an Object ID for this interface */
        static const char*  svc_ident(void)
        {
            return taf::TAFSERVER_OID;
        }

    public:  /* IDL Interface Methods */

        /** Send a Console message to this process */
        virtual void    sendConsoleMsg(const char *msg);
        /** Direct the process to close down - and Unwind all its services */
        virtual void    shutdown(void);         // Handle Shutdown Request

        /** Provide an interface to get the hostname */
        virtual char *  getHostName(void);
        /** Provide an interface to get roundtrip timing to an interface */
        virtual char *  getRepoQOS(CORBA::Object_ptr obj, CORBA::Long_out time);

    protected:  /* Service Interface */

        /** Initialize the service */
        virtual int init(int argc, ACE_TCHAR *argv[]);
        /** Finish the service */
        virtual int fini(void);
        /** Provide information about the service */
        virtual int info(ACE_TCHAR **info_string, size_t length = 0) const;

    protected:

        /** Execute a service action - Overloaded to call Executors implementation in the base */
        virtual int execute_svc_action(const DAF::Runnable_ref & command)
        {
            return this->execute(command);
        }

        /** Provide the Config Switch for file set to load */
        virtual const char * config_switch(void) const
        {
            return TAF_SERVICES;  // Switch for ServiceGestaltLoader
        }

    private:

        int parse_args(int argc, ACE_TCHAR *argv[]);
    };
} // namespace TAF

typedef class TAF::TAFServer_impl   TAFServerImpl;

#endif // TAF_TAFSERVER_IMPL_H
