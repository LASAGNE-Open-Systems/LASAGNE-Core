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

#include <daf/DateTime.h>
#include <daf/PropertyManager.h>
#include <daf/ServiceLoader.h>

#include <ace/Service_Types.h>
#include <ace/Arg_Shifter.h>

namespace
{
    struct TAF_EntityDescriptor : taf::EntityDescriptor {
        TAF_EntityDescriptor(   const std::string &ident,
                                const std::string &libpathname,
                                const std::string &objectclass,
                                const std::string &parameters,
                                CORBA::UShort flags = 0);
        TAF_EntityDescriptor(const taf::EntityDescriptor &ed)
            : taf::EntityDescriptor(ed)
        {}
    };

    TAF_EntityDescriptor::TAF_EntityDescriptor( const std::string &ident,
                                                const std::string &libpathname,
                                                const std::string &objectclass,
                                                const std::string &parameters,
                                                CORBA::UShort flags)
    {
        const ACE_Time_Value  loadTime(DAF_Date_Time::GMTime());
        this->loadTime_.sec     = CORBA::ULongLong(loadTime.sec());
        this->loadTime_.usec    = CORBA::ULong(loadTime.usec());
        this->ident_            = ident.c_str();
        this->libpathname_      = libpathname.c_str();
        this->objectclass_      = objectclass.c_str();
        this->parameters_       = parameters.c_str();
        this->info_             = "";
        this->obj_              = 0;
        this->flags_            = flags;
    }
}

/******************************************************************************/

namespace TAF
{
    GestaltService_impl::GestaltService_impl(const ACE_TCHAR *program_name)
        : DAF::ServiceGestalt(program_name), loadTime_(DAF_Date_Time::GMTime())
    {
        ACE_TEST_ASSERT(this->is_opened());
    }

    GestaltService_impl::~GestaltService_impl(void)
    {
    }

    taf::EntityDescriptor_var
    GestaltService_impl::makeEntityDescriptor(const ident_type &ident) const
    {
        // NOTE: READ Lock Held on entry

        ACE_Service_Repository *repo = const_cast<GestaltService_impl*>(this)->current_service_repository();

        descriptor_map_type::const_iterator it = this->svc_params_.find(ident);

        if (repo && it != this->svc_params_.end()) {
            taf::EntityDescriptor_var sd(new TAF_EntityDescriptor(it->second)); sd->obj_ = CORBA::Object::_nil();

#if defined(ACE_HAS_SERVICE_REPOSITORY_LOCK_ACCESS)
            ACE_GUARD_REACTION(ACE_Recursive_Thread_Mutex, taf_mon, repo->lock(), throw CORBA::BAD_OPERATION());
#else
            ACE_GUARD_REACTION(ACE_Recursive_Thread_Mutex, taf_mon, DAF::getRepositoryLock(repo), throw CORBA::BAD_OPERATION());
#endif
            const ACE_Service_Type *svc_type = 0;
            if (this->find(ident.c_str(), &svc_type, false) == 0 && svc_type) {
                if (svc_type->active())         sd->flags_ |= (1U << taf::SVC_ACTIVE);
                if (svc_type->fini_called())    sd->flags_ |= (1U << taf::SVC_FINIED);

                const ACE_Service_Type_Impl *svc_type_impl = svc_type->type();

                if (svc_type_impl) {
                    switch (svc_type_impl->service_type()) {
                    case ACE_Service_Type::MODULE:          sd->flags_ |= (1U << taf::SVC_MODULE); break;
                    case ACE_Service_Type::STREAM:          sd->flags_ |= (1U << taf::SVC_STREAM); break;
                    case ACE_Service_Type::SERVICE_OBJECT:  sd->flags_ |= (1U << taf::SVC_OBJECT);
                        try {
                            svc_type_impl->info(&sd->info_.out(), BUFSIZ);
                            ACE_Service_Object *so = static_cast<ACE_Service_Object*>(svc_type_impl->object());
                            if (so) {
                                PortableServer::Servant sb = dynamic_cast<PortableServer::Servant>(so);
                                if (sb) for (PortableServer::POA_var poa(sb->_default_POA()); poa;) {
                                    sd->obj_ = poa->servant_to_reference(sb); break;
                                }
                            }
                        }  DAF_CATCH_ALL { /* Ignore */ }
                        break;
                    }
                }
                return sd._retn();  // Return as much as we have
            }
        }

        throw CORBA::INV_IDENT();
    }

