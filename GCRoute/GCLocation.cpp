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

#define DAF_GCLOCATION_CPP

#include "GCRoute/GCLocation.h"

namespace GCRoute
{
    /** Construct a Location from a Location (copy) */
    GCLocation::GCLocation(const GCLocation &loc)
        : lat_(loc.lat_), lng_(loc.lng_)
    {
    }

    /** Construct a Location given lat/lng */
    GCLocation::GCLocation(GCLatitude lat, GCLongitude lng)
        : lat_(lat), lng_(lng)
    {
    }

    GCLocation&  /** Assignment of Location value */
    GCLocation::operator = (const GCLocation &loc)
    {
        lat_ = loc.lat_; lng_ = loc.lng_; return *this;
    }

    GCLocation
    GCLocation::radialLocation(const GCLocation &loc, const GCRadian &distance) const
    {
        return this->radialLocation(this->radialCourse(loc), distance);
    }

    GCRadial
    GCLocation::radial(const GCLocation &loc) const
    {
        return GCRadial(*this, this->radialCourse(loc));
    }

    GCRadial
    GCLocation::radial(const GCRadian &course) const
    {
        return GCRadial(*this, course);
    }

    GCRadian
    GCLocation::radialCourse(const GCLocation &loc) const
    {
        return GCRadian(_arcmod(_atan2(_sin(longitude()-loc.longitude())*_cos(loc.latitude()),
                        _cos(latitude())*_sin(loc.latitude())-_sin(latitude())*_cos(loc.latitude())*_cos(longitude()-loc.longitude()))));
    }

    GCRadian
    GCLocation::radialDistance(const GCLocation &loc) const
    {
        return GCRadian(2*_asin(sqrt(pow(_sin((latitude()-loc.latitude())/2),2)+_cos(latitude())*_cos(loc.latitude())*pow(_sin((longitude()-loc.longitude())/2),2))));
    }

    GCLocation
    GCLocation::radialLocation(const GCRadian &course, const GCRadian &distance) const
    {
        GCLatitude  lat = _asin(_sin(latitude())*_cos(distance)+_cos(latitude())*_sin(distance)*_cos(course));
        GCLongitude lng = _arcmodPI(longitude()-(_atan2(_sin(course)*_sin(distance)*_cos(latitude()),
                                     _cos(distance)-_sin(latitude())*_sin(lat))));
        return GCLocation(lat,lng);
    }

    /**
     *  Intermediate points on a great circle (lat,lon) can be found given a fraction of
     *  the distance (d) between them. Suppose the starting point is (lcn1) and the final
     *  point (lcn2) and we want the point a fraction (f) along the great circle route,
     *  where f=0 is lcn1. f=1 is lcn2.
     *  The two points cannot be antipodal (i.e. lat1+lat2=0 and __abs(lon1-lon2)=__PI)
     *  because then the route is undefined.
     **/

    GCLocation
    GCLocation::radialFraction(const GCLocation &loc, double f) const
    {
        GCRadian    d = this->radialDistance(loc);
        double      A = _sin((1.0-f)*d)/_sin(d);
        double      B = _sin(f*d)/_sin(d);
        double      x = A*_cos(latitude())*_cos(longitude()) + B*_cos(loc.latitude())*_cos(loc.longitude());
        double      y = A*_cos(latitude())*_sin(longitude()) + B*_cos(loc.latitude())*_sin(loc.longitude());
        double      z = A*_sin(latitude())                  + B*_sin(loc.latitude());
        return GCLocation(_atan2(z,sqrt((x*x)+(y*y))),_atan2(y,x));
    }

        /** Equavalence of locations (~same place) */
    int
    GCLocation::operator == (const GCLocation &loc) const
    {
        return ((DAF_OS::abs(latitude()-loc.latitude())+DAF_OS::abs(longitude()-loc.longitude())) < DAF_M_EPS);
    }

