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

/**
* @file    GestaltService_impl.h
* @author
* @author   $LastChangedBy$
* @date
* @version  $Revision$
* @ingroup  
*/

namespace TAF
{
    /**
    * @class GestaltService_impl
    * @brief Brief \todo{Add Brief}
    *
    * Details \todo{Add some Details}
    */
    typedef class TAF_Export GestaltService_impl : virtual public POA_taf::GestaltService
    {
    public:

        typedef DAF::ServiceGestalt::ident_type                 ident_type;
        typedef DAF::ServiceGestalt::ident_list_type            ident_list_type;

        typedef DAF::ServiceGestalt::service_descriptor_type    service_descriptor_type;
        typedef DAF::ServiceGestalt::service_list_type          service_list_type;
        typedef DAF::ServiceGestalt::service_map_type           service_map_type;

        /** \todo{Fill this in} */
        GestaltService_impl(const ACE_TCHAR *program_name = 0);
        /** \todo{Fill this in} */
        virtual ~GestaltService_impl(void);

        /** \todo{Fill this in} */
        virtual taf::EntityDescriptor *     findService(const char *ident);
        /** \todo{Fill this in} */
        virtual taf::EntityDescriptorSeq *  listServices(void);

        /** \todo{Fill this in} */
        virtual CORBA::Long loadConfigFile(const char *file_arg, CORBA::Long_out count); // "filename:<section>,<section>"

        /** \todo{Fill this in} */
        virtual void    loadStatic(const char *ident, const char *parameters);
        /** \todo{Fill this in} */
        virtual void    loadDynamic(const char *ident, const char *libpathname, const char *objectclass, const char *parameters);
        /** \todo{Fill this in} */
        virtual void    suspend(const char *ident);
        /** \todo{Fill this in} */
        virtual void    resume(const char *ident);
        /** \todo{Fill this in} */
        virtual void    remove(const char *ident);
        /** \todo{Fill this in} */
        virtual CORBA::Long suspend_all(void);
        /** \todo{Fill this in} */
        virtual CORBA::Long resume_all(void);
        /** \todo{Fill this in} */
        virtual CORBA::Long remove_all(void);

    protected:

        virtual const char * config_switch(void) const = 0; // Must provide a config switch to support CORBA interface

    protected:

        taf::EntityDescriptor_var   makeEntityDescriptor(const ident_type &ident) const;
        taf::EntityDescriptor_var   makeEntityDescriptor(const service_descriptor_type &svc_desc) const;

        class GestaltService : public DAF::ServiceGestalt
        {
            GestaltService_impl &gestalt_impl_;

        public:

            GestaltService(GestaltService_impl &gestalt_impl, const ACE_TCHAR *program_name = 0) : DAF::ServiceGestalt(program_name)
                , gestalt_impl_(gestalt_impl)
            {}

            virtual const char * config_switch(void) const
            {
                return this->gestalt_impl_.config_switch();
            }

        } gestalt_;

    public:

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
