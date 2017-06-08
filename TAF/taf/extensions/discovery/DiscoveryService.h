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
#ifndef TAF_DISCOVERYSERVICE_H
#define TAF_DISCOVERYSERVICE_H

#include "TAFDiscovery_export.h"

#include "DiscoveryHandler.h"

#include <daf/TaskExecutor.h>

#include <ace/Service_Config.h>
#include <ace/Service_Object.h>

namespace TAF
{
    class TAFDiscovery_Export DiscoveryService : public DAF::TaskExecutor
    {
        TAFDiscoveryHandler handler_;

    public:

        enum {
            SEND_QUERYREPLY_TIMEOUT  = 5 // Seconds
        };

        DiscoveryService(void);
        ~DiscoveryService(void);

        bool isActive(void) const
        {
            return this->active_;
        }

        static const char * svc_ident(void)
        {
            return taf::TAFDISCOVERY_OID;
        }

    protected: /* Service Interface */

        /// Initializes object when dynamic linking occurs.
        virtual int init(int argc, ACE_TCHAR *argv[]);

        /// Terminates object when dynamic unlinking occurs.
        virtual int fini(void);

        /// Returns information on a service object.
        virtual int info(ACE_TCHAR **info_string, size_t length = 0) const;

    protected:

        virtual ACE_HANDLE get_handle(void) const;

        virtual int handle_input(ACE_HANDLE);

    private:

        bool   active_;
    };

} // namespace TAF

typedef class TAF::DiscoveryService  TAFDiscoveryService;

ACE_FACTORY_DECLARE(TAFDiscovery, TAFDiscoveryService);
ACE_STATIC_SVC_DECLARE_EXPORT(TAFDiscovery, TAFDiscoveryService);

#endif
