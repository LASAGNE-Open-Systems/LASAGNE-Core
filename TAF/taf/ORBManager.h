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
#ifndef TAF_ORBMANAGER_H
#define TAF_ORBMANAGER_H

#include "TAF.h"

#include "NamingContext.h"
#include "POAPolicyList.h"
#include "ObjectStubRef_T.h"

#include <daf/TaskExecutor.h>
#include <daf/ServiceGestalt.h>

#include <tao/ORB.h>
#include <tao/ORB_Core.h>
#include <tao/LocalObject.h>
#include <tao/PI/ORBInitInfo.h>
#include <tao/ORBInitializer_Registry.h>

#include <tao/IORTable/IORTable.h>
#include <tao/PortableServer/PortableServer.h>

#include <ace/Singleton.h>

namespace {  // Stop Unused warnings
    DAF_UNUSED_STATIC(TAO_Requires_POA_Initializer);
    DAF_UNUSED_STATIC(TAO_Requires_IORTable_Initializer);
}

namespace TAF
{
    const size_t TAF_MIN_THREADS    = 2;   // Minimum Reactor Threads
    const size_t TAF_MAX_ACETHREADS = 20;  // ACE Reactor Max Threads
    const size_t TAF_MAX_ORBTHREADS = 200; // ORB Reactor Max Threads

    const size_t DEFAULT_ACETHREADS = (TAF_MIN_THREADS * 2);
    const size_t DEFAULT_ORBTHREADS = (TAF_MIN_THREADS * 4);

    class TAF_Export ORB;           // forward Declaration
    class TAF_Export ORBManager;    // forward Declaration

    typedef class ACE_DLL_Singleton_T<TAF::ORB, ACE_SYNCH_MUTEX>    ORBSingleton_type;

    class TAF_Export ORB : virtual PortableInterceptor::ORBInitializer, virtual protected CORBA::LocalObject
        , protected DAF::TaskExecutor
    {
        CORBA::ORB_var          orb_;
        PortableServer::POA_var rootPOA_, defaultPOA_;
        IORTable::Table_var     IORTable_;

    public:

        virtual ~ORB(void);

        friend ORBManager;
        friend ORBSingleton_type;

    public: // Access methods

        const CORBA::ORB_var &          orb(void) const;
        const PortableServer::POA_var & rootPOA(void) const;
        const PortableServer::POA_var & defaultPOA(void) const;

        const IORTable::Table_var &     IORTable(void) const;

        const TAF::NamingContext &      rootContext(ACE_Time_Value *timeout = 0) const;
        const TAF::NamingContext &      baseContext(ACE_Time_Value *timeout = 0) const;

        CORBA::Object_var   string_to_object(const std::string &s)  const;
        CORBA::String_var   object_to_string(CORBA::Object_ptr obj) const;

        void  register_initial_reference(const std::string &id, CORBA::Object_ptr obj);
        CORBA::Object_var unregister_initial_reference(const std::string &id);
        CORBA::Object_var resolve_initial_references(const std::string &id, ACE_Time_Value *timeout = 0);

    public: // singleton methods

        const ACE_TCHAR *dll_name(void) const
        {
            return TAF_DLL_NAME;
        }

        const ACE_TCHAR *name(void) const
        {
            return typeid(*this).name();
        }

    protected:  // service methods

        virtual int init(int argc, ACE_TCHAR *argv[]); // Endpoint for All Argments

        virtual int close(u_long flags);

        virtual int svc(void);  // Run the ORB Reactor loop

        virtual int run(size_t threads);  // Run The ORB - Called from friendly ORBManager

    protected: // Bootstrap methods

        virtual void pre_init(PortableInterceptor::ORBInitInfo_ptr info);
        virtual void post_init(PortableInterceptor::ORBInitInfo_ptr info);

    private:   // Bootstrap Initializer (friendly)

        ORB(void); // Managed by friendly singleton

        /* This ensures the ORB Singleton is statefull before the extension framework is bootstrapped */
        static struct ORBInitializer : virtual PortableInterceptor::ORBInitializer, virtual CORBA::LocalObject
        {
            ORBInitializer(void)
            {
                PortableInterceptor::register_orb_initializer(this);
            }

            virtual void pre_init(PortableInterceptor::ORBInitInfo_ptr info)
            {
                TAF::ORBSingleton_type::instance()->pre_init(info);
            }

            virtual void post_init(PortableInterceptor::ORBInitInfo_ptr info)
            {
                TAF::ORBSingleton_type::instance()->post_init(info);
            }

        } orbInitializer_;
    };

