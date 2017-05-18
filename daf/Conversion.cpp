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
#define DAF_CONVERSION_CPP

#include "Conversion.h"

namespace DAF
{
    /************ Angle Conversions *************************************************/

    const ACE_UINT32    M_BAMS32(1U << ((sizeof(ACE_UINT32) * 8) - 1));

    const double M_RADIAN_DEG(180.0 / DAF_M_PI);
    const double M_BAMS_RADIAN(M_BAMS32 / DAF_M_PI);
    const double M_BAMS_DEG(M_BAMS32 / 180.0);

    /* Convert Radian angles to degrees */
    double      rad_to_deg(double x)
    {
        return double(x * M_RADIAN_DEG);
    }

    /* Convert angles from degrees/minutes/seconds to radians */
    double      deg_to_rad(double d, double m, double s)
    {
        return double((d + (m / 60.0) + (s / 3600.0)) / M_RADIAN_DEG);
    }

    /* Convert angles from Radians to BAMS */
    ACE_UINT32  rad_to_BAMS32(double x)
    {
        return ACE_UINT32(x * M_BAMS_RADIAN);
    }

    /* Convert angles from BAMS to Radians */
    double      BAMS32_to_rad(ACE_UINT32 x)
    {
        return double(x / M_BAMS_RADIAN);
    }

    /* Convert angles from degrees to BAMS */
    ACE_UINT32  deg_to_BAMS32(double d, double m, double s)
    {
        return ACE_UINT32((d + (m / 60.0) + (s / 3600.0)) * M_BAMS_DEG);
    }

    /* Convert angles from BAMS to degrees */
    double      BAMS32_to_deg(ACE_UINT32 x)
    {
        return double(x / M_BAMS_DEG);
    }

} // namespace DAF
