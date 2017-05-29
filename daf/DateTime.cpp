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
#define DAF_DATETIME_CPP

/******************  DateValue  *******************************
*
*(c)Copyright 2011,
*   Defence Science and Technology Organisation,
*   Department of Defence,
*   Australia.
*
* All rights reserved.
*
* This is unpublished proprietary source code of DSTO.
* The copyright notice above does not evidence any actual or
* intended publication of such source code.
*
* The contents of this file must not be disclosed to third
* parties, copied or duplicated in any form, in whole or in
* part, without the express prior written permission of DSTO.
*
*
* @file     DateValue.cpp
* @author   Derek Dominish
* @author   $LastChangedBy$
* @date     1st September 2011
* @version  $Revision$
* @ingroup
*************************************************************/

#include "DateTime.h"

//----------------------------------------------------------------------------
// This struct is instantiated when the DLL is loaded which
// causes a call to OS::tzset()
//
// The call to OS::tzset() initialises for local timezones
// and takes account of daylight saving etc.
//----------------------------------------------------------------------------

// NOTE: this DEFAULT format is the same as used by LDBC/SQLite (GMT)
#define DAF_DATE_TIME_DEFAULT_FORMAT    ACE_TEXT("%04d-%02d-%02d %02d:%02d:%02d")       // 2015-10-08 13:15:35
#define DAF_DATE_TIME_VERBOSE_FORMAT    ACE_TEXT("%s, %d%s %s %04d %d:%02d:%02d%s")     // Thursday, 8th October 2015 1:15:35PM
#define DAF_DATE_TIME_DTGTIME_FORMAT    ACE_TEXT("%02d%02d%02dZ %s %02d")               // 081315Z OCT 15

namespace {

    const struct _DateInitTime {
        _DateInitTime(void) { DAF_OS::tzset(); }
    } _tzTime;

