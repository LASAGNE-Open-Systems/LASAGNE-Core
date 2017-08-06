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
#define TAF_GESTALTSERVICE_IMPL_CPP

#include "GestaltService_impl.h"

#include "ORBManager.h"
#include "TAFDebug.h"

#include <daf/PropertyManager.h>
#include <daf/ServiceGestaltLoader.h>

#include <ace/Service_Types.h>
#include <ace/Arg_Shifter.h>

namespace
{
    int
    throw_last_error(int error = DAF_OS::last_error())
    {
        switch (DAF_OS::last_error(error)) {
            case 0:         break;
            case ETIME:     throw CORBA::TIMEOUT();             // Service Action timeout -> may still complete!!
            case EINVAL:    throw CORBA::BAD_PARAM();
            case ENOENT:    throw CORBA::INV_IDENT();
            case EEXIST:    throw CORBA::OBJECT_NOT_EXIST();    // Ident Does Not exist
            case ENOTSUP:   throw CORBA::NO_IMPLEMENT();
            case ENOLCK:    throw CORBA::NO_RESOURCES();        // Unable to lock service container
            case EACCES:    throw CORBA::NO_PERMISSION();       // Service not Accessable
            case ENOEXEC:   throw CORBA::INVALID_ACTIVITY();    // Failed ServiceAction handoff
            default:        throw CORBA::BAD_OPERATION();
        }
        return error;
    }
}

/******************************************************************************/

namespace TAF
{
    using DAF::ServiceGestalt;

    GestaltService_impl::GestaltService_impl(const ACE_TCHAR *program_name)
        : gestalt_(*this, program_name)
    {
        ACE_TEST_ASSERT(this->gestalt_.is_opened());
    }

    GestaltService_impl::~GestaltService_impl(void)
    {
    }

    taf::EntityDescriptor *
    GestaltService_impl::findService(const char *svc_ident)
    {
        if (svc_ident && DAF_OS::strlen(svc_ident) > 0) {
            return this->makeEntityDescriptor(svc_ident)._retn();
        }

        throw_last_error(EINVAL); return 0;
    }

    taf::EntityDescriptorSeq *
    GestaltService_impl::listServices(void)
    {
        service_list_type service_list;

        CORBA::ULong max_size = CORBA::ULong(this->gestalt_.list_service_repository(service_list, false));

        taf::EntityDescriptorSeq_var sd(new taf::EntityDescriptorSeq(max_size));

        CORBA::ULong index = 0; sd->length(index);
        for (service_list_type::const_iterator it(service_list.begin()); it != service_list.end(); it++) {
            sd->length(index + 1); sd[index++] = this->makeEntityDescriptor(*it);
        }

        return sd._retn();
    }

    CORBA::Long
    GestaltService_impl::loadConfigFile(const char *profile_arg, CORBA::Long_out count)
    {
        if (profile_arg && DAF_OS::strlen(profile_arg) > 0) try {
            DAF::ServiceGestaltLoader svcLoader(this->gestalt_);
            if (svcLoader.load_file_profile(DAF::trim_string(profile_arg)) == 0) {
                count = CORBA::Long(svcLoader.size()); return CORBA::Long(svcLoader.process_directives());
            }
        } catch (const CORBA::Exception &) {
            throw;
        } DAF_CATCH_ALL { /* Fall through to Bad Error */ }

        count = 0; return -1;
    }

    void
    GestaltService_impl::loadStatic(const char *ident, const char *parameters)
    {
        if (ident && DAF_OS::strlen(ident) > 0) {
            if (this->gestalt_.loadStatic(ident, (parameters ? parameters : "")) == 0) {
                return;
            }
        }
        else {
            DAF_OS::last_error(EINVAL);
        }

        throw_last_error();
    }

    void
    GestaltService_impl::loadDynamic(const char *ident, const char *libpathname, const char *objectclass, const char *parameters)
    {
        if (ident && DAF_OS::strlen(ident) > 0) {
            if (this->gestalt_.loadDynamic(ident, libpathname, objectclass, (parameters ? parameters : "")) == 0) {
                return;
            }
        }
        else {
            DAF_OS::last_error(EINVAL);
        }

        throw_last_error();
    }

    void
    GestaltService_impl::suspend(const char *ident)
    {
        if (ident && DAF_OS::strlen(ident) > 0) {
            if (this->gestalt_.suspendService(ident) == 0) {
                return;
            }
        }
        else {
            DAF_OS::last_error(EINVAL);
        }

        throw_last_error();
    }

    void
    GestaltService_impl::resume(const char *ident)
    {
        if (ident && DAF_OS::strlen(ident) > 0) {
            if (this->gestalt_.resumeService(ident) == 0) {
                return;
            }
        }
        else {
            DAF_OS::last_error(EINVAL);
        }

        throw_last_error();
    }