    int
    GestaltService_impl::listServiceRepository(ident_list_type &svc_list, bool reverse) const
    {
        svc_list.clear();

        ACE_READ_GUARD_RETURN(ACE_SYNCH_RW_MUTEX, taf_mon, this->lock_, 0);  // Stop re-entrancy

        ACE_Service_Repository *repo = const_cast<GestaltService_impl*>(this)->current_service_repository();

        if (repo) do {
#if defined(ACE_HAS_SERVICE_REPOSITORY_LOCK_ACCESS)
            ACE_GUARD_REACTION(ACE_Recursive_Thread_Mutex, ace_mon, repo->lock(), break);
#else
            ACE_GUARD_REACTION(ACE_Recursive_Thread_Mutex, ace_mon, DAF::getRepositoryLock(repo), break);
#endif
            ACE_Service_Repository_Iterator it(*repo, false);
            const ACE_Service_Type *svc_type = 0; do {
                if (it.next(svc_type)) {
                    const std::string svc_ident(svc_type->name());
                    if (this->svc_params_.find(svc_ident) != this->svc_params_.end()) {
                        reverse ? svc_list.push_front(svc_ident) : svc_list.push_back(svc_ident);
                    }
                } else break;
            } while (it.advance());
        } while(false);

        return int(svc_list.size());
    }

    taf::EntityDescriptor *
    GestaltService_impl::findService(const char *svc_ident)
    {
        if (svc_ident && DAF_OS::strlen(svc_ident) > 0) do {
            ACE_READ_GUARD_REACTION(ACE_SYNCH_RW_MUTEX, taf_mon, this->lock_, throw CORBA::BAD_OPERATION()); // Stop re-entrancy
            return this->makeEntityDescriptor(svc_ident)._retn();
        } while (false);

        throw CORBA::INV_IDENT();
    }

    taf::EntityDescriptorSeq *
    GestaltService_impl::listServices(void)
    {
        ident_list_type ident_list;

        taf::EntityDescriptorSeq_var sd(new taf::EntityDescriptorSeq(CORBA::ULong(this->svc_params_.size() + 1))); sd->length(0);

        CORBA::ULong index = 0;
        if (this->listServiceRepository(ident_list, false) > 0) do {
            ACE_READ_GUARD_REACTION(ACE_SYNCH_RW_MUTEX, taf_mon, this->lock_, throw CORBA::BAD_OPERATION()); // Stop re-entrancy
            for (ident_list_type::const_iterator it(ident_list.begin()); it != ident_list.end(); it++) {
                if (sd->maximum() > index) try {
                    taf::EntityDescriptor_var ed(this->makeEntityDescriptor(*it)); // May Throw if we dont know it
                    sd->length(index + 1); // Increment the length
                    sd[index++] = ed;      // Save EntityDescriptor
                }
                catch (const CORBA::Exception &) {
                    throw;
                } DAF_CATCH_ALL{
                    /* Ignore and Loop Again on next */
                } else break;
            }
        } while(false);

        return sd._retn();
    }

