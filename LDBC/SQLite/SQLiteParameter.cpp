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
#define LDBC_SQLITEPARAMETER_CPP

#include "SQLiteParameter.h"

#include "SQLiteQuery.h"

namespace LDBC
{
    namespace SQLite
    {
        Parameter::Parameter(const sqlite3_stmt_ref & stmt, int index)
            : stmt_ (stmt)
            , null_ (false)
            , index_(index)
            , type_ (PT_UNKNOWN)
        {
            if (*this && index > 0) {
                for (const char *p = ::sqlite3_bind_parameter_name(*this, index); p;) {
                    this->name_.assign(p); break;
                }
            }
        }

        Parameter::operator sqlite3_stmt_ref::_handle_in_type () const
        {
            return this->stmt_.handle_in();
        }

        Parameter &
        Parameter::bindNull(void)
        {
            if (*this) {
                if (::sqlite3_bind_null(*this, this->index())) {
                    DAF_THROW_EXCEPTION(LDBC::InternalException);
                }
                this->type_ = PT_UNKNOWN; this->null_ = true; return *this;
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        Parameter &
        Parameter::bind(const BOOL_type &v)
        {
            if (*this) {
                if (::sqlite3_bind_int(*this, this->index(), static_cast<int>(v ? true : false))) {
                    DAF_THROW_EXCEPTION(LDBC::InternalException);
                }
                this->type_ = PT_BOOL; this->null_ = false; return *this;
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        Parameter &
        Parameter::bind(const CHAR_type &v)
        {
            if (*this) {
                if (::sqlite3_bind_int(*this, this->index(), static_cast<int>(v))) {
                    DAF_THROW_EXCEPTION(LDBC::InternalException);
                }
                this->type_ = PT_CHAR;  this->null_ = false; return *this;
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        Parameter &
        Parameter::bind(const BYTE_type &v)
        {
            if (*this) {
                if (::sqlite3_bind_int(*this, this->index(), static_cast<int>(v))) {
                    DAF_THROW_EXCEPTION(LDBC::InternalException);
                }
                this->type_ = PT_BYTE; this->null_ = false; return *this;
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        Parameter &
        Parameter::bind(const SHORT_type &v)
        {
            if (*this) {
                if (::sqlite3_bind_int(*this, this->index(), static_cast<int>(v))) {
                    DAF_THROW_EXCEPTION(LDBC::InternalException);
                }
                this->type_ = PT_SHORT; this->null_ = false; return *this;
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        Parameter &
        Parameter::bind(const USHORT_type &v)
        {
            if (*this) {
                if (::sqlite3_bind_int(*this, this->index(), static_cast<int>(v))) {
                    DAF_THROW_EXCEPTION(LDBC::InternalException);
                }
                this->type_ = PT_USHORT; this->null_ = false; return *this;
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        Parameter &
        Parameter::bind(const LONG_type &v)
        {
            if (*this) {
                if (::sqlite3_bind_int(*this, this->index(), static_cast<int>(v))) {
                    DAF_THROW_EXCEPTION(LDBC::InternalException);
                }
                this->type_ = PT_LONG; this->null_ = false; return *this;
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        Parameter &
        Parameter::bind(const ULONG_type &v)
        {
            if (*this) {
                if (::sqlite3_bind_int(*this, this->index(), static_cast<int>(v))) {
                    DAF_THROW_EXCEPTION(LDBC::InternalException);
                }
                this->type_ = PT_ULONG; this->null_ = false; return *this;
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        Parameter &
        Parameter::bind(const LONGLONG_type &v)
        {
            if (*this) {
                if (::sqlite3_bind_int64(*this, this->index(), static_cast<sqlite3_int64>(v))) {
                    DAF_THROW_EXCEPTION(LDBC::InternalException);
                }
                this->type_ = PT_LONGLONG; this->null_ = false; return *this;
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        Parameter &
        Parameter::bind(const ULONGLONG_type &v)
        {
            if (*this) {
                if (::sqlite3_bind_int64(*this, this->index(), static_cast<sqlite3_uint64>(v))) {
                    DAF_THROW_EXCEPTION(LDBC::InternalException);
                }
                this->type_ = PT_ULONGLONG; this->null_ = false; return *this;
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        Parameter &
        Parameter::bind(const FLOAT_type &v)
        {
            if (*this) {
                if (::sqlite3_bind_double(*this, this->index(), static_cast<double>(v))) {
                    DAF_THROW_EXCEPTION(LDBC::InternalException);
                }
                this->type_ = PT_FLOAT; this->null_ = false; return *this;
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        Parameter &
        Parameter::bind(const DOUBLE_type &v)
        {
            if (*this) {
                if (::sqlite3_bind_double(*this, this->index(), static_cast<double>(v))) {
                    DAF_THROW_EXCEPTION(LDBC::InternalException);
                }
                this->type_ = PT_DOUBLE; this->null_ = false; return *this;
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        Parameter &
        Parameter::bind(const DATETIME_type &v)
        {
            if (*this) try {
                for (const std::string s(DAF::Date_Time(v).toString()); s.length();) {

                    if (::sqlite3_bind_text(*this, this->index(), s.c_str(), int(s.length()), SQLITE_TRANSIENT)) {
                        DAF_THROW_EXCEPTION(LDBC::InternalException);
                    }
                    this->type_ = PT_DATETIME; this->null_ = false; return *this;
                }

                return this->bindNull();

            } catch (const DAF::DateTimeException &) {
                DAF_THROW_EXCEPTION(LDBC::IllegalArgumentException);
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        Parameter &
        Parameter::bind(const STRING_type &v)
        {
            if (*this) {

                if (v.length()) {

                    if (::sqlite3_bind_text(*this, this->index(), v.c_str(), int(v.length()), SQLITE_TRANSIENT)) {
                        DAF_THROW_EXCEPTION(LDBC::InternalException);
                    }
                    this->type_ = PT_STRING; this->null_ = false; return *this;
                }

                return this->bindNull();
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        Parameter &
        Parameter::bind(const BLOB_type &v, size_t size)
        {
            if (*this) {

                for (int len = int(size); v;) {

                    if (len >= 0) {

                        if (::sqlite3_bind_blob(*this, this->index(), v, len, SQLITE_TRANSIENT)) {
                            DAF_THROW_EXCEPTION(LDBC::InternalException);
                        }
                        this->type_ = PT_BLOB; this->null_ = false; return *this;

                    } else break;
                }

                return this->bindNull();
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

    } // namespace SQLite

} // namespace LDBC