    const int daysInCalanderMonth[] = { 0,
        31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

    const int firstDayOfEachMonth[] = { 0,
        1, 32, 60, 91, 121, 152, 182, 213, 244, 274, 305, 335 };

    bool assertWeekDayRange(long d)
    {
        return (d == long(ace_range(DAF::SUNDAY, DAF::SATURDAY, DAF::WeekDayTy(d))));
    }

    bool assertDayRange(long d)
    {
        return (d == ace_range(1L, 31L, d));
    }

    bool assertMonthRange(long m)
    {
        return (m == long(ace_range(DAF::JANUARY, DAF::DECEMBER, DAF::MonthTy(m))));
    }

    bool assertYearRange(long y)
    {
        return (y == ace_range(1L, 3000L, y));
    }

    bool assertHourRange(long h)
    {
        return (h == ace_range(0L, 23L, h));
    }

    bool assertMinuteRange(long m)
    {
        return (m == ace_range(0L, 59L, m));
    }

    bool assertSecondRange(long s)
    {
        return (s == ace_range(0L, 59L, s));
    }

    bool assertMicrosecRange(long m)
    {
        return (m == ace_range(0L, 999999L, m));
    }

    bool assertDateRange(const ACE_Date_Time &dt)
    {
        if (DAF::Date_Time::isDayWithinMonth(dt.day(), dt.month(), dt.year()))
            if (assertHourRange(dt.hour()))
                if (assertMinuteRange(dt.minute()))
                    if (assertSecondRange(dt.second()))
                        if (assertMicrosecRange(dt.microsec()))
                            return true;

        return false;
    }

    const char * monthNAME(long m)
    {
        static const char * const _monthNAME [] = { ""
            , "January"
            , "February"
            , "March"
            , "April"
            , "May"
            , "June"
            , "July"
            , "August"
            , "September"
            , "October"
            , "November"
            , "December"
        };

        return assertMonthRange(m) ? _monthNAME[m] : "";
    }

    const char * monthDTGNAME(long m)
    {
        static const char * const _monthDTGNAME [] = { ""
            , "JAN"
            , "FEB"
            , "MAR"
            , "APR"
            , "MAY"
            , "JUN"
            , "JUL"
            , "AUG"
            , "SEP"
            , "OCT"
            , "NOV"
            , "DEC"
        };

        return assertMonthRange(m) ? _monthDTGNAME[m] : "";
    }

    const char * weekdayNAME(long d)
    {
        static const char * const _weekdayNAME[] = {
            "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
        };

        return assertWeekDayRange(d) ? _weekdayNAME[d] : "";
    }

    const char * dayACRONIUM(long d)
    {
        static const char * const _dayACRONIUM[] = {
            "th", "st", "nd", "rd", "th", "th", "th", "th", "th", "th"
        };

        return assertDayRange(d) ? _dayACRONIUM[((d < 11 || d > 13) ? (d % 10) : 0)] : "";
    };


    //= Convert a JUL clock to its corresponding Gregorian calendar
    //= date.  Algorithm 199 from Communications of the ACM, Volume 6, No. 8,
    //= (Aug. 1963), p. 444.  Gregorian calendar started on Sep. 14, 1752.
    //= This function is therefore theoretically not valid before that date.
    DAF::Date_Time  JUGDate(long jj, long hh, long mm, long ss, long us)
    {
        long j = (jj - 1721119L);
        long y, m, d;

        y = ((j * 4) - 1) / 146097;
        j = ((j * 4) - 1) - (y * 146097);
        d = (j / 4);
        j = ((d * 4) + 3) / 1461;
        d = ((d * 4) + 3) - (j * 1461);
        d = (d + 4) / 4;
        m = ((d * 5) - 3) / 153;
        d = ((d * 5) - 3) - (m * 153);
        y = ((100 * y) + j);

        d = ((d + 5) / 5);
        m = ((m < 10) ? (m + 3) : (++y, m - 9));

        return DAF::Date_Time(d, m, y, hh, mm, ss, us);
    }

    // Convert Gregorian calendar date to the corresponding Julian reference day
    // number j.  Algorithm 199 from Communications of the ACM, Volume 6, No.
    // 8, (Aug. 1963), p. 444.  Gregorian calendar started on Sep. 14, 1752.
    // This function is therefore theoretically not valid before that date.
    long JUGDay(long d, long m, long y)
    {
        long ma = ((m > 2) ? (m - 3) : (--y, m + 9));
        long ca = (y / 100);
        long ya = (y - (ca * 100));

        d += ((ca * 146097) / 4);
        d += ((ya * 1461) / 4);
        d += (((ma * 153) + 2) / 5);

        return d + 1721119L;
    }

    long WEEKDay(long d)
    {
        return long(((DAF_OS::abs(d + 1) % 7) + 7) % 7);
    }

    long WEEKDay(long d, long m, long y)
    {
        return WEEKDay(JUGDay(d, m, y));
    }
} // Annanomous Namespace

namespace DAF
{
    Date_Time::Date_Time(const ACE_Date_Time &dt)
        : ACE_Date_Time(dt)
    {
        if (assertDateRange(*this)) {
            this->weekday(WEEKDay(this->day(), this->month(), this->year())); return;
        }
        DAF_THROW_EXCEPTION(DAF::DateTimeException);
    }

    Date_Time::Date_Time(const ACE_Time_Value &tv)
        : ACE_Date_Time()
    {
        time_t tv_t = tv.sec();
        struct tm* gmt = DAF_OS::gmtime(&tv_t); // returns NULL if invalid time

        if (gmt) {

            this->day(gmt->tm_mday);            // day of the month         - [1,31]
            this->month(gmt->tm_mon + 1);       // month of the year        - [1,12]
            this->year(gmt->tm_year + 1900);    // year
            this->hour(gmt->tm_hour);           // hours since midnight     - [0,23]
            this->minute(gmt->tm_min);          // minutes after the hour   - [0,59]
            this->second(gmt->tm_sec);          // seconds after the minute - [0,59]
            this->microsec(tv.usec());

            if (assertDateRange(*this)) {
                this->weekday(WEEKDay(this->day(), this->month(), this->year())); return;
            }
        }
        DAF_THROW_EXCEPTION(DAF::DateTimeException);     // "DAF::Date invalid_time_value");
    }

    Date_Time::Date_Time(long d, long m, long y, long hh, long mm, long sec, long usec)
        : ACE_Date_Time(d,m,y,hh,mm,sec,usec)
    {
        if (assertDateRange(*this)) {
            this->weekday(WEEKDay(this->day(), this->month(), this->year())); return;
        }
        DAF_THROW_EXCEPTION(DAF::DateTimeException);     // "DAF::Date invalid_time_value");
    }

