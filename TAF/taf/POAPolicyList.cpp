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
#define TAF_POAPOLICYLIST_CPP

#include "POAPolicyList.h"

#include "ORBManager.h"

namespace TAF
{
    POAPolicyList::POAPolicyList(const PortableServer::POA_var &poa, CORBA::ULong max)
        : CORBA::PolicyList(max), poa_(poa)
    {
        this->length(0); if (CORBA::is_nil(poa.in())) {
            DAF_THROW_EXCEPTION(DAF::IllegalArgumentException);
        }
    }

    POAPolicyList::~POAPolicyList(void)
    {
        for (CORBA::ULong i = 0; i < this->length(); ++i) {
            (*this)[i]->destroy();
        }
    }

    PortableServer::POA_ptr
    POAPolicyList::createPOA(const std::string &poaName) const
    {
        for (PortableServer::POAManager_var poaMGR(this->poa_->the_POAManager()); poaMGR;) {
            return this->poa_->create_POA(poaName.c_str(), poaMGR, *this);
        }
        DAF_THROW_EXCEPTION(DAF::IllegalStateException);
    }

    POAPolicyList&  POAPolicyList::append(const CORBA::Policy_var &policy)
    {
        CORBA::ULong i = this->length(); this->length(i + 1); (*this)[i] = policy; return *this;
    }

    POAPolicyList&
    POAPolicyList::operator += (const ::PortableServer::ThreadPolicyValue &val)
    {
        return this->append(this->poa_->create_thread_policy(val));
    }

    POAPolicyList&
    POAPolicyList::operator += (const ::PortableServer::LifespanPolicyValue &val)
    {
        return this->append(this->poa_->create_lifespan_policy(val));
    }

    POAPolicyList&
    POAPolicyList::operator += (const ::PortableServer::IdUniquenessPolicyValue &val)
    {
        return this->append(this->poa_->create_id_uniqueness_policy(val));
    }

    POAPolicyList&
    POAPolicyList::operator += (const ::PortableServer::IdAssignmentPolicyValue &val)
    {
        return this->append(this->poa_->create_id_assignment_policy(val));
    }

    POAPolicyList&
    POAPolicyList::operator += (const ::PortableServer::ServantRetentionPolicyValue &val)
    {
        return this->append(this->poa_->create_servant_retention_policy(val));
    }

    POAPolicyList&
    POAPolicyList::operator += (const ::PortableServer::RequestProcessingPolicyValue &val)
    {
        return this->append(this->poa_->create_request_processing_policy(val));
    }

    POAPolicyList&
    POAPolicyList::operator += (const ::PortableServer::ImplicitActivationPolicyValue &val)
    {
        return this->append(this->poa_->create_implicit_activation_policy(val));
    }
} // namespace TAF
