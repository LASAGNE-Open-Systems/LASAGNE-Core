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
#define PERF_STATSDATA_CPP

#include "STATSData.h"

#include <math.h>

namespace PERF
{
    STATSData::STATSData(const std::string &ident)
        : ACE_Semaphore(0), timeCount_(0), ident_(ident)
    {
        DAF_OS::memset(&this->timeData_, 0, sizeof(this->timeData_));
    }

    double &
    STATSData::operator [] (int i)
    {
        if (ace_range(0, MAX_COUNT - 1, i) == i) {
            this->timeCount_ = ace_max(this->timeCount_, size_t(i + 1)); return this->timeData_[i];
        }
        throw "Index-Out-Of-Bounds";
    }

    const std::string
    STATSData::calculate_stats(void) const
    {
        char s[128]; DAF_OS::sprintf(s, "count=%u,mean=%4.4f,sd=%4.4f",
            unsigned(this->timeCount_),
            this->calculate_mean(),
            this->calculate_sd());
        return std::string(s);
    }

    double
    STATSData::calculate_mean(void) const
    {
        double mean_val = 0.0;
        for (size_t i = 0; i < this->timeCount_; i++) {
            mean_val += this->timeData_[i];
        }
        return mean_val / this->timeCount_;
    }

    double
    STATSData::calculate_sd(void) const
    {
        double sd_val = 0.0, mean_val = this->calculate_mean();
        for (size_t i = 0; i < this->timeCount_; i++) {
            sd_val += pow(mean_val - this->timeData_[i], 2);
        }
        return sqrt(sd_val / this->timeCount_);
    }

} // namespace PERF