    Date_Time::Date_Time(const struct tm &s, long microsec)
        : ACE_Date_Time(  s.tm_mday         // day of the month         - [1,31]
                        , s.tm_mon  + 1     // month of the year        - [1,12]
                        , s.tm_year + 1900  // year
                        , s.tm_hour         // hours since midnight     - [0,23]
                        , s.tm_min          // minutes after the hour   - [0,59]
                        , s.tm_sec          // seconds after the minute - [0,59]
                        , microsec)
    {
        if (assertDateRange(*this)) {
            this->weekday(WEEKDay(this->day(), this->month(), this->year())); return;
        }
        DAF_THROW_EXCEPTION(DAF::DateTimeException);     // "DAF::Date invalid_time_value");
    }

    Date_Time::Date_Time(const std::string &s)
        : ACE_Date_Time()
    {
        for (int y, m, d, hh = 0, mm = 0, ss = 0, us = 0; s.length() > 0;) {
            if (::sscanf(s.c_str(), DAF_DATE_TIME_DEFAULT_FORMAT ACE_TEXT(".%d"), &y, &m, &d, &hh, &mm, &ss, &us) > 0) {
                this->year(y); this->month(m); this->day(d); this->hour(hh); this->minute(mm); this->second(ss); this->microsec(us * 1000L);
                if (assertDateRange(*this)) {
                    this->weekday(WEEKDay(this->day(), this->month(), this->year())); return;
                }
            }
            break;
        }
        DAF_THROW_EXCEPTION(DAF::DateTimeException);     // "DAF::Date invalid_time_value");
    }

    //
    // Returns a string name for the weekday number.
    // Sunday == 0, ... , Sunday == 6
    // Return 0 for weekday number out of range
    //
    const char *
    Date_Time::dayName(long weekDay)
    {
        return weekdayNAME(weekDay);
    }

    //
    // Returns a string name for the month number.
    //
    const char *
    Date_Time::monthName(long month)
    {
        return monthNAME(month);
    }

    Date_Time::operator ACE_Time_Value () const
    {
        long    jd  = JUGDay(this->day(), this->month(), this->year()) - 2440588L;
        long    ds  = 86400L  * jd;
        long    hs  = 3600L   * this->hour();
        long    ms  = 60L     * this->minute();
        return ACE_Time_Value(ds + hs + ms + this->second(), this->microsec());
    }

    long
    Date_Time::julianDay(void) const
    {
        return JUGDay(this->day(), this->month(), this->year()) - JUGDay(31, DECEMBER, this->year() - 1);
    }

    long
    Date_Time::daysInMonth(long year, long month)
    {
        if (assertMonthRange(month))  {
            if (assertYearRange(year))  {
                long dy = daysInCalanderMonth[month];
                if (month == FEBRUARY && isLeapYear(year)) {
                    dy++;
                }
                return dy;
            }
        }
        DAF_THROW_EXCEPTION(DAF::IllegalArgumentException);     // "DAF::Date invalid_time_value");
    }

    //
    // Is a day (1-31) within a given month?
    //
    bool
    Date_Time::isDayWithinMonth(long day, long month, long year)
    {
        if (assertDayRange(day)) {
            for (long dy = daysInMonth(year, month); dy > 0;) {
                return day <= dy;
            }
        }
        return false;
    }

    Date_Time&
    Date_Time::operator = (const ACE_Date_Time &dt)
    {
        if (assertDateRange(dt)) {

            this->day(dt.day());           // day of the month         - [1,31]
            this->month(dt.month());       // month of the year        - [1,12]
            this->year(dt.year());         // year
            this->hour(dt.hour());         // hours since midnight     - [0,23]
            this->minute(dt.minute());     // minutes after the hour   - [0,59]
            this->second(dt.second());     // seconds after the minute - [0,59]
            this->microsec(dt.microsec());
            this->weekday(WEEKDay(this->day(), this->month(), this->year()));

            return *this;
        }
        DAF_THROW_EXCEPTION(DAF::IllegalArgumentException);     // "DAF::Date invalid_time_value");
    }

    Date_Time&
    Date_Time::operator += (long days)
    {
        return *this = JUGDate(JUGDay(this->day(), this->month(), this->year()) + days,
                                this->hour(),
                                this->minute(),
                                this->second(),
                                this->microsec());
    }

    Date_Time&
    Date_Time::operator -= (long days)
    {
        return *this = JUGDate(JUGDay(this->day(), this->month(), this->year()) - days,
                                this->hour(),
                                this->minute(),
                                this->second(),
                                this->microsec());
    }

