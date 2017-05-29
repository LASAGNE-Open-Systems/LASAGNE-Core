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

#ifndef TAF_XMPL_CORBASERVICE_H
#define TAF_XMPL_CORBASERVICE_H

#include "CORBAService_export.h"

#include <taf/CORBAInterfaceHandler_T.h>

#include <ace/Service_Object.h>
#include <ace/Service_Config.h>

#include "CORBAServiceS.h"

namespace TAF_XMPL
{
    typedef class TAF::CORBAInterfaceHandler_T<POA_taf_xmpl::SimpleServer>  SimpleServerInterfaceHandler;

    class CORBAService_Export CORBAService : virtual public SimpleServerInterfaceHandler
        , public ACE_Service_Object
    {
        int parse_args(int argc, ACE_TCHAR *argv[]);

    public:

        CORBAService(void);
        virtual ~CORBAService(void);

        static const ACE_TCHAR  *svc_ident(void)
        {
            return ACE_TEXT("TAF_XMPL_CORBAService");
        }

        /* IDL methods */

        virtual void register_callback(taf_xmpl::SimpleCallback_ptr);

        virtual CORBA::Long test_method(CORBA::Long x, const taf_xmpl::Structure& in_struct, taf_xmpl::Structure_out out_struct, char *&name);

        virtual void test_callback(taf_xmpl::SimpleCallback_ptr callback_ptr, const char *text);

        virtual void test_oneway(const char *name);

        virtual CORBA::Long test_val(void);

        virtual void test_val(CORBA::Long test_val);

        virtual void raise_user_exception(void);

        virtual void raise_system_exception(void);

    protected:

        /// Initializes object when dynamic linking occurs.
        virtual int init(int argc, ACE_TCHAR *argv[]);

        virtual int suspend(void)   { DAF_OS::last_error(ENOTSUP); return -1; }

        virtual int resume(void)    { DAF_OS::last_error(ENOTSUP); return -1; }

        /// Terminates object when dynamic unlinking occurs.
        virtual int fini(void);

        /// Returns information on a service object.
        virtual int info(ACE_TCHAR **info_string, size_t length = 0) const;

    private:

        CORBA::Long   vlong_;
        std::string   svc_args_;
    };
} // namespace TAF_XMPL

typedef class TAF_XMPL::CORBAService    TAF_XMPL_CORBAService; // Typedef out namespace for service macros

ACE_FACTORY_DECLARE(CORBAService, TAF_XMPL_CORBAService);
ACE_STATIC_SVC_DECLARE_EXPORT(CORBAService, TAF_XMPL_CORBAService);

#endif // TAF_XMPL_CORBASERVICE_H
