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
// -*- C++ -*-

#include <SQLiteConnection.h>
#include <SQLiteQuery.h>

#include <daf/DAF.h>
#include <daf/DateTime.h>

#include <iostream>
#include <string>

#define __DROP_STMT__    \
    ACE_TEXT("DROP TABLE IF EXISTS lasagne_sheet")

#define __CREATE_STMT__  \
    ACE_TEXT("CREATE TABLE IF NOT EXISTS lasagne_sheet(")   \
    ACE_TEXT("uid INTEGER PRIMARY KEY AUTOINCREMENT,")      \
    ACE_TEXT("timeofday DATETIME,")                         \
    ACE_TEXT("firstname TEXT,")                             \
    ACE_TEXT("middlename TEXT,")                            \
    ACE_TEXT("surname TEXT)")

/* Wildcard Insert */
# define __INSERT_STMT__  \
    ACE_TEXT("INSERT INTO lasagne_sheet (timeofday, firstname, middlename, surname) ") \
    ACE_TEXT("VALUES (?, 'Derek', 'W', 'Dominish')")

#define __SELECT_STMT__  \
    ACE_TEXT("SELECT * FROM lasagne_sheet")

namespace {

    size_t  print_column_names(const LDBC::SQLiteQuery &query)
    {
        int max_column = query->column_count();
        if (max_column > 0) {
            std::cout << "Column-Names:\t'";
            for (int index = 0; max_column > index; index++) {
                if (index) std::cout << ' ';
                std::cout << query->column_name(index);
            }
            std::cout << '\'' << std::endl;
        }
        return max_column;
    }
}

int main(int argc, char * argv[])
{
    ACE_UNUSED_ARG(argc); ACE_UNUSED_ARG(argv);

    DAF_Date_Time now(DAF_Date_Time::LOCALTime());

    std::cout << "\nLOCALTime:\t'" << now.toString(true) << '\'' << std::endl;

    try {
        DAF::HighResTimer timer("LDBCSQLiteTest-Time");

        LDBC::SQLiteConnection connection("simple.db", long(SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE));

        connection->execute_query(__DROP_STMT__);
        connection->execute_query(__CREATE_STMT__);

        for (int i = 0; i < 10; i++) {

            LDBC::SQLiteQuery query(connection->execute_query(__INSERT_STMT__));

            int bind_count = query->bind_count();
            if (bind_count > 0) for (int j = 1; j <= bind_count; j++) { // NOTE: Bind indexes start at 1.
                switch (j) {
                case 1:     query[j]->bind(DAF::Date_Time::LOCALTime()); break;
                default:    DAF_THROW_EXCEPTION(LDBC::InternalException); /* ERROR in this example */
                }
            }
        }

        LDBC::SQLiteQuery query(connection->execute_query(__SELECT_STMT__));

        ACE_TEST_ASSERT(print_column_names(query));

        while (query->getNext()) {

            ACE_CDR::ULong uid; ACE_Date_Time to_day; std::string f_name, m_name, s_name;

            query->getData(0, uid);
            query->getData(1, to_day);
            query->getData(2, f_name);
            query->getData(3, m_name);
            query->getData(4, s_name);

            ACE_DEBUG((LM_INFO, ACE_TEXT("Row(%03d):\t'%03d,\"%s\",\"%s\",\"%s\",\"%s\"'\n")
                , int(uid), int(uid)
                , DAF::toDateString(to_day).c_str()
                , f_name.c_str()
                , m_name.c_str()
                , s_name.c_str()
                ));
        }

    } catch (const LDBC::Exception &ex) {
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("LDBCSQLiteTest Failed - '%s'.\n"), ex.what()), -1);
    } DAF_CATCH_ALL{
        ACE_ERROR_RETURN((LM_ERROR, ACE_TEXT("LDBCSQLiteTest Failed.\n")), -1);
    }

    return 0;
}
