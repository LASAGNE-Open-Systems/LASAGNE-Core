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
#define TAF_STRINGMANAGERTEST_CPP

#include "taf/TAF.h"
#include "taf/StringManager_T.h"
#include "daf/OS.h"

#include <tao/CORBA_String.h>
#include <tao/StringSeqC.h>

#include <sstream>

//#include "tao/StringSeqC.h"

#if defined(_MSC_VER) && defined(_DEBUG)
#include <crtdbg.h>
namespace
{
    const int _crtDbgFlagSet = _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    class CRTMemDiffMonitor : public _CrtMemState
    {
    public:
        CRTMemDiffMonitor(void)
        {
            _CrtMemCheckpoint(&begin_state); ACE_UNUSED_ARG(_crtDbgFlagSet);
        }

        ~CRTMemDiffMonitor(void)
        {
            _CrtMemCheckpoint(&end_state);
            if (_CrtMemDifference(this, &begin_state, &end_state)) {
#pragma warning(suppress : 4297) // throw without declaration
                _CrtMemDumpStatistics(this); throw "Memory-Leakage-Detected";
            }
        }
    protected:
        _CrtMemState begin_state, end_state;
    };
}
# define CRT_MEMORY_DIFF_MONITOR(mon) CRTMemDiffMonitor (mon); do { ACE_UNUSED_ARG(mon); } while(0)
#else
# define CRT_MEMORY_DIFF_MONITOR(mon) do {} while(0)
#endif

typedef TAF::StringSequence_T < CORBA::StringSeq >  StringVectorType;
typedef StringVectorType::_value_type               StringManagerType;

#define DEREK_TEXT  ACE_TEXT("Derek")

namespace {

    const size_t  MAX_SEQUENCE_SIZE(10);

    char * generate_corba_string_type(void)
    {
        return CORBA::string_dup(DEREK_TEXT);
    }

    CORBA::StringSeq * generate_corba_stringseq_type(void)
    {
        CORBA::StringSeq_var seq(new CORBA::StringSeq(MAX_SEQUENCE_SIZE));

        for (CORBA::ULong i = 0; i < CORBA::ULong(MAX_SEQUENCE_SIZE); i++) {
            std::stringstream ss; std::ends(ss << DEREK_TEXT << int(i));
            seq->length(i + 1); seq[i] = ss.str().c_str();
        }

        return seq._retn();
    }

    int validate_string_type(const std::string &s)
    {
        return s == DEREK_TEXT;
    }

    int validate_string_var_type(const CORBA::String_var &s)
    {
        return DAF_OS::strcmp(s, DEREK_TEXT) == 0;
    }

    int validate_string_vector_type(const StringVectorType &s)
    {
        if (s.size() == MAX_SEQUENCE_SIZE) {
            for (size_t i = 0; i < MAX_SEQUENCE_SIZE; i++) {
                std::stringstream ss; std::ends(ss << DEREK_TEXT << int(i));
                if (DAF_OS::strcmp(ss.str().c_str(), s[i].in())) return false;
            }
            return true;
        }
        return false;
    }

    int validate_stringseq_type(const CORBA::StringSeq &s)
    {
        if (s.length() == CORBA::ULong(MAX_SEQUENCE_SIZE)) {
            for (CORBA::ULong i = 0; i < CORBA::ULong(MAX_SEQUENCE_SIZE); i++) {
                std::stringstream ss; std::ends(ss << DEREK_TEXT << int(i));
                if (DAF_OS::strcmp(ss.str().c_str(), s[i].in())) return false;
            }
            return true;
        }
        return false;
    }

    int validate_stringseq_var_type(const CORBA::StringSeq_var &s)
    {
        return validate_stringseq_type(s.in());
    }
}

int main(int argc, ACE_TCHAR *argv[])
{
    ACE_UNUSED_ARG(argc); ACE_UNUSED_ARG(argv);

    do try {

        {
            CRT_MEMORY_DIFF_MONITOR(fixed_mon);

            DAF::HighResTimer timer("StringManagerTest \t"); ACE_UNUSED_ARG(timer);

            StringManagerType  s(generate_corba_string_type());

            if (validate_string_type(s) == 0) break;
            if (validate_string_var_type(s) == 0) break;
            StringManagerType  s1(s);
            CORBA::String_var sv1(s);
            if (validate_string_var_type(sv1) == 0) break;
            CORBA::String_var sv2(s1);
            if (validate_string_var_type(sv2) == 0) break;
            sv2 = s;
            if (validate_string_var_type(sv2) == 0) break;
            if (validate_string_var_type(s1) == 0) break;


            StringVectorType    sv(generate_corba_stringseq_type());

            if (validate_string_vector_type(sv) == 0) break;

            if (validate_string_type(s1) == 0) break;

            if (validate_stringseq_var_type(sv) == 0) break;

            const char* xx = "Hello";
            sv += xx;
            s = "Hello";

        }

        ACE_ERROR_RETURN((LM_INFO, ACE_TEXT("StringManagerTest - PASSED!!\n")), 0);
    }
    catch (const char *s) {
        if (s) {
            ACE_ERROR_RETURN((LM_INFO, ACE_TEXT("StringManagerTest - FAILED with error '%s'!!\n"), s), -1);
        }
    } DAF_CATCH_ALL {
    } while (false);

    ACE_ERROR_RETURN((LM_INFO, ACE_TEXT("StringManagerTest - FAILED!!\n")), -1);
}
