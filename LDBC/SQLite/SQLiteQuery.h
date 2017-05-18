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
#ifndef LDBC_SQLITE_QUERY_H
#define LDBC_SQLITE_QUERY_H

#include "SQLiteParameter.h"

#include <vector>

namespace LDBC
{
    namespace SQLite
    {
        class SQLite_Export Query : public sqlite3_stmt_ref
            , protected PARAMETERList_type
        {
            friend class SQLite_Export SQLite::Connection;

            const SQLite::Connection_ref  connection_;

        public:

            virtual ~Query(void);

            int         query_changes(void) const;  // Get the number of rows affected by last execute.

            int         column_count(void) const;
            std::string column_name(int index) const;
            DATA_type   column_type(int index) const;

            int         bind_count(void) const;

            int         execute_query(const std::string &query);

            DAF_DEFINE_REFCOUNTABLE(Query);

            using PARAMETERList_type::const_reference;
            using PARAMETERList_type::reference;

        public: // Read Data from query

            bool        getNext(void) const;

            const Query & getData(int index, BOOL_type        &) const;
            const Query & getData(int index, CHAR_type        &) const;
            const Query & getData(int index, BYTE_type        &) const;
            const Query & getData(int index, SHORT_type       &) const;
            const Query & getData(int index, USHORT_type      &) const;
            const Query & getData(int index, LONG_type        &) const;
            const Query & getData(int index, ULONG_type       &) const;
            const Query & getData(int index, LONGLONG_type    &) const;
            const Query & getData(int index, ULONGLONG_type   &) const;
            const Query & getData(int index, FLOAT_type       &) const;
            const Query & getData(int index, DOUBLE_type      &) const;
            const Query & getData(int index, DATETIME_type    &) const;
            const Query & getData(int index, STRING_type      &) const;

            const Query & getData(int index, BLOB_type &, int &) const; // Caller does NOT own returned buffer

        public:

            const_reference operator [] (int index) const;
            reference       operator [] (int index);

            const_reference operator [] (const std::string &s) const;
            reference       operator [] (const std::string &s);

            int &  query_state(void) const
            {
                return this->query_state_;
            }

        protected:

            Query(void);
            Query(const Connection_ref &connection);

            virtual void _finalize(_handle_inout_type);

        private:

            mutable int query_state_;
        };

    }  // namespace SQLite

    struct SQLite_Export SQLiteQuery : SQLite::Query_ref
    {
        SQLiteQuery(void)                           : SQLite::Query_ref(0)      {}
        SQLiteQuery(const SQLite::Query_ref & ref)  : SQLite::Query_ref(ref)    {}
        SQLiteQuery(const SQLite::Query & ref)      : SQLite::Query_ref(ref)    {}

        SQLite::Query::const_reference  operator [] (int i) const                   { return (**this)[i]; }
        SQLite::Query::reference        operator [] (int i)                         { return (**this)[i]; }
        SQLite::Query::const_reference  operator [] (const std::string &s) const    { return (**this)[s]; }
        SQLite::Query::reference        operator [] (const std::string &s)          { return (**this)[s]; }
    };

} // namespace LDBC

#endif  // LDBC_SQLITE_QUERY_H
