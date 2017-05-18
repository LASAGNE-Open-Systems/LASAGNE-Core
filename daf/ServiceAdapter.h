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
#ifndef DAF_SERVICEADAPTER_H
#define DAF_SERVICEADAPTER_H

/**
* @file     ServiceAdapter.h
* @author   Derek Dominish
* @author   $LastChangedBy$
* @date     1st September 2011
* @version  $Revision$
* @ingroup
* @namespace DAF
*/

#include <ace/Service_Config.h>
#include <ace/Service_Object.h>

#include <memory>

namespace DAF
{
    /** @class ServiceAdapter
    *@brief Brief \todo{Fill this in}
    *
    * Details \todo{Detailed description}
    */
    template <typename T>
    class ServiceAdapter : public ACE_Service_Object
    {
        std::auto_ptr<T> tp;

    public:

        /** \todo{Fill this in} */
        ServiceAdapter(void) : ACE_Service_Object()
        {
            try { tp = std::auto_ptr<T>(new T()); } DAF_CATCH_ALL { }
        }

        /** \todo{Fill this in} */
        virtual int init(int argc, ACE_TCHAR *argv[])
        {
            for (ACE_Service_Object *p = dynamic_cast<ACE_Service_Object*>(this->tp.get()); p;) try {
                return p->init(argc, argv);
            } DAF_CATCH_ALL {}
            return -1;
        }

        /** \todo{Fill this in} */
        virtual int suspend(void)
        {
            for (ACE_Service_Object *p = dynamic_cast<ACE_Service_Object*>(this->tp.get()); p;) try {
                return p->suspend();
            } DAF_CATCH_ALL {}
            return -1;
        }

        /** \todo{Fill this in} */
        virtual int resume(void)
        {
            for (ACE_Service_Object *p = dynamic_cast<ACE_Service_Object*>(this->tp.get()); p;) try {
                return p->resume();
            } DAF_CATCH_ALL {}
            return -1;
        }

        /** \todo{Fill this in} */
        virtual int fini(void)
        {
            for (ACE_Service_Object *p = dynamic_cast<ACE_Service_Object*>(this->tp.get()); p;) try {
                return p->fini();
            } DAF_CATCH_ALL {}
            return -1;
        }

        /** \todo{Fill this in} */
        virtual int info(ACE_TCHAR **info_string, size_t length = 0) const
        {
            for (ACE_Service_Object *p = dynamic_cast<ACE_Service_Object*>(this->tp.get()); p;) try {
                return p->info(info_string, length);
            } DAF_CATCH_ALL {}
            return -1;
        }
    };
} // namespace DAF

# define DAF_FACTORY_DEFINE(CLS,SERVICE_CLASS) \
void ACE_MAKE_SVC_CONFIG_GOBBLER_NAME(ACE_VERSIONED_NAMESPACE_NAME,SERVICE_CLASS) (void *p) { \
  ACE_VERSIONED_NAMESPACE_NAME::ACE_Service_Object * _p = \
    static_cast< ACE_VERSIONED_NAMESPACE_NAME::ACE_Service_Object *> (p); \
  ACE_ASSERT (_p != 0); \
  delete _p; } \
extern "C" CLS##_Export ACE_VERSIONED_NAMESPACE_NAME::ACE_Service_Object *\
ACE_MAKE_SVC_CONFIG_FACTORY_NAME(ACE_VERSIONED_NAMESPACE_NAME,SERVICE_CLASS) (ACE_Service_Object_Exterminator *gobbler) \
{ \
  ACE_TRACE (#SERVICE_CLASS); \
  if (gobbler != 0) \
    *gobbler = (ACE_Service_Object_Exterminator) ACE_MAKE_SVC_CONFIG_GOBBLER_NAME(ACE_VERSIONED_NAMESPACE_NAME,SERVICE_CLASS); \
  return new DAF::ServiceAdapter< SERVICE_CLASS >; \
}

#endif
