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
#ifndef TAF_GESTALTSERVICE_IMPL_H
#define TAF_GESTALTSERVICE_IMPL_H

#include "TAF.h"

#include <daf/ServiceGestalt.h>
#include <daf/ServiceLoader.h>
#include <daf/TaskExecutor.h>

#include <ace/Service_Object.h>
#include <ace/Semaphore.h>

#include "GestaltServiceS.h"

#include <map>

namespace TAF
{
    typedef class TAF_Export GestaltService_impl : virtual public POA_taf::GestaltService
        , public DAF::ServiceGestalt
    {
    public:

        typedef std::string             ident_type;
        typedef std::list<ident_type>   ident_list_type;

        typedef class std::map<ident_type, taf::EntityDescriptor>   descriptor_map_type;

        GestaltService_impl(const ACE_TCHAR *program_name = 0);
        virtual ~GestaltService_impl(void);

        virtual taf::EntityDescriptor *     findService(const char *ident);
        virtual taf::EntityDescriptorSeq *  listServices(void);

        virtual CORBA::Long loadConfigFile(const char *file_arg, CORBA::Long_out count); // "filename:<section>,<section>"

        virtual void    loadStatic(const char *ident, const char *parameters);
        virtual void    loadDynamic(const char *ident, const char *libpathname, const char *objectclass, const char *parameters);
        virtual void    suspend(const char *ident);
        virtual void    resume(const char *ident);
        virtual void    remove(const char *ident);

        virtual CORBA::Long suspend_all(void);
        virtual CORBA::Long resume_all(void);
        virtual CORBA::Long remove_all(void);

    protected:

        virtual int suspend(void);
        virtual int resume(void);

        virtual int info(ACE_TCHAR **info_string, size_t length) const;

    protected:

        const ACE_Time_Value    loadTime_;

    private:

        descriptor_map_type svc_params_;

        taf::EntityDescriptor_var   makeEntityDescriptor(const ident_type &ident) const;

        int listServiceRepository(ident_list_type&, bool reverse = false) const;

        mutable ACE_SYNCH_RW_MUTEX lock_;  // Lock for Configuration (re)Processing / Access

    } GestaltServiceImpl;

    /****************************************************************************************/

    class TAF_Export GestaltServiceLoader : public DAF::ServiceLoader
    {
    public:

        GestaltServiceLoader(GestaltServiceImpl &gestalt) : gestalt_(gestalt)
        {}

    protected:

        virtual int load_service(const std::string &ident, const std::string &libpathname, const std::string &objectclass, const std::string &params);

    private:

        GestaltServiceImpl &gestalt_;

        const char * config_switch(void) const
        {
            return TAF_SERVICES;
        }
    };

} // namespace TAF

typedef class TAF::GestaltService_impl  TAFGestaltServiceImpl;

#endif // TAF_GESTALTSERVICE_IMPL_H