    inline const CORBA::ORB_var & ORB::orb(void) const
    {
        return this->orb_;
    }

    inline const PortableServer::POA_var & ORB::rootPOA(void) const
    {
        return this->rootPOA_;
    }

    inline const PortableServer::POA_var & ORB::defaultPOA(void) const
    {
        return this->defaultPOA_;
    }

    inline const IORTable::Table_var & ORB::IORTable(void) const
    {
        return this->IORTable_;
    }

    inline CORBA::Object_var ORB::string_to_object(const std::string &s) const
    {
        return this->orb()->string_to_object(s.c_str());
    }

    inline CORBA::String_var ORB::object_to_string(CORBA::Object_ptr obj) const
    {
        return this->orb()->object_to_string(obj);
    }

    inline void ORB::register_initial_reference(const std::string &id, CORBA::Object_ptr obj)
    {
        this->orb()->register_initial_reference(id.c_str(), obj);
    }

    inline CORBA::Object_var ORB::unregister_initial_reference(const std::string &id)
    {
        return this->orb()->unregister_initial_reference(id.c_str());
    }

    /*********************************************************************************************/

    class TAF_Export ORBManager : public DAF::TaskExecutor, ORBSingleton_type
    {
        size_t orbThreads_;

    public:

        ORBManager(int argc, ACE_TCHAR *argv[]);
        virtual ~ORBManager(void);

        using ORBSingleton_type::instance;

        size_t orb_threads(void) const     // ORB Reactor Threads
        {
            return this->orbThreads_;
        }

        CORBA::ORB_ptr operator -> () const;
        operator CORBA::ORB_ptr const & () const;

        virtual int run(size_t orb_threads = TAF_MIN_THREADS, bool wait_for_completion = false);

    protected:

        virtual int init(int argc, ACE_TCHAR *argv[]);  // Initializes ORB Environment
        virtual int fini(void);

    protected:  // Must Be Sub-Classed

        ORBManager(void);       // Used By TAFServer

        virtual int svc(void);  // Run the ACE Reactor loop

        virtual int close(u_long flags);

        size_t set_orb_threads(size_t threads);  // ORB Reactor Threads
    };

}  // namespace TAF

/***************** define some helpers ****************/

#if !defined(TheTAFOrbInstance)
# define TheTAFOrbInstance                  (TAF::ORBManager::instance)
#endif
#if !defined(TheTAFOrb)
# define TheTAFOrb                          (TheTAFOrbInstance()->orb)
#endif
#if !defined(TheTAFRootPOA)
# define TheTAFRootPOA                      (TheTAFOrbInstance()->rootPOA)
#endif
#if !defined(TheTAFDefaultPOA)
# define TheTAFDefaultPOA                   (TheTAFOrbInstance()->defaultPOA)
#endif
#if !defined(TheTAFIORTable)
# define TheTAFIORTable                     (TheTAFOrbInstance()->IORTable)
#endif
#if !defined(TheTAFRootContext)
# define TheTAFRootContext                  (TheTAFOrbInstance()->rootContext)
#endif
#if !defined(TheTAFBaseContext)
# define TheTAFBaseContext                  (TheTAFOrbInstance()->baseContext)
#endif
#if !defined(TheTAFOrbCore)
# define TheTAFOrbCore                      (TheTAFOrb()->orb_core)
#endif
#if !defined(TAFObjectToString)
# define TAFObjectToString                  (TheTAFOrbInstance()->object_to_string)
#endif
#if !defined(TAFStringToObject)
# define TAFStringToObject                  (TheTAFOrbInstance()->string_to_object)
#endif
#if !defined(TAFRegisterInitialReference)
# define TAFRegisterInitialReference        (TheTAFOrbInstance()->register_initial_reference)
#endif
#if !defined(TAFUnRegisterInitialReference)
# define TAFUnRegisterInitialReference      (TheTAFOrbInstance()->unregister_initial_reference)
#endif
#if !defined(TAFResolveInitialReferences)
# define TAFResolveInitialReferences        (TheTAFOrbInstance()->resolve_initial_references)
#endif

TAF_SINGLETON_DECLARE(ACE_DLL_Singleton_T, TAF::ORB, ACE_SYNCH_MUTEX);

#endif // TAF_ORBMANAGER_H
