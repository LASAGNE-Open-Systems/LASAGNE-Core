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
#ifndef DAF_GCLOCATION_H
#define DAF_GCLOCATION_H

/************************ BitSet ******************************
*
* ATTRIBUTION: Aviation Formulary http://williams.best.vwh.net/avform.htm
*
* @see Aviation Formulary http://williams.best.vwh.net/avform.htm
*
* It is a well known that the shortest distance between two points
* is a straight line. However anyone attempting to fly from
* Perth to Sydney on the straight line connecting them would have
* to dig a very substantial tunnel first. The shortest distance,
* following the earth's surface lies vertically above the
* aforementioned straight line route. This route can be
* constructed by slicing the earth in half with an
* imaginary plane through Perth and Sydney. This plane cuts the
* (assumed spherical) earth in a circular arc connecting the two
* points, called a great circle. Only planes through the center of
* the earth give rise to great circles. Any plane will cut a sphere
* in a circle, but the resulting little circles are not the shortest
* distance between the points they connect. A little thought will
* show that lines of longitude (meridians) are great circles, but
* lines of latitude, with the exception of the equator, are not.
* <p>
* The first important fact to realise is that in general a great
* circle route has a true course that varies from point to point.
* For instance the great circle route between two points of equal
* (non-zero) latitude does not follow the line of latitude in an E-W
* direction, but arcs towards the pole. It is possible to fly between
* two points using an unvarying true course, but in general the
* resulting route differs from the great circle route and is called
* a rhumb line. Unlike a great circle which encircles the earth, a
* pilot flying a rhumb line would spiral indefinitely poleward.
* <p>
* Natural questions are to seek the great circle distance between
* two specified points and true course at points along the route.
* The required spherical trigonometric formulae are greatly
* simplified if angles and distances are measured in the appropriate
* natural units, which are both radians! A radian, by definition, is
* the angle subtended by a circular arc of unit length and unit radius.
* Since the length of a complete circular arc of unit radius is 2*__PI,
* the conversion is 360 degrees equals 2*__PI radians, or:
*
*    angle_radians = (__PI/180) * angle_degrees
*    angle_degrees = (180/__PI) * angle_radians
*
* Great circle distance can be likewise be expressed in radians by
* defining the distance to be the angle subtended by the arc at the
* center of the earth. Since by definition, one nautical mile subtends
* one minute (= 1/60 degree) of arc, we have:
*
*    distance_radians  = (__PI/(180*60)) * distance_nautical
*    distance_nautical = ((180*60)/__PI) * distance_radians
*
* In all subsequent formulae all distances and angles, such as latitudes,
* longitudes and true courses will be assumed to be given in radians,
* greatly simplifying them, and in applications the above formulae and
* their inverses are necessary to convert back and forth between natural
* and practical units.
*
***********************************************************************/

#include "GCRoute_export.h"

#include "daf/Conversion.h"

#include <exception>
#include <ostream>
#include <math.h>

namespace GCRoute
{
    typedef double          GCRadian;
    typedef GCRadian        GCLatitude;
    typedef GCRadian        GCLongitude;

    //  Exception for non intersecting radials.
    struct NoRadialIntersect : std::exception {};

    /************************************************************************/

    /* Convert North latitude angles to radians */
    inline double radNORTH(double d, double m = 0.0, double s = 0.0)
    {
        return DAF::deg_to_rad(d,m,s);
    }

    /* Convert South latitude angles to radians */
    inline double radSOUTH(double d, double m = 0.0, double s = 0.0)
    {
        return -(DAF::deg_to_rad(d,m,s));
    }

    /* Convert West longitude angles to radians */
    inline double radWEST(double d, double m = 0.0, double s = 0.0)
    {
        return DAF::deg_to_rad(d,m,s);
    }

    /* Convert East longitude angles to radians */
    inline double radEAST(double d, double m = 0.0, double s = 0.0)
    {
        return -(DAF::deg_to_rad(d,m,s));
    }

    inline double _cos(double x)
    {
        return DAF_OS::eps_zero(::cos(x));
    }

    inline double _sin(double x)
    {
        return DAF_OS::eps_zero(::sin(x));
    }

    inline double _tan(double x)
    {
        return DAF_OS::eps_zero(::tan(x));
    }

    /*
    * A potential implementation problem is that the arguments of asin
    * and/or acos may, because of rounding errors, exceed one in magnitude.
    * With perfect arithmetic this can't happen, however to ensure a "safe"
    * version of asin and acos we implement it with range evaluations.
    */

    inline double _asin(double x)
    {
        return DAF_OS::eps_zero(::asin(ace_range(double(-1),double(1),x)));
    }

