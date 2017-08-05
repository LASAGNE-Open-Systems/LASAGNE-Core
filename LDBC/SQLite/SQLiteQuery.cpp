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
#define LDBC_SQLITEQUERY_CPP

#include "SQLiteQuery.h"

#include <daf/DateTime.h>

#include <sstream>
#include <sqlite3.h>

namespace LDBC
{
    namespace SQLite
    {
        Query::Query(void) : sqlite3_stmt_ref(0)
        {
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        Query::Query(const Connection_ref &connection) : sqlite3_stmt_ref(0)
            , connection_(connection)
            , query_state_(SQLITE_OK)
        {
            if (connection) {
                this->resize(0); return;
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        Query::~Query(void)
        {
            this->_finalize(this->handle_inout()); this->connection_->remove_query(this);
        }

        std::string
        Query::column_name(int column) const
        {
            for (const char *result = ::sqlite3_column_name(*this, column); result;) {
                return std::string(result);
            }
            DAF_THROW_EXCEPTION(LDBC::IndexOutOfRange);
        }

        DATA_type
        Query::column_type(int column) const
        {
            switch (::sqlite3_column_type(*this, column)) {
            case 0: break; // Index out of Range
            case SQLITE_INTEGER:    return DT_INTEGER;
            case SQLITE_FLOAT:      return DT_FLOAT;
            case SQLITE_BLOB:       return DT_BLOB;
            case SQLITE_NULL:       return DT_NULL;
            case SQLITE3_TEXT:      return DT_TEXT;
            default:                return DT_UNKNOWN;
            }
            DAF_THROW_EXCEPTION(LDBC::IndexOutOfRange);
        }

        int
        Query::query_changes(void) const
        {
            return ace_max(0, ::sqlite3_changes(*this->connection_));
        }

        int
        Query::column_count(void) const
        {
            return ace_max(0, ::sqlite3_column_count(*this));
        }

        int
        Query::bind_count(void) const
        {
            return ace_max(0,::sqlite3_bind_parameter_count(*this));
        }

        int
        Query::execute_query(const std::string &query)
        {
             if (query.length() > 0) {
                int state = ::sqlite3_prepare_v2(*this->connection_, query.c_str(), EOF, &this->handle_out(), 0);
                switch (state) {
                    case SQLITE_OK: state = ::sqlite3_reset(*this);
                    {
                        int i = ::sqlite3_bind_parameter_count(*this); if (i > 0) {
                            this->resize(size_t(i)); for (int j = i; j--;) {
                                this->at(size_t(j)) = new Parameter(*this, j + 1); // Initialize to default
                            }
                            break;
                        }
                    }
                }
                return assertErrorState(this->query_state() = state);

             } else this->_finalize(this->handle_inout());

            DAF_THROW_EXCEPTION(LDBC::IllegalArgumentException);
        }

        bool
        Query::getNext(void) const
        {
            switch (assertErrorState(this->query_state() = ::sqlite3_step(*this))) {
            case SQLITE_ROW: return true;
            }
            return false;
        }

        void
        Query::_finalize(Query::_handle_inout_type p)
        {
            if (p) {
                ::sqlite3_step(*this); ::sqlite3_finalize(p); p = 0;
            }
            this->resize(0);
        }

        /*******************************************************************************************/

        const Query &
        Query::getData(int index, BOOL_type &val) const
        {
            if (*this && this->query_state() == SQLITE_ROW) {
                val = (::sqlite3_column_int(*this, index) ? true : false); return *this;
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        const Query &
        Query::getData(int index, CHAR_type &val) const
        {
            if (*this && this->query_state() == SQLITE_ROW) {
                for (const CHAR_type *s = reinterpret_cast<const CHAR_type*>(::sqlite3_column_text(*this, index)); s;) {
                    val = *s; return *this;
                }
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        const Query &
        Query::getData(int index, BYTE_type &val) const
        {
            if (*this && this->query_state() == SQLITE_ROW) {
                for (const BYTE_type *s = reinterpret_cast<const BYTE_type*>(::sqlite3_column_text(*this, index)); s;) {
                    val = *s; return *this;
                }
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        const Query &
        Query::getData(int index, SHORT_type &val) const
        {
            if (*this && this->query_state() == SQLITE_ROW) {
                val = static_cast<SHORT_type>(::sqlite3_column_int(*this, index)); return *this;
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        const Query &
        Query::getData(int index, USHORT_type &val) const
        {
            if (*this && this->query_state() == SQLITE_ROW) {
                val = static_cast<USHORT_type>(::sqlite3_column_int(*this, index)); return *this;
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        const Query &
        Query::getData(int index, LONG_type &val) const
        {
            if (*this && this->query_state() == SQLITE_ROW) {
                val = static_cast<LONG_type>(::sqlite3_column_int(*this, index)); return *this;
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        const Query &
        Query::getData(int index, ULONG_type &val) const
        {
            if (*this && this->query_state() == SQLITE_ROW) {
                val = static_cast<ULONG_type>(::sqlite3_column_int(*this, index)); return *this;
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        const Query &
        Query::getData(int index, LONGLONG_type &val) const
        {
            if (*this && this->query_state() == SQLITE_ROW) {
                val = static_cast<LONGLONG_type>(::sqlite3_column_int64(*this, index)); return *this;
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        const Query &
        Query::getData(int index, ULONGLONG_type &val) const
        {
            if (*this && this->query_state() == SQLITE_ROW) {
                val = static_cast<ULONGLONG_type>(::sqlite3_column_int64(*this, index)); return *this;
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        const Query &
        Query::getData(int index, FLOAT_type &val) const
        {
            if (*this && this->query_state() == SQLITE_ROW) {
                val = static_cast<FLOAT_type>(::sqlite3_column_double(*this, index)); return *this;
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        const Query &
        Query::getData(int index, DOUBLE_type &val) const
        {
            if (*this && this->query_state() == SQLITE_ROW) {
                val = static_cast<DOUBLE_type>(::sqlite3_column_double(*this, index)); return *this;
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        const Query &
        Query::getData(int index, DATETIME_type &val) const
        {
            if (*this && this->query_state() == SQLITE_ROW) {

                for (const CHAR_type *s = reinterpret_cast<const CHAR_type*>(::sqlite3_column_text(*this, index)); s;) try {
                    val = DAF::Date_Time(s); return *this;
                } DAF_CATCH_ALL{
                    break;  // Invalid Date
                }
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        const Query &
        Query::getData(int index, STRING_type &val) const
        {
            if (*this && this->query_state() == SQLITE_ROW) {

                const CHAR_type *s = reinterpret_cast<const CHAR_type*>(::sqlite3_column_text(*this, index));

                int len = ::sqlite3_column_bytes(*this, index);

                if (s && len > 0) { val.assign(s, len); } else val.clear();

                return *this;
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        const Query &
        Query::getData(int index, BLOB_type &buffer, int &size) const
        {
            if (*this && this->query_state() == SQLITE_ROW) {
                if ((size = ::sqlite3_column_bytes(*this, index)) >= 0) {
                    if ((buffer = ::sqlite3_column_blob(*this, index)) != 0) {
                        return *this;
                    }
                }
                size = 0; buffer = 0; return *this;
            }
            DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
        }

        /*************************************************************************/

        Query::const_reference
        Query::operator [] (int index) const
        {
            if (index > 0 && index <= int(this->size())) do {
                try {
                    for (const_reference ref(PARAMETERList_type::operator [] (size_type(index - 1))); ref;) {
                        return ref;
                    }
                } DAF_CATCH_ALL { break; }
                DAF_THROW_EXCEPTION(LDBC::InternalException); // We Have a null Parameter reference ???
            } while (false);
            DAF_THROW_EXCEPTION(LDBC::IndexOutOfRange);
        }

        Query::reference
        Query::operator [] (int index)
        {
            if (index > 0 && index <= int(this->size())) do {
                try {
                    for (reference ref(PARAMETERList_type::operator [] (size_type(index - 1))); ref;) {
                        return ref;
                    }
                } DAF_CATCH_ALL { 
                    break;
                }
                DAF_THROW_EXCEPTION(LDBC::InternalException); // We Have a null Parameter reference ???
            } while (false);
            DAF_THROW_EXCEPTION(LDBC::IndexOutOfRange);
        }

        Query::const_reference
        Query::operator [] (const std::string &s) const
        {
            return this->operator [] (::sqlite3_bind_parameter_index(*this, s.c_str()));
        }

        Query::reference
        Query::operator [] (const std::string &s)
        {
            return this->operator [] (::sqlite3_bind_parameter_index(*this, s.c_str()));
        }

    } // namespace SQLite

} // namespace LDBC
