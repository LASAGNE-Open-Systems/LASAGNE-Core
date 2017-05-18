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
#define LDBC_SQLITECONNECTION_CPP

#include "SQLiteConnection.h"

#include "SQLiteQuery.h"

namespace {
    struct SQLiteConfig {
        SQLiteConfig(void) {
            if (::sqlite3_config(SQLITE_CONFIG_SERIALIZED)) { // Ensure Serialized (multi_threaded - multi access connections)
                ACE_DEBUG((LM_WARNING, ACE_TEXT("WARNING: SQLite (sqlite3) is unable to configure ")
                    ACE_TEXT("'SQLITE_CONFIG_SERIALIZED' for multi-threaded operations.\n")));
            }
        }
    };

    const struct SQLiteInitialize : SQLiteConfig {
        SQLiteInitialize(void)     { ::sqlite3_initialize();   }
        ~SQLiteInitialize(void)    { ::sqlite3_shutdown();     }
    } SQLiteInitialize_;

    void assert_configuration(void) {
        if (::sqlite3_threadsafe() == 0) {
            DAF_THROW_EXCEPTION(LDBC::InvalidConfiguration);
        }
    }
}

namespace LDBC
{
    namespace SQLite {

        Connection::Connection(void) : sqlite3_ref(0)
        {
            assert_configuration(); ACE_UNUSED_ARG(SQLiteInitialize_);
        }

        Connection::Connection(const std::string &name, long flags, const char *vfs) : sqlite3_ref(0)
        {
            assert_configuration();
            if (this->open(name, flags, vfs)) {
                DAF_THROW_EXCEPTION(LDBC::InitializationException);
            }
        }

        Connection::~Connection(void)
        {
            this->close();
        }

        int
        Connection::open(const std::string &name, long flags, const char *vfs)
        {
            for (int retval = ::sqlite3_open_v2(name.c_str(), &this->handle_out(), flags, vfs); retval;) {
                this->connection_name_.clear(); return retval;
            }

            //            ::sqlite3_db_config

            this->connection_name_.assign(name); return 0; // Set New Name.
        }

        int
        Connection::close(void)
        {
            if (this->size()) do {

                int i = 0; // Remove Count
                {
                    ACE_GUARD_REACTION(ACE_SYNCH_MUTEX, mon, *this, break);

                    while (this->size()) { // DCL
                        value_type &p = this->back();
                        if (p && *p) {
                            p->_finalize(p->handle_inout()); i++;
                        }
                        this->pop_back();
                    }

                }

                if (i) {
                    ACE_DEBUG((LM_WARNING,
                        ACE_TEXT("WARNING: SQLite Connection closed with %d outstanding query statements.\n"), i));
                }

            } while (false);

            this->_finalize(this->handle_inout()); return 0;
        }

        Query_ref
        Connection::execute_query(const std::string &query)
        {
            for (Query_ref query_ref(new Query(*this)); query_ref;) {

                {
                    ACE_GUARD_REACTION(ACE_SYNCH_MUTEX, mon, *this, break); this->push_back(query_ref);
                }

                if (query_ref->execute_query(DAF::trim_string(query))) {
                    DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
                }

                return query_ref._retn();
            }

            DAF_THROW_EXCEPTION(DAF::ResourceExhaustionException);
        }

        long
        Connection::last_insert_id(void) const
        {
            return static_cast<long>(::sqlite3_last_insert_rowid(*this));
        }

        int
        Connection::remove_query(Query_ptr p)
        {
            if (p) {
                ACE_GUARD_REACTION(ACE_SYNCH_MUTEX, mon, *this, DAF_THROW_EXCEPTION(DAF::ResourceExhaustionException));
                for (iterator it = this->begin(); it != this->end(); ++it) {
                    if (*it == p) {
                        this->erase(it); return 0;
                    }
                }
            }

            return -1;
        }

        void
        Connection::_finalize(Connection::_handle_inout_type p)
        {
            if (p) {
                ::sqlite3_interrupt(p); ::sqlite3_close_v2(p); p = 0;
            }
        }

        /**************************************************************/

        int assertErrorState(int state)
        {
            switch (state) {
            case SQLITE_ERROR:
            case SQLITE_MISUSE:
                DAF_THROW_EXCEPTION(LDBC::IllegalUseException);
            }
            return state;
        }

    } // namespace SQLite

} // namespace LDBC