    GCLocation
    GCLocation::radialPosition(const GCLocation &loc, GCLongitude lng) const
    {
        GCLatitude lat = _atan((_sin(latitude())*_cos(loc.latitude())*_sin(lng-loc.longitude())
                      -_sin(loc.latitude())*_cos(latitude())*_sin(lng-longitude()))/(_cos(latitude())*_cos(loc.latitude())*_sin(longitude()-loc.longitude())));
        return GCLocation(lat,lng);
    }

    std::string
    GCLocation::toString(void) const
    {
        double lat = DAF::rad_to_deg(DAF_OS::abs(latitude()));
        double lng = DAF::rad_to_deg(DAF_OS::abs(longitude()));

        int lat_d = int(lat);
        int lat_m = int(60*(lat-double(lat_d)));
        float lat_s = float(60*(60*(lat-double(lat_d))-double(lat_m)));
        int lng_d = int(lng);
        int	lng_m = int(60*(lng-double(lng_d)));
        float lng_s = float(60*(60*(lng-double(lng_d))-double(lng_m)));

        char str[64];
        DAF_OS::sprintf(str, "%03d:%02d:%02f%c %03d:%02d:%02f%c",
            lat_d,lat_m,lat_s, ((latitude() < 0.0)  ? 'S' : 'N'),
            lng_d,lng_m,lng_s, ((longitude() < 0.0) ? 'E' : 'W'));

        return std::string(str);
    }

    std::string
    GCLocation::toDecimalString(void) const
    {
        double lat = DAF::rad_to_deg(DAF_OS::abs(latitude()));
        double lng = DAF::rad_to_deg(DAF_OS::abs(longitude()));

        int lat_d = int(lat);
        float lat_m = float(60*(lat-double(lat_d)));
        int lng_d = int(lng);
        float lng_m = float(60*(lng-double(lng_d)));

        char str[64];
        DAF_OS::sprintf(str, "%d %02.04f%c %d %02.04f%c",
            lat_d,lat_m, ((latitude() < 0.0)  ? 'S' : 'N'),
            lng_d,lng_m, ((longitude() < 0.0) ? 'E' : 'W'));

        return std::string(str);
    }

    /***********************************************************************/

    GCRadial::GCRadial(const GCRadial &loc)
        : GCLocation(loc)
        , course_(loc.course_)
    {
    }

    GCRadial::GCRadial(const GCLocation &loc, const GCRadian &course)
        : GCLocation(loc)
        , course_(course)
    {
    }

    GCRadial::GCRadial(const GCLocation &lcn1, const GCLocation &lcn2)
        : GCLocation(lcn1)
    {
        this->course_ = this->radialCourse(lcn2);
    }

    GCRadial&
    GCRadial::operator = (const GCRadial &loc)
    {
        lat_ = loc.lat_; lng_ = loc.lng_; course_ = loc.course_; return *this;
    }

    GCRadian
    GCRadial::maxLatitude(void) const
    {
        return _acos(DAF_OS::abs(_sin(trueCourse())*_cos(latitude())));
    }

    GCLocation
    GCRadial::radialIntersect(const GCRadial &loc) const
    {
        if (*this == loc) {
            return GCLocation(*this);
        }
        GCRadian ang01  = _arcmodPI(trueCourse()-radialCourse(loc));
        GCRadian ang02  = _arcmodPI(loc.radialCourse(*this)-loc.trueCourse());
        if ((sin(ang01)*sin(ang02)) < sqrt(DAF_M_EPS)) {
            throw NoRadialIntersect();
        }
        ang01 = DAF_OS::abs(ang01); ang02 = DAF_OS::abs(ang02);
        GCRadian dst12  = this->radialDistance(loc);
        GCRadian ang03  = _acos(-(_cos(ang01))*_cos(ang02)+_sin(ang01)*_sin(ang02)*_cos(dst12));
        GCRadian dst    = _asin((_sin(ang02)*_sin(dst12))/_sin(ang03));
        return radialLocation(trueCourse(), dst);
    }

} // namespace GCRoute
