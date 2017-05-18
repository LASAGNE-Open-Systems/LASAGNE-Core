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
#ifndef DAF_CONVERSION_H
#define DAF_CONVERSION_H

#include "OS.h"

#include <stdlib.h>

#if !defined(DAF_M_PI)
# if defined(M_PI)
#  define DAF_M_PI  double(M_PI)
# else
#  define DAF_M_PI  double(3.1415926535897932384626433832795)
# endif
#endif

/**
* Helper constants and functions for angle conversions
*
* @file     Conversion.h
* @author
* @author   $LastChangedBy$
* @date
* @version  $Revision$
* @ingroup  \todo{which group?}
* @namespace DAF
*/

namespace DAF
{
    /** Conversion multiplican for nautical mile(international) to kilometers */
    const double _NMS_KMS_(1.852);

    /** Conversion multiplican for nautical mile(international) to mile (mi[int]) */
    const double _NMS_MLS_(1.150779448);

    /** Conversion multiplican for metres(international) to feet */
    const double _MTRS_FT_(3.28083989501);

    /** Conversion multiplican for knots to mtrs per sec */
    const double _KNTS_MS_(0.514444444);

    /** Convert Radian angles to degrees */
    DAF_Export double       rad_to_deg(double x);
    /** Convert angles from degrees/minutes/seconds to radians */
    DAF_Export double       deg_to_rad(double d, double m = 0.0, double s = 0.0);
    /** Convert angles from Radians to BAMS */
    DAF_Export ACE_UINT32   rad_to_BAMS32(double x);
    /** Convert angles from BAMS to Radians */
    DAF_Export double       BAMS32_to_rad(ACE_UINT32 x);
    /** Convert angles from degrees to BAMS */
    DAF_Export ACE_UINT32   deg_to_BAMS32(double d, double m = 0.0, double s = 0.0);
    /** Convert angles from BAMS to degrees */
    DAF_Export double       BAMS32_to_deg(ACE_UINT32 x);

    /** Convert distances expressed in radians to nautical-miles */
    inline double rad_to_NMS(double x)
    {
        return double(((180.0 * 60.0) / DAF_M_PI) * x);
    }

    /** Convert distances expressed in radians to kilometers */
    inline double rad_to_KMS(double x)
    {
        return double(rad_to_NMS(x) * _NMS_KMS_);
    }

    /** Convert distances expressed in radians to normal (US) miles */
    inline double rad_to_MLS(double x)
    {
        return double(rad_to_NMS(x) * _NMS_MLS_);
    }

    /** Convert distances expressed in nautical-miles to radians */
    inline double NMS_to_rad(double x)
    {
        return double((x / (180.0 * 60.0)) * DAF_M_PI);
    }
}

#endif // DAFCONVERSION_H
