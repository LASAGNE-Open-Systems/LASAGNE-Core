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
#ifndef DAF_SERVICEGESTALT_H
#define DAF_SERVICEGESTALT_H

#include "DAF.h"

#include "Runnable.h"
#include "Semaphore.h"

#include <ace/Service_Config.h>
#include <ace/Service_Gestalt.h>

#include <list>
#include <map>

namespace DAF
{
    /**
    * \class ServiceGestalt
    * \brief Service Container and Lifecycle Control
    *
    * The ServiceGestalt provides a container to control the lifecycle of
    * ACE_Service_Objects. It has a number of features:
    * - load/unload dynamically loaded services
    * - load/unload statically loaded services
    * - suspend/resume loaded services
    *
    * Coupled with its sibling DAF::ServiceGestaltLoader the ServiceGestalt
    * allows loading via CLI with DAF::Configurator syntax.
    *
    * Properties
    * ----------
    *
    *   DAFServiceActionTimeout
    *
    * All Service Actions (load, remove, suspend, resume) are performed in a
    * future result manner with timeouts. This default time period is specified
    * by the 'DAFServiceActionTimeout' property. Its default value is 10 seconds.
    * It can be changed via the property value but it is bounded by the values specified
    * in DEFAULT_MAXIMUM_TIMEOUT (high value) and DEFAULT_XXXX_TIMEOUT(low value)
    *
    * Errors and Failures
    * -------------------
    *
    * Each of the service actions (load, remove, suspend, resume) can result in a
    * number of errors or failed states. There are two mechanisms to transfer the
    * error state of an action command. Most of the functions will have a return code (int)
    * which when non-zero indicates an error state has been encountered. The second
    * mechanism is via errno. Errno is passed from the Thread executing the
    * Service Action to the calling thread. This means a service being operated on
    * can set errno using DAF_OS::last_error(errno) in the service method (init, fini, suspend, resume)
    * and the result can be passed to the caller.
    *
    * In addition, there are numerous errno values that the Gestalt infrastructure
    * uses to provide further clarification on processing of commands:
    *
    * ETIME  - Used to indicate a timeout has been encountered between issuing
    *          the ServiceAction command and receiving a result. NOTE: This can
    *          mask the error internal the service or the action.
    *
    * EINVAL - Can indicate a syntax error in the ident of the service (ie whitespace not allowed)
    *        - Can indicate a syntax error in the object class factory method (ie whitespace NOT allowed)
    *        - Can indicate a syntax error in the libpath.
    *        - Service Shared Library (dll, so) is not present on the load path (PATH, LD_LIBRARY_PATH, DYLD_LIBRARY_PATH)
    *        - Service factory method (_make_xxx) is not present or not accessible (exported)
    *
    * ENOLCK - Could not obtain the concurrency lock to perform the opertion.
    *
    * EEXIST - The service ident is already present or absent in the gestalt making the operation
    *          invalid.
    *        - The service is present in the gestalt but not accessible. Is present from a dependent load.
    *
    * EACCES - The gestalt entry is not an ACE_Service_Object
    *        - The service entry has already had fini_called and access is denied
    *
    * ENOTSUP- Operation Not supported, generally returned by the service when
    *          operations are not supported (NOTE: use ACE_NOTSUP_RETURN(-1) within overloaded suspend/resume)
    *
    * ENOEXEC - Internal Error creating the execution of the Service Action.
    *
    * ENOENT  - ??
    *
    *
    * Notes
    * -----
    *
    * Dynamic library management in a gestalt is handled by the ACE_DLL_Manager
    * singleton. There is different behaviour in the handling of dlls depending
    * on the Operating System. On Windows, DLLs are reference counted by the
    * ACE_DLL_Manager and when a service remove(fini) is called may result in the
    * dll being unloaded from memory. This is controlled by the DLL_UNLOAD_POLICY
    * of the ACE_DLL_Manager.
    */

    class DAF_Export ServiceGestalt : public ACE_Service_Gestalt
    {
        using ACE_Service_Gestalt::process_directives;
        using ACE_Service_Gestalt::process_directive;
        using ACE_Service_Gestalt::process_file;
        using ACE_Service_Gestalt::suspend;
        using ACE_Service_Gestalt::resume;
        using ACE_Service_Gestalt::remove;

