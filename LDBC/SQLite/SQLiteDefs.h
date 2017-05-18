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
#ifndef LDBC_SQLITEDEFS_H
#define LDBC_SQLITEDEFS_H

#include "SQLite_export.h"

#include <LDBC/Exception.h>

#include <daf/DAF.h>
#include <daf/DateTime.h>
#include <daf/RefCountHandler_T.h>

#include <ace/CDR_Base.h>
#include <ace/Log_Msg.h>

#include <sqlite3.h>
#include <string>

namespace LDBC
{
    namespace SQLite
    {
        typedef struct ::sqlite3        sqlite3;        // Forward Declaration
        typedef struct ::sqlite3 *      sqlite3_ptr;

        typedef class DAF::RefCountHandler_T<sqlite3_ptr>       sqlite3_ref;

        typedef struct ::sqlite3_stmt   sqlite3_stmt;   // Forward Declaration
        typedef struct ::sqlite3_stmt * sqlite3_stmt_ptr;

        typedef class DAF::RefCountHandler_T<sqlite3_stmt_ptr>  sqlite3_stmt_ref;

        typedef ACE_CDR::Boolean    BOOL_type;
        typedef ACE_CDR::Char       CHAR_type;
        typedef ACE_CDR::Octet      BYTE_type;
        typedef ACE_CDR::Short      SHORT_type;
        typedef ACE_CDR::UShort     USHORT_type;
        typedef ACE_CDR::Long       LONG_type;
        typedef ACE_CDR::ULong      ULONG_type;
        typedef ACE_CDR::LongLong   LONGLONG_type;
        typedef ACE_CDR::ULongLong  ULONGLONG_type;
        typedef ACE_CDR::Float      FLOAT_type;
        typedef ACE_CDR::Double     DOUBLE_type;
        typedef ACE_Date_Time       DATETIME_type;
        typedef std::string         STRING_type;
        typedef const void *        BLOB_type;

        enum DATA_type {
            DT_UNKNOWN = 0,
            DT_INTEGER,
            DT_FLOAT,
            DT_BLOB,
            DT_NULL,
            DT_TEXT
        };

        enum PARAMETER_type { // The types of parameters supported.
            PT_UNKNOWN = 0,
            PT_BOOL,
            PT_CHAR,
            PT_BYTE,
            PT_SHORT,
            PT_USHORT,
            PT_LONG,
            PT_ULONG,
            PT_LONGLONG,
            PT_ULONGLONG,
            PT_FLOAT,
            PT_DOUBLE,
            PT_DATETIME,
            PT_STRING,
            PT_BLOB
        };

        enum DIRECTION_type { // The direction of the parameter.
            DIRECTION_INPUT,
            DIRECTION_OUTPUT,
            DIRECTION_INOUT
        };
    } // namespace SQLite

} // namespace LDBC

#endif // LDBC_SQLITEDEFS_H
