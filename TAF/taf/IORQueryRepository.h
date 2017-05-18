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
#ifndef TAF_IORQUERYREPOSITORY_H
#define TAF_IORQUERYREPOSITORY_H

#include "IORQueryServant.h"

#include <ace/Singleton.h>

namespace TAF {

    class TAF_Export IORServantRepository; // Forward Declaration

    typedef ACE_DLL_Singleton_T<IORServantRepository, ACE_SYNCH_MUTEX>  IORServantRepositorySingleton;

    class TAF_Export IORServantRepository : public IORQueryServantList
    {
        mutable ACE_SYNCH_MUTEX lock_;

        friend IORServantRepositorySingleton; // Used to create repository

    public:

        virtual ~IORServantRepository(void);

        const ACE_TCHAR *dll_name(void) const
        {
            return TAF_DLL_NAME;
        }

        const ACE_TCHAR *name(void) const
        {
            return typeid(*this).name();
        }

        operator ACE_SYNCH_MUTEX & () const;

    public:

        int registerQueryService(CORBA::Object_ptr, const std::string &name);
        int unregisterQueryService(const std::string &name);

    protected:

        IORServantRepository(void); // Constructed by singleton
    };
}

TAF_SINGLETON_DECLARE(ACE_DLL_Singleton_T, TAF::IORServantRepository, ACE_SYNCH_MUTEX);

#define TheIORQueryRepository   (TAF::IORServantRepositorySingleton::instance)

#endif
