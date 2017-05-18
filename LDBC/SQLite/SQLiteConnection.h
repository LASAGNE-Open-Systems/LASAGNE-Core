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
#ifndef LDBC_SQLITE_CONNECTION_H
#define LDBC_SQLITE_CONNECTION_H

#include "SQLiteDefs.h"

#include <list>

namespace LDBC
{
    namespace SQLite
    {
        class SQLite_Export Query;          // Forward Declaration
        DAF_DECLARE_REFCOUNTABLE(Query);    // Forward Declarations

        SQLite_Export int assertErrorState(int state);

        class SQLite_Export Connection : public sqlite3_ref
            , protected std::list<Query_ptr>
        {
            friend class SQLite_Export SQLite::Query;

            mutable ACE_SYNCH_MUTEX lock_;

        public:

            Connection(void);
            Connection(const std::string &db, long flags, const char * vfs = 0);

            virtual ~Connection(void);

            const std::string & connection_name(void) const
            {
                return this->connection_name_;
            }

            /**
            * Connect to the specified database.
            *
            * @param[in]       db       Connection string for database
            * @param[in]       flags    Flags for the connection
            * @param[in]       vfs      File system flags
            */
            virtual int     open(const std::string &db, long flags, const char * vfs = 0);

            virtual int     close(void);

            Query_ref       execute_query(const std::string &query);

            /**
            * Get the last insert id. This method is only value if an
            * insert was made to a table with an \a auto_increment field.
            *
            * @return The last id inserted.
            */
            long    last_insert_id(void) const;

        public:

            DAF_DEFINE_REFCOUNTABLE(Connection);

        protected:

            operator ACE_SYNCH_MUTEX & () const
            {
                return this->lock_;
            }

            virtual void _finalize(_handle_inout_type);

        private:

            int remove_query(Query_ptr);

            std::string connection_name_;
        };

        DAF_DECLARE_REFCOUNTABLE(Connection);

    } // namespace SQLite

    struct SQLite_Export SQLiteConnection : SQLite::Connection_ref
    {
        SQLiteConnection(void) : SQLite::Connection_ref(new SQLite::Connection) {}
        SQLiteConnection(const std::string &db, long flags, const char * vfs = 0)
            : SQLite::Connection_ref(new SQLite::Connection(db, flags,vfs))
        {}

        operator SQLite::Connection::_handle_type () const
        {
            return (this->ptr() ? (*this)->handle() : 0);
        }
    };

} // namespace LDBC

#endif  // LDBC_SQLITE_CONNECTION_H