    CORBA::Long
    GestaltService_impl::loadConfigFile(const char *profile_arg, CORBA::Long_out count)
    {
        if (profile_arg && DAF_OS::strlen(profile_arg) > 0) try {
            GestaltServiceLoader svcLoader(*this);
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
        if (ident && DAF_OS::strlen(ident)) do {

            const std::string svc_ident(DAF::trim_string(ident));

            if (svc_ident.length() == 0 || int(svc_ident.find_first_of(' ')) > 0) {
                break;
            }

            const std::string svc_params(DAF::parse_args(parameters, true));

            const std::string command(ServiceGestalt::static_directive(svc_ident.c_str(), svc_params.c_str()));

            const TAF_EntityDescriptor  svc_entity(svc_ident, "", "", svc_params, (1U << taf::SVC_STATIC));

            do try { // Scope Lock

                ACE_WRITE_GUARD_REACTION(ACE_SYNCH_RW_MUTEX, taf_mon, this->lock_, throw CORBA::BAD_OPERATION()); // Stop re-entrancy

                const ACE_Service_Type *svc_type = 0;

                if (this->find(svc_ident.c_str(), &svc_type, false) == 0) {
                    DAF_OS::last_error(EEXIST); break;
                }

                if (this->process_command(command, DEFAULT_SLOAD_TIMEOUT)) {
                    break;
                }

                this->svc_params_[svc_ident] = svc_entity; return; // All Done

            }
            catch (const CORBA::Exception &) {
                throw;
            } DAF_CATCH_ALL{
            } while (false);

            if (TAF::debug()) {
                ACE_DEBUG((LM_WARNING,
                    ACE_TEXT("TAF::GestaltService - Failure to load static service '%s' %s\n")
                    , svc_ident.c_str(), DAF::last_error_text().c_str()));
            }

            switch (DAF_OS::last_error()) {
            case ETIME: throw CORBA::TIMEOUT();
            default:    throw CORBA::BAD_OPERATION();
            }

        } while (false);

        throw CORBA::BAD_PARAM();
    }

    void
    GestaltService_impl::loadDynamic(const char *ident, const char *libpathname, const char *objectclass, const char *parameters)
    {
        if (ident && DAF_OS::strlen(ident)) do {

            const std::string svc_ident(DAF::trim_string(ident));

            if (svc_ident.length() == 0 || int(svc_ident.find_first_of(' ')) > 0) {
                break;
            }

            const std::string objclass(DAF::trim_string(objectclass));

            if (objclass.length() == 0 || int(objclass.find_first_of(' ')) > 0) {
                break;
            }

            std::string libpath(DAF::parse_args(libpathname, true));

            if (libpath.length() == 0) {
                break;
            }
            else if (int(libpath.find_first_of(' ')) > 0) {
                for (int pos = int(libpath.length()); pos-- > 0;) {
                    if (libpath[0] == '\"' && libpath[pos] == '\"') {
                        if (pos) { libpath.erase(pos--, 1); } libpath.erase(0, 1); // Balance erase from both ends
                    }
                    else if (libpath[0] == '\'' && libpath[pos] == '\'') {
                        if (pos) { libpath.erase(pos--, 1); } libpath.erase(0, 1); // Balance erase from both ends
                    }
                    else if (::isspace(int(libpath[0]))) {
                        libpath.erase(0, 1);
                    }
                    else if (::isspace(int(libpath[pos]))) {
                        libpath.erase(pos, 1);
                    }
                    else break;
                }
                if (libpath.length()) { libpath.insert(0, "'").append("'"); } else break;
            }

            const std::string svc_params(DAF::parse_args(parameters, true));

            const std::string command(ServiceGestalt::dynamic_directive(svc_ident.c_str(), libpath.c_str(), objclass.c_str(), svc_params.c_str()));

            const TAF_EntityDescriptor  svc_entity(svc_ident, libpath, objclass, svc_params, (1U << taf::SVC_DYNAMIC));

            do try { // Scope Lock

                ACE_WRITE_GUARD_REACTION(ACE_SYNCH_RW_MUTEX, taf_mon, this->lock_, throw CORBA::BAD_OPERATION()); // Stop re-entrancy

                const ACE_Service_Type *svc_type = 0;

                if (this->find(svc_ident.c_str(), &svc_type, false) == 0) {
                    DAF_OS::last_error(EEXIST); break;
                }

                if (this->process_command(command, DEFAULT_DLOAD_TIMEOUT)) {
                    break;
                }

                this->svc_params_[svc_ident] = svc_entity; return; // All Done

            } catch (const CORBA::Exception &) {
                throw;
            } DAF_CATCH_ALL {
            } while (false);

            if (TAF::debug()) {
                ACE_DEBUG((LM_WARNING,
                    ACE_TEXT("TAF::GestaltService - Failure to load dynamic service '%s' %s\n")
                    , svc_ident.c_str(), DAF::last_error_text().c_str()));
            }

            switch (DAF_OS::last_error()) {
            case ETIME: throw CORBA::TIMEOUT();
            default:    throw CORBA::BAD_OPERATION();
            }

        } while (false);

        throw CORBA::BAD_PARAM();
    }

    void
    GestaltService_impl::suspend(const char *ident)
    {
        if (ident && DAF_OS::strlen(ident)) do {

            const std::string svc_ident(DAF::trim_string(ident));

            if (svc_ident.length() == 0 || int(svc_ident.find_first_of(' ')) > 0) {
                break;
            }

            const std::string command(ServiceGestalt::suspend_directive(svc_ident.c_str()));

            do try { // Scope Lock

                ACE_READ_GUARD_REACTION(ACE_SYNCH_RW_MUTEX, taf_mon, this->lock_, throw CORBA::BAD_OPERATION()); // Stop re-entrancy

                const ACE_Service_Type *svc_type = 0;

                if (this->find(svc_ident.c_str(), &svc_type, false) || svc_type == 0) {
                    DAF_OS::last_error(EEXIST); break;
                }

                if (svc_type->fini_called()) {
                    DAF_OS::last_error(EACCES); break;
                }

                if (svc_type->active() ? this->process_command(command, DEFAULT_SUSPEND_TIMEOUT) : 0) {
                    const_cast<ACE_Service_Type*>(svc_type)->active(true); // Set back to active
                    switch (DAF_OS::last_error()) {
                    case EINVAL: DAF_OS::last_error(ENOTSUP); // Gestalt sets EINVAL when -1 returned
                    default: continue;
                    }
                }

                return;

            }
            catch (const CORBA::Exception &) {
                throw;
            } DAF_CATCH_ALL{
            } while (false);

            if (TAF::debug()) {
                ACE_DEBUG((LM_WARNING,
                    ACE_TEXT("TAF::GestaltService - Failure to suspend service '%s' %s\n")
                    , svc_ident.c_str(), DAF::last_error_text().c_str()));
            }

            switch (DAF_OS::last_error()) {
            case ENOTSUP:   throw CORBA::NO_IMPLEMENT();
            case ETIME:     throw CORBA::TIMEOUT();
            default:        throw CORBA::BAD_OPERATION();
            }

        } while (false);

        throw CORBA::BAD_PARAM();
    }

    void
    GestaltService_impl::resume(const char *ident)
    {
        if (ident && DAF_OS::strlen(ident)) do {

            const std::string svc_ident(DAF::trim_string(ident));

            if (svc_ident.length() == 0 || int(svc_ident.find_first_of(' ')) > 0) {
                break;
            }

            const std::string command(ServiceGestalt::resume_directive(svc_ident.c_str()));

            do try { // Scope Lock

                ACE_READ_GUARD_REACTION(ACE_SYNCH_RW_MUTEX, taf_mon, this->lock_, throw CORBA::BAD_OPERATION()); // Stop re-entrancy

                const ACE_Service_Type *svc_type = 0;

                if (this->find(svc_ident.c_str(), &svc_type, false) || svc_type == 0) {
                    DAF_OS::last_error(EEXIST); break;
                }

                if (svc_type->fini_called()) {
                    DAF_OS::last_error(EACCES); break;
                }

                if (svc_type->active() ? 0 : this->process_command(command, DEFAULT_RESUME_TIMEOUT)) {
                    const_cast<ACE_Service_Type*>(svc_type)->active(false);  // Set back to in-active
                    switch (DAF_OS::last_error()) {
                    case EINVAL: DAF_OS::last_error(ENOTSUP); // Gestalt sets EINVAL when -1 returned
                    default: continue;
                    }
                }

                return; // Noop if already active

            }
            catch (const CORBA::Exception &) {
                throw;
            } DAF_CATCH_ALL{
            } while (false);

            if (TAF::debug()) {
                ACE_DEBUG((LM_WARNING,
                    ACE_TEXT("TAF::GestaltService - Failure to resume service '%s' %s\n")
                    , svc_ident.c_str(), DAF::last_error_text().c_str()));
            }

            switch (DAF_OS::last_error()) {
            case ENOTSUP:   throw CORBA::NO_IMPLEMENT();
            case ETIME:     throw CORBA::TIMEOUT();
            default:        throw CORBA::BAD_OPERATION();
            }

        } while (false);

        throw CORBA::BAD_PARAM();
    }

    void
    GestaltService_impl::remove(const char *ident)
    {
        if (ident && DAF_OS::strlen(ident)) do {

            const std::string svc_ident(DAF::trim_string(ident));

            if (svc_ident.length() == 0 || int(svc_ident.find_first_of(' ')) > 0) {
                break;
            }

            const std::string command(ServiceGestalt::remove_directive(svc_ident.c_str()));

            do try { // Scope Lock

                ACE_WRITE_GUARD_REACTION(ACE_SYNCH_RW_MUTEX, taf_mon, this->lock_, throw CORBA::BAD_OPERATION()); // Stop re-entrancy

                // If we know about it remove it from our list
                for (descriptor_map_type::iterator it = this->svc_params_.find(svc_ident); it != this->svc_params_.end();) {
                    this->svc_params_.erase(it); break; // Remove it from our load list
                }

                const ACE_Service_Type *svc_type = 0;

                if (this->find(svc_ident.c_str(), &svc_type, false) || svc_type == 0) {
                    DAF_OS::last_error(EEXIST); break;
                }

                if (svc_type->fini_called() ? 0 : this->process_command(command, DEFAULT_REMOVE_TIMEOUT)) {
                    break;
                }

                return; // All Done

            }
            catch (const CORBA::Exception &) {
                throw;
            } DAF_CATCH_ALL{
            } while (false);

            if (TAF::debug()) {
                ACE_DEBUG((LM_WARNING,
                    ACE_TEXT("TAF::GestaltService - Failure to remove service '%s' %s\n")
                    , svc_ident.c_str(), DAF::last_error_text().c_str()));
            }

            switch (DAF_OS::last_error()) {
            case ETIME: throw CORBA::TIMEOUT();
            default:    throw CORBA::BAD_OPERATION();
            }

        } while (false);

        throw CORBA::BAD_PARAM();
    }

    CORBA::Long
    GestaltService_impl::suspend_all(void)
    {
        ident_list_type svc_list;

        CORBA::Long fails = 0;
        if (this->listServiceRepository(svc_list, true) > 0) {
            for (ident_list_type::const_iterator it(svc_list.begin()); it != svc_list.end(); it++) {
                try { this->suspend(it->c_str()); DAF_OS::thr_yield(); } DAF_CATCH_ALL { fails++; }
            }
        }

        return fails;
    }

    CORBA::Long
        GestaltService_impl::resume_all(void)
    {
        ident_list_type svc_list;

        CORBA::Long fails = 0;
        if (this->listServiceRepository(svc_list, true) > 0) {
            for (ident_list_type::const_iterator it(svc_list.begin()); it != svc_list.end(); it++) {
                try { this->resume(it->c_str()); DAF_OS::thr_yield(); } DAF_CATCH_ALL { fails++; }
            }
        }

        return fails;
    }

    CORBA::Long
    GestaltService_impl::remove_all(void)
    {
        ident_list_type svc_list;

        CORBA::Long fails = 0;
        if (this->listServiceRepository(svc_list, true) > 0) {
            for (ident_list_type::const_iterator it(svc_list.begin()); it != svc_list.end(); it++) {
                try { this->remove(it->c_str()); DAF_OS::thr_yield(); } DAF_CATCH_ALL { fails++; }
            }
        }

        return fails;
    }

    int
    GestaltService_impl::suspend(void)
    {
        return int(this->suspend_all());
    }

    int
    GestaltService_impl::resume(void)
    {
        return int(this->resume_all());
    }

    int
    GestaltService_impl::info(ACE_TCHAR **info_string, size_t length) const
    {
        static const char *info_desc =
        {
            "A ServiceGestalt is managed as an object server of dynamically loaded components through the TAF framework services."
        };

        return (info_string ? (*info_string = ACE::strnnew(info_desc, length), 0) : -1);
    }

    /**************************************************************************************************/
    int
    GestaltServiceLoader::load_service( const std::string &ident,
                                        const std::string &libpathname,
                                        const std::string &objectclass,
                                        const std::string &params)
    {
        this->gestalt_.loadDynamic(ident.c_str(), libpathname.c_str(), objectclass.c_str(), params.c_str()); return 0;
    }


} // namespace TAF
