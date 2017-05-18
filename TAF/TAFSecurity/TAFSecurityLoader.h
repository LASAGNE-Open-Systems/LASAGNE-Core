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
#ifndef TAF_SECURITYLOADER_H
#define TAF_SECURITYLOADER_H

#include "TAFSecurity.h"

#include <daf/RefCount.h>
#include <daf/PropertyManager.h>

#include <ace/Service_Config.h>

#include <tao/PI/PI.h>
#include <tao/LocalObject.h>

namespace TAFSecurity
{
    class TAFSecurity_Export Loader : virtual public PortableInterceptor::ORBInitializer
        , virtual public CORBA::LocalObject
        , DAF::PropertyManager
    {
    public:

        Loader(int &argc, ACE_TCHAR *argv[], bool use_property = true);
        virtual ~Loader(void);

        int init(std::string &orb_params);

        DAF_DEFINE_REFCOUNTABLE(Loader);

    protected:

        virtual void pre_init(PortableInterceptor::ORBInitInfo_ptr info);
        virtual void post_init(PortableInterceptor::ORBInitInfo_ptr info);

    protected:

        virtual const char * config_switch(void) const
        {
            return TAF_SECURITY; // Not Used
        }
    };

} // namespace TAFSecurity

typedef class TAFSecurity::Loader     TAFSecurityLoader;

DAF_DECLARE_REFCOUNTABLE(TAFSecurityLoader);

#endif //#ifndef TAF_SECURITYLOADER_H
