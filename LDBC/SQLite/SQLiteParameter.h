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
#ifndef LDBC_SQLITE_PARAMETER_H
#define LDBC_SQLITE_PARAMETER_H

#include "SQLiteConnection.h"

#include <vector>

namespace LDBC
{
    namespace SQLite
    {
        class SQLite_Export Query;          // Forward Declaration
        DAF_DECLARE_REFCOUNTABLE(Query);    // Forward Declaration

        class SQLite_Export Parameter : public DAF::RefCount
        {
            const sqlite3_stmt_ref &stmt_;

        public:

            Parameter(const sqlite3_stmt_ref &, int index);

            operator sqlite3_stmt_ref::_handle_in_type () const;

            bool                null(void) const    { return this->null_; }
            int                 index(void) const   { return this->index_;}
            PARAMETER_type      type(void) const    { return this->type_; }
            const std::string & name(void) const    { return this->name_; }

            Parameter & bindNull(void);

            Parameter & bind(const BOOL_type       &);
            Parameter & bind(const CHAR_type       &);
            Parameter & bind(const BYTE_type       &);
            Parameter & bind(const SHORT_type      &);
            Parameter & bind(const USHORT_type     &);
            Parameter & bind(const LONG_type       &);
            Parameter & bind(const ULONG_type      &);
            Parameter & bind(const LONGLONG_type   &);
            Parameter & bind(const ULONGLONG_type  &);
            Parameter & bind(const FLOAT_type      &);
            Parameter & bind(const DOUBLE_type     &);
            Parameter & bind(const DATETIME_type   &);
            Parameter & bind(const STRING_type     &);

            Parameter & bind(const BLOB_type &, size_t size);

            DAF_DEFINE_REFCOUNTABLE(Parameter);

        private:

            bool            null_; // NULL state of the parameter.
            const int       index_;
            PARAMETER_type  type_;
            std::string     name_;
        };

        DAF_DECLARE_REFCOUNTABLE(Parameter);

        typedef class std::vector<Parameter_ref>   PARAMETERList_type;

    } // namespace SQLite

    typedef SQLite::Parameter_ref   SQLiteParameter;

} // namespace LDBC

#endif  // !defined LDBC_SQLITE_PARAMETER_H