    inline double _acos(double x)
    {
        return DAF_OS::eps_zero(::acos(ace_range(double(-1),double(1),x)));
    }

    inline double _atan(double x)
    {
        return DAF_OS::eps_zero(::atan(x));
    }

    inline double _atan2(double y, double x)
    {
        return DAF_OS::eps_zero(::atan2(y,x));
    }

    /* Ensure arc's are in the range 0 - 2*__PI (full circle) */
    inline double _arcmod(double x)
    {
        return DAF_OS::eps_zero(DAF_OS::mod(x, (2.0 * DAF_M_PI)));
    }

    /* Ensure arc's are in the range +/-__PI (full circle) */
    inline double _arcmodPI(double x)
    {
        return DAF_OS::eps_zero(_arcmod(x + DAF_M_PI) - DAF_M_PI);
    }

    /**
     * Wrapper for LOCATION that enables encapsulated calculation
     * and manipulation of various aspects of great circle navigation
     */
    // MPM - Predefinition so that the GHS compiler can pick it up
    class GCRoute_Export GCRadial;

    class GCRoute_Export GCLocation
    {
        friend class GCRadial;  // Forward 'friend' Declaration

        GCRadian    lat_, lng_;

    public:

        /** Construct a Location given lat/lng */
        GCLocation(GCLatitude lat = 0.0, GCLongitude lng = 0.0);

        /** Construct a Location from a Location (copy) */
        GCLocation(const GCLocation&); // Copy Constructor

        /** Assignment of Location value */
        GCLocation& operator = (const GCLocation&);

        /** Current location latitude (Radians) */
        GCLatitude  latitude(void) const
        {
            return GCLatitude(this->lat_);
        }

        /** Current location longitude (Radians) */
        GCLongitude longitude(void) const
        {
            return GCLongitude(this->lng_);
        }

        /** Equavalence of locations (~same place) */
        int operator == (const GCLocation&) const;

        //  Formats string in the form DDD:MM:SSc.(e.g. Sydney = 033:52:00S 151:12:00E)
        std::string toString(void) const;

        //  Formats string in the form DDD:MM.c.(e.g. Sydney = 033:52.00S 151:12.00E)
        std::string toDecimalString(void) const;

        /** Radial from current location at specified true course */
        GCRadial    radial(const GCRadian&) const;

        /** Radial from current location to specified location */
        GCRadial    radial(const GCLocation&) const;

        /** The great circle true course to specified location */
        GCRadian    radialCourse(const GCLocation&) const;

        /** The great circle distance to the specified location */
        GCRadian    radialDistance(const GCLocation&) const;

        /**
         *  Intermediate points on a great circle (lat,lon) can be found given a fraction of
         *  the distance (d) between them. Suppose the starting point is (lcn1) and the final
         *  point (lcn2) and we want the point a fraction (f) along the great circle route,
         *  where f=0 is lcn1. f=1 is lcn2.
         *  The two points cannot be antipodal (i.e. lat1+lat2=0 and __abs(lon1-lon2)=__PI)
         *  because then the route is undefined.
         **/
        GCLocation  radialFraction(const GCLocation&, double f) const;

        /** Intermediate location on the great circle given longitude. */
        GCLocation  radialPosition(const GCLocation&, GCLongitude) const;

        /** Intermediate location on the great circle given distance. */
        GCLocation  radialLocation(const GCLocation&, const GCRadian &n_miles) const;

        /** Intermediate location on the great circle given a course and distance. */
        GCLocation  radialLocation(const GCRadian &course, const GCRadian &n_miles) const;
    };

    class GCRoute_Export GCRadial : public GCLocation
    {
        GCRadian course_;

    public:

        GCRadial(const GCRadial&); // Copy Constructor
        GCRadial(const GCLocation&, const GCRadian &course);
        GCRadial(const GCLocation&, const GCLocation&);

        /** Assignment of Location value */
        GCRadial& operator = (const GCRadial&);

        GCRadian  trueCourse(void) const
        {
            return GCRadian(this->course_);
        }

        /* The highest latitude reached by the great circle */
        GCRadian  maxLatitude(void) const;

        /* Intersecting Location of great circle radials */
        GCLocation radialIntersect(const GCRadial&) const;
    };

} // namespace GCRoute

//  Formats LOCATION in the form DDD:MM:SSc DDD:MM:SSc.(e.g. Sydney = 033:52:00S 151:12:00E)
inline std::ostream&    operator << (std::ostream &os, const GCRoute::GCLocation &loc)
{
    return os << loc.toString().c_str();
}

#endif