    public:

        /**
         * \param program_name used to identify the ACE_Log associated with
         *        this container
         */
        ServiceGestalt(const ACE_TCHAR * program_name = 0);

        virtual ~ServiceGestalt(void);

        /**
         * config_switch is a parameter that is used by Service Loaders to
         * identify the CLI args that should be read and given to this Gestalt
         * for processing. Examples used throughout this infrastructure are
         * '-TAFProperties <file:sec>'
         *
         * Should Be Abstract but it may not be used in every instance
         */
        virtual const char * config_switch(void) const;

        typedef std::string             ident_type;

        typedef std::list<ident_type>   ident_list_type;

        typedef enum {  // Same as <taf/EntityDescriptor.idl> -> SVC_Flags
            LOAD_EXECUTE    = unsigned(1U << 0),    // Currently Un-Used
            LOAD_STATIC     = unsigned(1U << 1),
            LOAD_DYNAMIC    = unsigned(1U << 2),
            LOAD_OBJECT     = unsigned(1U << 3),
            LOAD_MODULE     = unsigned(1U << 4),    // Currently Un-Used
            LOAD_STREAM     = unsigned(1U << 5),    // Currently Un-Used
            LOAD_ACTIVE     = unsigned(1U << 6),
            LOAD_FINIED     = unsigned(1U << 7)     // Currently Un-Used
        } service_load_flags;

        /**
         \class ServiceDescriptor
         \brief Return structure for meta-data on managed services.
         */
        typedef struct ServiceDescriptor {
            ident_type          ident_;
            std::string         libpathname_;
            std::string         objectclass_;
            std::string         parameters_;
            ACE_Time_Value      loadtime_;
            unsigned            loadflags_; // mixin of service_load_flags
        } service_descriptor_type;

        typedef std::list<service_descriptor_type>              service_list_type;
        typedef std::map<ident_type, service_descriptor_type>   service_map_type;
        typedef std::pair<service_map_type::iterator, bool>     service_map_Pairib; // Meta Type not know on Linux

        int list_service_repository(service_list_type &, bool reverse = false) const;

        /// Allow accessor to repository lock.
        static ACE_Recursive_Thread_Mutex & repository_lock(const ACE_Service_Repository * repo);

        const ACE_Time_Value &  loadTime(void) const;

    public:

        /**
         * Load a service that is statically linked
         *
         * \param ident is the key used in the service map to uniquely identify
         *              the instance of the service
         * \param parameters string of arguments to pass to the service as initialisation
         *              information.
         * \return non-zero  failure of some sort. See Errors and Failure section of
         *              class description
         */
        int loadStatic  (const std::string & ident,
                         const std::string & parameters);

       /**
        * Load a service that is dynamically linked
        *
        * \param ident is the key used in the service map to uniquely identify
        *              the instance of the service
        * \param libpathname is the library name, full path or expandable path to
        *              the library (dll,so) containing the service.
        * \param objectclass is the service factory method (ie _make_xxx) to create
                       the service. In general this is generated via the ACE_FACTORY_DECLARE
                       and ACE_FACTORY_DEFINE macros.
        * \param parameters string of arguments to pass to the service as initialisation
        *              information.
        * \return non-zero  failure of some sort. See Errors and Failure section of
        *              class description
        */
        int loadDynamic (const std::string & ident,
                         const std::string & libpathname,
                         const std::string & objectclass,
                         const std::string & parameters);

        /**
        * Suspend a service present in the container
        *
        * \param ident is the key used in the service map to uniquely identify
        *              the instance of the service
        * \return non-zero  failure of some sort. See Errors and Failure section of
        *              class description
        */
        int suspendService(const std::string & ident);

        /**
        * Resume a service present in the container
        *
        * \param ident is the key used in the service map to uniquely identify
        *              the instance of the service
        * \return non-zero  failure of some sort. See Errors and Failure section of
        *              class description
        */
        int resumeService(const std::string & ident);

