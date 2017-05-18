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
#define TAF_REPOQOS_CPP

#include "RepoQOS.h"

namespace TAF
{
    std::string
    getRepoQOS(CORBA::Object_ptr obj, ACE_INT32 &time)
    {
        ACE_Auto_Array_Ptr<char> id; time = -1;
        ACE_High_Res_Timer hrt; if (obj) {
            {
                hrt.start(); id.reset(obj->_repository_id()); hrt.stop();
            }
            ACE_hrtime_t tm; hrt.elapsed_microseconds(tm); time = ACE_INT32(tm);
        }
        return id.get();
    }
}
