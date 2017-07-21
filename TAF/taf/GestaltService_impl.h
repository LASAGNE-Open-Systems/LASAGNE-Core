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
#ifndef TAF_GESTALTSERVICE_IMPL_H
#define TAF_GESTALTSERVICE_IMPL_H

#include "TAF.h"

#include <daf/TaskExecutor.h>

#include <daf/ServiceGestalt.h>
#include <daf/ServiceGestaltLoader.h>

#include <ace/Service_Object.h>
#include <ace/Semaphore.h>

#include "GestaltServiceS.h"

#include <map>

namespace TAF
{
    /**
    * @class GestaltService_impl
    * @brief Provides a CORBA accessable facade to an underlying Gestalt
    *
    * Details This Facade is mixed-in to the TAFServer interface as a concrete implementation
    */
    typedef class TAF_Export GestaltService_impl : virtual public POA_taf::GestaltService
    {
    public:

        typedef DAF::ServiceGestalt::ident_type                 ident_type;
        typedef DAF::ServiceGestalt::ident_list_type            ident_list_type;

        typedef DAF::ServiceGestalt::service_descriptor_type    service_descriptor_type;
        typedef DAF::ServiceGestalt::service_list_type          service_list_type;
        typedef DAF::ServiceGestalt::service_map_type           service_map_type;

        /** Constructor */
        GestaltService_impl(const ACE_TCHAR *program_name = 0);
        /** Destructor */
        virtual ~GestaltService_impl(void);

        /** Find a service descriptor by ident */
        virtual taf::EntityDescriptor *     findService(const char *ident);
        /** List all the services with a sequence of descriptors */
        virtual taf::EntityDescriptorSeq *  listServices(void);

        /** load a set of services as described through a file set "filename:<section>,<section>" */
        virtual CORBA::Long loadConfigFile(const char *file_arg, CORBA::Long_out count); // "filename:<section>,<section>"

        /** load a static service (located through static descriptor */
        virtual void    loadStatic(const char *ident, const char *parameters);
        /** load a dynamic service through its @a libpathname (dll name), @a objectclass (factory pattern) and @a parameters */
        virtual void    loadDynamic(const char *ident, const char *libpathname, const char *objectclass, const char *parameters);
        /** signal an active service to suspend itself */
        virtual void    suspend(const char *ident);
        /** signal a suspended (non-active) service to resume its normal operation */
        virtual void    resume(const char *ident);
        /** remove (and shutdown) a service */
        virtual void    remove(const char *ident);
        /** signal all active services within the gestalt to suspend */
        virtual CORBA::Long suspend_all(void);
        /** signal all non-active services within the gestalt to resume normal operation */
        virtual CORBA::Long resume_all(void);
        /** remove (and shutdown) all services within the gestalt */
        virtual CORBA::Long remove_all(void);

    protected:

        /** Abstract method to provide the configuration property switch for locating the file set to load */ 
        virtual const char * config_switch(void) const = 0; // Must provide a config switch to support CORBA interface

    protected:

        /** Make a descriptor for a service entity (by ident) within this gestalt */
        taf::EntityDescriptor_var   makeEntityDescriptor(const ident_type &ident) const;
        /** Make a descriptor for a service entity (by descriptor type) within this gestalt */
        taf::EntityDescriptor_var   makeEntityDescriptor(const service_descriptor_type &svc_desc) const;

        /** The actual underlying Gestalt instance - composition */
        class GestaltService : public DAF::ServiceGestalt
        {
            GestaltService_impl &gestalt_impl_;

        public:

            /** Constructor for underlying Gestalt instance */
            GestaltService(GestaltService_impl &gestalt_impl, const ACE_TCHAR *program_name = 0) : DAF::ServiceGestalt(program_name)
                , gestalt_impl_(gestalt_impl)
            {}

            /** Fill in the abstract File set switch from composer interface */
            virtual const char * config_switch(void) const
            {
                return this->gestalt_impl_.config_switch();
            }

        } gestalt_;

    public:

        /** Accessor to the gestalts initial load time */
        const ACE_Time_Value &  loadTime(void) const
        {
            return this->gestalt_.loadTime();
        }

    } GestaltServiceImpl;

    /// Compatability with older revisions requires a TAF::GestaltServiceLoader
    typedef class DAF::ServiceGestaltLoader GestaltServiceLoader;

} // namespace TAF

typedef class TAF::GestaltService_impl  TAFGestaltServiceImpl;

#endif // TAF_GESTALTSERVICE_IMPL_H