    void
    GestaltService_impl::remove(const char *ident)
    {
        if (ident && DAF_OS::strlen(ident) > 0) {
            if (this->gestalt_.removeService(ident) == 0) {
                return; // Dont report any other errors as Gestalt will ignore anyway
            }
        } else {
            DAF_OS::last_error(EINVAL);
        }

        throw_last_error();
    }

    CORBA::Long
    GestaltService_impl::suspend_all(void)
    {
        return CORBA::Long(this->gestalt_.suspendServiceAll());
    }

    CORBA::Long
    GestaltService_impl::resume_all(void)
    {
        return CORBA::Long(this->gestalt_.resumeServiceAll());
    }

    CORBA::Long
    GestaltService_impl::remove_all(void)
    {
        return CORBA::Long(this->gestalt_.removeServiceAll());
    }

    /**************************************************************************************************/

    taf::EntityDescriptor_var
    GestaltService_impl::makeEntityDescriptor(const ident_type &ident) const
    {
        ACE_READ_GUARD_REACTION(ACE_SYNCH_RW_MUTEX, mon, this->gestalt_.svc_descriptors(), throw_last_error(ENOLCK)); // Stop re-entrancy

        for (service_map_type::const_iterator it = this->gestalt_.svc_descriptors().find(ident); it != this->gestalt_.svc_descriptors().end();) {
            return this->makeEntityDescriptor(it->second)._retn();
        }

        throw_last_error(EEXIST); return 0;
    }

    taf::EntityDescriptor_var
    GestaltService_impl::makeEntityDescriptor(const service_descriptor_type &svc_desc) const
    {
        taf::EntityDescriptor_var ed(new taf::EntityDescriptor()); // Our taf::EntityDescriptor to return

        ed->ident_          = svc_desc.ident_.c_str();
        ed->libpathname_    = svc_desc.libpathname_.c_str();
        ed->objectclass_    = svc_desc.objectclass_.c_str();
        ed->parameters_     = svc_desc.parameters_.c_str();
        ed->info_           = "";
        ed->obj_            = CORBA::Object::_nil();
        ed->loadTime_.sec   = CORBA::ULongLong(svc_desc.loadtime_.sec());
        ed->loadTime_.usec  = CORBA::ULong(svc_desc.loadtime_.usec());
        ed->flags_          = CORBA::UShort(0);

        if (ACE_BIT_ENABLED(svc_desc.loadflags_, ServiceGestalt::LOAD_EXECUTE)) {
            ed->flags_ |= CORBA::UShort(1U << taf::SVC_EXECUTE);
        }
        if (ACE_BIT_ENABLED(svc_desc.loadflags_, ServiceGestalt::LOAD_STATIC))  {
            ed->flags_ |= CORBA::UShort(1U << taf::SVC_STATIC);
        }
        if (ACE_BIT_ENABLED(svc_desc.loadflags_, ServiceGestalt::LOAD_DYNAMIC)) {
            ed->flags_ |= CORBA::UShort(1U << taf::SVC_DYNAMIC);
        }

        ACE_Service_Repository *repo = const_cast<GestaltService &>(this->gestalt_).current_service_repository();

        if (repo) { // Scope Lock

            ACE_GUARD_REACTION(ACE_Recursive_Thread_Mutex, mon, this->gestalt_.repository_lock(repo), throw_last_error(ENOLCK));

            const ACE_Service_Type *svc_type = 0;

            if (repo->find(ed->ident_.in(), &svc_type, false) == 0 && svc_type) {

                if (svc_type->active())         { ed->flags_ |= CORBA::UShort(1U << taf::SVC_ACTIVE); }
                if (svc_type->fini_called())    { ed->flags_ |= CORBA::UShort(1U << taf::SVC_FINIED); }

                const ACE_Service_Type_Impl *svc_type_impl = svc_type->type();

                if (svc_type_impl) {
                    switch (svc_type_impl->service_type()) {
                    case ACE_Service_Type::MODULE:          ed->flags_ |= CORBA::UShort(1U << taf::SVC_MODULE); break;
                    case ACE_Service_Type::STREAM:          ed->flags_ |= CORBA::UShort(1U << taf::SVC_STREAM); break;
                    case ACE_Service_Type::SERVICE_OBJECT:  ed->flags_ |= CORBA::UShort(1U << taf::SVC_OBJECT);
                        try {
                            svc_type_impl->info(&ed->info_.out(), BUFSIZ);
                            ACE_Service_Object *so = static_cast<ACE_Service_Object*>(svc_type_impl->object());
                            if (so) {
                                PortableServer::Servant sb = dynamic_cast<PortableServer::Servant>(so);
                                if (sb) {
                                    for (PortableServer::POA_var poa(sb->_default_POA()); poa;) {
                                        ed->obj_ = poa->servant_to_reference(sb); break;
                                    }
                                }
                            }
                        } DAF_CATCH_ALL { /* Ignore */ } break;
                    }
                }
            }
        }

        return ed._retn();
    }

} // namespace TAF