        /**
        * Remove a service present in the container
        *
        * \param ident is the key used in the service map to uniquely identify
        *              the instance of the service
        * \return non-zero  failure of some sort. See Errors and Failure section of
        *              class description
        */
        int removeService(const std::string & ident);

        /**
        * Suspend all the services in this container
        * \return non-zero  the number of failures in performing the operation.
        */
        int suspendServiceAll(void);

        /**
        * Resume all the services in this container
        * \return non-zero  the number of failures in performing the operation.
        */
        int resumeServiceAll(void);

        /**
        * Remove all the services in this container
        * \return non-zero  the number of failures in performing the operation.
        */
        int removeServiceAll(void);

    public:

        /// Translation method from ident, params to ACE_Service_Configurator Syntax
        static std::string  static_directive    (const std::string & ident,
                                                 const std::string & parameters = "");

        /// Translation method from ident, params to ACE_Service_Configurator Syntax
        static std::string  dynamic_directive   (const std::string & ident,
                                                 const std::string & libpathname,
                                                 const std::string & objectclass,
                                                 const std::string & parameters = "");

        /// Translation method from ident to ACE_Service_Configurator Syntax
        static std::string  suspend_directive   (const std::string & ident);

        /// Translation method from ident to ACE_Service_Configurator Syntax
        static std::string  resume_directive    (const std::string & ident);

        /// Translation method from ident to ACE_Service_Configurator Syntax
        static std::string  remove_directive    (const std::string & ident);

    protected:

        virtual int execute(const Runnable_ref & command);

    private:

        class ServiceDescriptors : public service_map_type
        {
        public:

            ServiceDescriptors(void) : service_map_type()
            {
            }

            operator ACE_SYNCH_RW_MUTEX & () const
            {
                return this->svc_lock_;
            }

        private:

            mutable ACE_SYNCH_RW_MUTEX  svc_lock_; // service Lock

        } svc_descriptors_;

        ServiceDescriptors &        svc_descriptors(void);

    public:

        const ServiceDescriptors &  svc_descriptors(void) const;

    public:

        class ServiceActivity : public Runnable
        {
        public:

            DAF_DEFINE_REFCOUNTABLE(ServiceActivity);

            virtual int wait_result(time_t timeout) const;

        protected:

            ServiceActivity(ServiceGestalt * gestalt);

            ServiceDescriptors & svc_descriptors(void);

            virtual int process_directive(const std::string & directive);

        protected:

            mutable Semaphore   resultLock_;
            ServiceGestalt *    gestalt_;
            volatile int        result_;
            volatile int        error_;
        };

        DAF_DECLARE_REFCOUNTABLE(ServiceActivity);

    private:

        int process_svc_action(const ServiceActivity_ref &, time_t action_timeout);

        /** Time The Gestalt was constructed - operational */
        const ACE_Time_Value    loadTime_;
    };

    inline const ACE_Time_Value &
    ServiceGestalt::loadTime(void) const
    {
        return this->loadTime_;
    }

    inline ServiceGestalt::ServiceDescriptors &
    ServiceGestalt::svc_descriptors(void)
    {
        return this->svc_descriptors_;
    }

    inline const ServiceGestalt::ServiceDescriptors &
    ServiceGestalt::svc_descriptors(void) const
    {
        return this->svc_descriptors_;
    }

    inline ServiceGestalt::ServiceDescriptors &
    ServiceGestalt::ServiceActivity::svc_descriptors(void)
    {
        return this->gestalt_->svc_descriptors();
    }

} // namespace DAF

/**
* \struct DAF_Service_Config_Guard
* \brief Service Configurator Thread Guard
*
* Allows a thread to change the Thread Specific Global ACE_Service_Configurator
* to the input argument gestalt. This is used when working on multiple gestalts.
* and ensures operations in a gestalt are being performed.
* NOTE: The ACE infrastructure uses Thread Specific Storage to manage the
* ACE_Service_Configurator entry points.
*/
struct DAF_Service_Config_Guard : ACE_Service_Config_Guard {
    DAF_Service_Config_Guard(ACE_Service_Gestalt *gestalt = ACE_Service_Config::global())
        : ACE_Service_Config_Guard(gestalt)
    {}
};

#endif // DAF_SERVICEGESTALT
