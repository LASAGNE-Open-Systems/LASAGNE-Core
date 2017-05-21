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
#ifndef PERF_STATSDATA_H
#define PERF_STATSDATA_H

#include "daf/DAF.h"

#include "ace/Semaphore.h"

#include <iostream>

namespace PERF
{
    class STATSData : public ACE_Semaphore
    {
    public:

        enum {
            MAX_COUNT = 100
        };

        STATSData(const std::string &ident);

        const std::string & ident(void) const
        {
            return this->ident_;
        }

        double & operator [] (int i);

        const std::string calculate_stats(void) const;

        double  calculate_mean(void) const;
        double  calculate_sd(void) const;

        friend ostream & operator << (ostream &os, const STATSData &s)
        {
            os << s.ident() << ":Count=" << s.timeCount_ << std::endl;

            for (size_t i = 0; i < s.timeCount_; i++) {
                if (i) {
                    os << ',';
                }
                os << s.timeData_[i];
            }

            return os << std::endl;
        }

    private:

        double              timeData_[MAX_COUNT]; // microseconds
        size_t              timeCount_;
        const std::string   ident_;
    };

}
#endif // PERF_STATSDATA_H
