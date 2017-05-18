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
#ifndef TAF_NAMINGSERVICE_H
#define TAF_NAMINGSERVICE_H

#include "NamingService_export.h"

#include "taf/CORBAInterfaceHandler_T.h"

#include <orbsvcs/Naming/Naming_Server.h>

#include <ace/Service_Object.h>
#include <ace/Service_Config.h>

namespace TAF
{
    typedef class CORBAInterfaceHandler_T<POA_CosNaming::NamingContext>     NamingInterfaceHandler;

    class NamingService_Export NamingService : virtual public NamingInterfaceHandler
        , public ACE_Service_Object, TAO_Naming_Server
    {
    public:

        NamingService(void);
        ~NamingService(void);

        static const char * svc_ident(void)
        {
            return ACE_TEXT("TAFNamingService");
        }

    protected:          // CosNaming::NamingContext IDL Methods

        virtual void bind(const CosNaming::Name &, CORBA::Object_ptr);

        virtual void rebind(const CosNaming::Name &, CORBA::Object_ptr);

        virtual void bind_context(const CosNaming::Name &, CosNaming::NamingContext_ptr);

        virtual void rebind_context(const CosNaming::Name &, CosNaming::NamingContext_ptr);

        virtual CORBA::Object_ptr   resolve(const CosNaming::Name &);

        virtual void unbind(const CosNaming::Name &);

        virtual CosNaming::NamingContext_ptr new_context(void);

        virtual CosNaming::NamingContext_ptr bind_new_context(const CosNaming::Name &);

        virtual void destroy(void);

        virtual void list(CORBA::ULong how_many, CosNaming::BindingList_out bl, CosNaming::BindingIterator_out bi);

    protected:        // Service methods

        /// Initializes object when dynamic linking occurs.
        virtual int init(int argc, ACE_TCHAR *argv[]);

        /// Suspends object.
        virtual int suspend(void);

        /// Resume object.
        virtual int resume(void);

        /// Terminates object when dynamic unlinking occurs.
        virtual int fini(void);

        /// Returns information on a service object.
        virtual int info(ACE_TCHAR **info_string, size_t length = 0) const;
    };
} // namespace TAF

typedef class TAF::NamingService  TAF_NamingService;

ACE_FACTORY_DECLARE(NamingService, TAF_NamingService);
ACE_STATIC_SVC_DECLARE_EXPORT(NamingService, TAF_NamingService);

#endif
