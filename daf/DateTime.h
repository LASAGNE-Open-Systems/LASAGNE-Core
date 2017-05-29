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
#ifndef DAF_DATETIME_H
#define DAF_DATETIME_H

/**
 * ATTRIBUTION: Based On OpenSource 'Date' - Runtime Borland C++ Builder 4
 *
 * @file     DateTime.h
 * @author   Derek Dominish
 * @author   $LastChangedBy$
 * @date     1st September 2011
 * @version  $Revision$
 * @ingroup
 * @namespace DAF
 */

#include "DAF.h"

#include <ace/Date_Time.h>

namespace DAF
{
    DAF_Export std::string toDateString(const ACE_Date_Time &dt, bool verbose = false);
    DAF_Export std::string toDTGString(const ACE_Date_Time &dt);

    enum MonthTy  {
        JANUARY   = 1,
        FEBRUARY,
        MARCH,
        APRIL,
        MAY,
        JUNE,
        JULY,
        AUGUST,
        SEPTEMBER,
        OCTOBER,
        NOVEMBER,
        DECEMBER
    };

    enum WeekDayTy { // from struct tm!
        SUNDAY    = 0,
        MONDAY,
        TUESDAY,
        WEDNESDAY,
        THURSDAY,
        FRIDAY,
        SATURDAY,
    };

    /** @class Date_Time
     * @brief Brief \todo { fill this in }
     *
     * Detailed description \todo { fill this in }
     */
    class DAF_Export Date_Time : public ACE_Date_Time
    {
        using ACE_Date_Time::update;  // Dissallow this on the interface

        public:

            /** \todo{Fill this in} */
            static DAF::Date_Time  GMTime    (const timeval &tv = DAF_OS::gettimeofday())
            {
                time_t tv_t = time_t(tv.tv_sec); return DAF::Date_Time(*DAF_OS::gmtime(&tv_t), tv.tv_usec);
            }

            /** \todo{Fill this in} */
            static DAF::Date_Time  LOCALTime (const timeval &tv = DAF_OS::gettimeofday())
            {
                time_t tv_t = time_t(tv.tv_sec); return DAF::Date_Time(*DAF_OS::localtime(&tv_t), tv.tv_usec);
            }

            /** \todo{Fill this in} */
            Date_Time(const ACE_Date_Time &);
            /** \todo{Fill this in} */
            Date_Time(const ACE_Time_Value &);
            /** \todo{Fill this in} */
            Date_Time(const struct tm &, long microsec = 0);
            /** \todo{Fill this in} */
            Date_Time( long day,
                       long month,
                       long year,
                       long hour      = 0,
                       long minute    = 0,
                       long second    = 0,
                       long microsec  = 0);

            // Parse string using DAF_DATE_TIME_DEFAULT_FORMAT to build date_time
            /** \todo{Fill this in} */
            Date_Time(const std::string &);

            /** \todo{Fill this in} */
            operator ACE_Time_Value () const;

            /** \todo{Fill this in} */
            DAF::Date_Time    next(long weekday) const;
            /** \todo{Fill this in} */
            DAF::Date_Time    prev(long weekday) const;

            /** \todo{Fill this in} */
            std::string toString(bool verbose = false) const
            {
                return DAF::toDateString(*this, verbose);
            }

            /** \todo{Fill this in} */
            std::string toDTGString(void) const
            {
                return DAF::toDTGString(*this);
            }

            /** \todo{Fill this in} */
            long    julianDay() const;                  // Day in Year (Julian Date)
            /** \todo{Fill this in} */
            long    firstDayOfMonth(long month) const;  // Day in year for 1st of month

            /** \todo{Fill this in} */
            bool    isLeapYear(void) const
            {
                return isLeapYear(this->year());
            }

            /** \todo{Fill this in} */
            time_t  msecs(void) const;

            /** \todo{Fill this in} */
            static time_t  UTCTime(const timeval &tv = DAF_OS::gettimeofday())
            {
                return DAF::Date_Time::GMTime(tv).msecs();
            }

            /** \todo{Fill this in} */
            int compare_to  (const ACE_Date_Time&) const;
            /** \todo{Fill this in} */
            int operator <  (const ACE_Date_Time &dt) const    { return this->compare_to(dt) <  0; }
            /** \todo{Fill this in} */
            int operator <= (const ACE_Date_Time &dt) const    { return this->compare_to(dt) <= 0; }
            /** \todo{Fill this in} */
            int operator >  (const ACE_Date_Time &dt) const    { return this->compare_to(dt) >  0; }
            /** \todo{Fill this in} */
            int operator >= (const ACE_Date_Time &dt) const    { return this->compare_to(dt) >= 0; }
            /** \todo{Fill this in} */
            int operator == (const ACE_Date_Time &dt) const    { return this->compare_to(dt) == 0; }
            /** \todo{Fill this in} */
            int operator != (const ACE_Date_Time &dt) const    { return this->compare_to(dt) != 0; }

            /** \todo{Fill this in} */
            DAF::Date_Time &  operator =  (const ACE_Date_Time &dt);
            /** \todo{Fill this in} */
            DAF::Date_Time &  operator += (long days);
            /** \todo{Fill this in} */
            DAF::Date_Time &  operator -= (long days);

            /** \todo{Fill this in} */
            static long daysInMonth(long year, long month);

            /** \todo{Fill this in} */
            static bool isLeapYear(long year)
            {
                return ((year % 4) ? false : ((year % 400) ? (year % 100) != 0 : true));
            }

            /** \todo{Fill this in} */
            static long daysInYear(long year)
            {
                return isLeapYear(year) ? 366 : 365;
            }

            /** \todo{Fill this in} */
            static bool         isDayWithinMonth(long day, long month, long year );
            /** \todo{Fill this in} */
            static const char * dayName  (long weekday);
            /** \todo{Fill this in} */
            static const char * monthName(long month);
            /** \todo{Fill this in} */
            static long         julianDay(long day, long month, long year);
    };

    inline  DAF::Date_Time     operator +  (const ACE_Date_Time &dt, long days)
    {
        return Date_Time(dt) += days;
    }

    inline  DAF::Date_Time     operator -  (const ACE_Date_Time &dt, long days)
    {
        return Date_Time(dt) -= days;
    }

    inline  ACE_Time_Value      operator -  (const DAF::Date_Time &lhs, const DAF::Date_Time &rhs)
    {
        return ACE_Time_Value(ACE_Time_Value(lhs) - ACE_Time_Value(rhs));
    }

} // namespace DAF

inline std::ostream&  operator << (std::ostream &os, const ACE_Date_Time &dt)
{
    return os << DAF::toDateString(dt, true);
}

typedef class DAF::Date_Time   DAF_Date_Time;

#endif   // DAF_DATETIME_H
