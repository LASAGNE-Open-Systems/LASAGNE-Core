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
#ifndef TAF_POAPOLICYLIST_H
#define TAF_POAPOLICYLIST_H

#include "TAF.h"

#include <tao/PortableServer/PortableServer.h>

namespace TAF
{
    class TAF_Export POAPolicyList : public CORBA::PolicyList
    {
    public:

        POAPolicyList(const PortableServer::POA_var &poa, CORBA::ULong max = 8);

        virtual ~POAPolicyList(void);

        PortableServer::POA_ptr createPOA(const std::string &poaName) const;

        POAPolicyList & operator += (const PortableServer::ThreadPolicyValue &);
        POAPolicyList & operator += (const PortableServer::LifespanPolicyValue &);
        POAPolicyList & operator += (const PortableServer::IdUniquenessPolicyValue &);
        POAPolicyList & operator += (const PortableServer::IdAssignmentPolicyValue &);
        POAPolicyList & operator += (const PortableServer::ServantRetentionPolicyValue &);
        POAPolicyList & operator += (const PortableServer::RequestProcessingPolicyValue &);
        POAPolicyList & operator += (const PortableServer::ImplicitActivationPolicyValue &);

    protected:

        POAPolicyList&  append(const CORBA::Policy_var&);

    private:

        const PortableServer::POA_var poa_;
    };
} // namespace TAF

#endif // TAF_POAPOLICYLIST_H