    int
    Date_Time::compare_to(const ACE_Date_Time &dt) const
    {
        int rtn, idx = 0;

        do switch (idx++) {
            case 0: rtn = (this->year()     < dt.year()     ? -1 : int(this->year()     > dt.year()))     ; break;
            case 1: rtn = (this->month()    < dt.month()    ? -1 : int(this->month()    > dt.month()))    ; break;
            case 2: rtn = (this->day()      < dt.day()      ? -1 : int(this->day()      > dt.day()))      ; break;
            case 3: rtn = (this->hour()     < dt.hour()     ? -1 : int(this->hour()     > dt.hour()))     ; break;
            case 4: rtn = (this->minute()   < dt.minute()   ? -1 : int(this->minute()   > dt.minute()))   ; break;
            case 5: rtn = (this->second()   < dt.second()   ? -1 : int(this->second()   > dt.second()))   ; break;
            case 6: rtn = (this->microsec() < dt.microsec() ? -1 : int(this->microsec() > dt.microsec())) ; break;
            case 7: return 0;
        } while (rtn == 0);

        return rtn;
    }

    Date_Time
    Date_Time::next(long d) const
    {
        for (long jDay = JUGDay(this->day(), this->month(), this->year()); assertWeekDayRange(d);) {
            return JUGDate(jDay + ((6 + d - WEEKDay(jDay)) % 7) + 1,
                this->hour(),
                this->minute(),
                this->second(),
                this->microsec());
        }
        DAF_THROW_EXCEPTION(DAF::IllegalArgumentException);     // "DAF::Date invalid_time_value");
    }

    Date_Time
    Date_Time::prev(long d) const
    {
        for (long jDay = JUGDay(this->day(), this->month(), this->year()); assertWeekDayRange(d);) {
            return JUGDate(jDay - ((6 + WEEKDay(jDay) - d) % 7) - 1,
                this->hour(),
                this->minute(),
                this->second(),
                this->microsec());
        }
        DAF_THROW_EXCEPTION(DAF::IllegalArgumentException);
    }

    //
    // Return the number of the first day of a given month
    // Return 0 if "month" is outside of the range 1 through 12, inclusive.
    //
    long
    Date_Time::firstDayOfMonth(long month) const
    {
        if (assertMonthRange(month)) {
            long dy = firstDayOfEachMonth[month];
            if ((month > FEBRUARY) && this->isLeapYear()) {
                dy++;
            }
            return dy;
        }
        DAF_THROW_EXCEPTION(DAF::IllegalArgumentException);     // "DAF::Date invalid_time_value");
    }

    time_t
    Date_Time::msecs(void) const
    {
        const ACE_Time_Value tm(*this); return time_t((tm.sec() * 1000U) + (tm.usec() / 1000U));
    }

    std::string toDateString(const ACE_Date_Time &dt, bool verbose)
    {
        char s[64];  // Build String

        if (assertDateRange(dt)) do {

            if (verbose) {
                if (0 >= DAF_OS::sprintf(s, DAF_DATE_TIME_VERBOSE_FORMAT,
                    weekdayNAME(dt.weekday()),
                    int(dt.day()),
                    dayACRONIUM(dt.day()),
                    monthNAME(dt.month()),
                    int(dt.year()),
                    int((dt.hour() % 12) ? (dt.hour() % 12) : 12),
                    int(dt.minute()),
                    int(dt.second()),
                    ((dt.hour() >= 12) ? "PM" : "AM"))) break;

            } // SAME AS SQLite DATETIME string
            else if (0 >= DAF_OS::sprintf(s, DAF_DATE_TIME_DEFAULT_FORMAT,
                int(dt.year()),
                int(dt.month()),
                int(dt.day()),
                int(dt.hour()),
                int(dt.minute()),
                int(dt.second()))) break;

            return std::string(s);  // Return string

        } while (false);

        DAF_THROW_EXCEPTION(DAF::IllegalArgumentException);     // "DAF::Date invalid_time_value");
    }

    std::string toDTGString(const ACE_Date_Time &dt)
    {
        char s[64];  // Build String

        if (assertDateRange(dt)) do {

            // The DTG is often used in message traffic. EXAMPLE: 091630Z JUL 11 represents 1630 GMT on 9 July 2011

            if (0 >= DAF_OS::sprintf(s, DAF_DATE_TIME_DTGTIME_FORMAT,
                int(dt.day()),
                int(dt.hour()),
                int(dt.minute()),
                monthDTGNAME(dt.month()),
                int(dt.year() % 100))) break;

            return std::string(s);  // Return string

        } while (false);

        DAF_THROW_EXCEPTION(DAF::IllegalArgumentException);     // "DAF::Date invalid_time_value");
    }
} // namespace DAF
