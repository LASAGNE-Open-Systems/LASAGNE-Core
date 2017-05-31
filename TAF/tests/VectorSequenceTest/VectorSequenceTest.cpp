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
#define VECTORSEQUENCETEST_CPP

#include "taf/TAF.h"
#include "taf/VectorSequence_T.h"
#include "VectorSequenceTestC.h"

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

typedef TAF::VectorSequence_T<test::VSTestFixedTypeSeq>       VSTestFixedTypeVector;
typedef TAF::VectorSequence_T<test::VSTestVariableTypeSeq>    VSTestVariableTypeVector;

namespace {

    const size_t  MAX_SEQUENCE_SIZE(10);

    test::VSTestFixedTypeSeq *  generate_corba_fixed_type(void)
    {
        test::VSTestFixedTypeSeq_var t(new test::VSTestFixedTypeSeq(CORBA::ULong(MAX_SEQUENCE_SIZE)));

        for (CORBA::ULong i = 0; i < t->maximum(); i++) {
            t->length(i + 1); t[i].v1 = CORBA::UShort(i); t[i].v2 = CORBA::UShort(i * 100);
        }

        return t._retn();
    }

    test::VSTestVariableTypeSeq *  generate_corba_variable_type(void)
    {
        test::VSTestVariableTypeSeq_var t(new test::VSTestVariableTypeSeq(CORBA::ULong(MAX_SEQUENCE_SIZE)));

        for (CORBA::ULong i = 0; i < t->maximum(); i++) {
            t->length(i + 1); char s[16];
            {
                DAF_OS::sprintf(s, ACE_TEXT("%04d"), int(i));
                t[i].s1 = std::string(s).c_str();
            }
            {
                DAF_OS::sprintf(s, ACE_TEXT("%04d"), int(i * 100));
                t[i].s2 = std::string(s).c_str();
            }
        }

        return t._retn();
    }

    int validate_fixed_vector_type(const VSTestFixedTypeVector &v)
    {
        if (v.size() == MAX_SEQUENCE_SIZE) {
            for (size_t i = 0; i < MAX_SEQUENCE_SIZE; i++) {
                if (v[i].v1 != CORBA::UShort(i)) {
                    return false;
                }

                if (v[i].v2 != CORBA::UShort(i * 100)) {
                    return false;
                }
            }

            return true;
        }

        return false;
    }

    int validate_variable_vector_type(const VSTestVariableTypeVector &v)
    {
        if (v.size() == MAX_SEQUENCE_SIZE) {
            for (size_t i = 0; i < MAX_SEQUENCE_SIZE; i++) {
                char s[16];
                {
                    DAF_OS::sprintf(s, ACE_TEXT("%04d"), int(i));
                    if (std::string(s) != v[i].s1.in()) {
                        return false;
                    }
                }
                {
                    DAF_OS::sprintf(s, ACE_TEXT("%04d"), int(i * 100));
                    if (std::string(s) != v[i].s2.in()) {
                        return false;
                    }
                }
            }

            return true;
        }

        return false;
    }

    int validate_fixed_sequence_type(const test::VSTestFixedTypeSeq &t)
    {
        if (t.length() == CORBA::ULong(MAX_SEQUENCE_SIZE)) {
            for (CORBA::ULong i = 0; i < t.length(); i++) {
                if (t[i].v1 != CORBA::UShort(i)) {
                    return false;
                }

                if (t[i].v2 != CORBA::UShort(i * 100)) {
                    return false;
                }
            }

            return true;
        }

        return false;
    }

    int validate_fixed_sequence_type(const test::VSTestFixedTypeSeq_var &t)
    {
        return validate_fixed_sequence_type(t.in());
    }

    int validate_variable_sequence_type(const test::VSTestVariableTypeSeq &t)
    {
        if (t.length() == CORBA::ULong(MAX_SEQUENCE_SIZE)) {
            for (CORBA::ULong i = 0; i < t.length(); i++) {
                char s[16];
                {
                    DAF_OS::sprintf(s, ACE_TEXT("%04d"), int(i));
                    if (std::string(s) != t[i].s1.in()) {
                        return false;
                    }
                }
                {
                    DAF_OS::sprintf(s, ACE_TEXT("%04d"), int(i * 100));
                    if (std::string(s) != t[i].s2.in()) {
                        return false;
                    }
                }
            }

            return true;
        }

        return false;
    }

    int validate_variable_sequence_type(const test::VSTestVariableTypeSeq_var &t)
    {
        return validate_variable_sequence_type(t.in());
    }

}

int main(int argc, ACE_TCHAR *argv[])
{
    ACE_UNUSED_ARG(argc); ACE_UNUSED_ARG(argv);

    do try {

        {
            CRT_MEMORY_DIFF_MONITOR(fixed_mon);

            DAF::HighResTimer timer("VSTestFixedTypeVector    \t"); ACE_UNUSED_ARG(timer);

            VSTestFixedTypeVector::_seq_type F_val;
            {
                test::VSTestFixedTypeSeq_var     F_ref;
                {
                    VSTestFixedTypeVector                   F_empty;
                    F_val = F_empty.in(); if (F_val.length()) break;

                    F_val.length(1); F_val[0].v1 = CORBA::ULong(99); F_val[0].v2 = CORBA::ULong(99 * 100);
                    VSTestFixedTypeVector                   F_one(F_val);
                    if (F_one.size() == 0) break;

                    VSTestFixedTypeVector       F_vector(generate_corba_fixed_type());

                    if (validate_fixed_vector_type(F_vector) == 0)  break;
                    if (validate_fixed_sequence_type(F_val = F_ref = F_vector) == 0)   break;
                    VSTestFixedTypeVector           F_vector1(F_ref);
                    if (validate_fixed_vector_type(F_vector1) == 0) break;
                    VSTestFixedTypeVector           F_vector2(F_ref);
                    if (validate_fixed_vector_type(F_vector2) == 0) break;
                    test::VSTestFixedTypeSeq_var    F_ref1, F_ref2;
                    if (validate_fixed_sequence_type(F_ref1 = F_vector1) == 0)  break;
                    if (validate_fixed_sequence_type(F_ref2 = F_vector2) == 0)  break;
                    if (validate_fixed_sequence_type(F_val = F_vector1.in()) == 0)  break;
                    if (validate_fixed_sequence_type(F_vector2.in()) == 0)  break;
                    if (validate_fixed_sequence_type(F_vector1._retn()) == 0)  break;
                    if (validate_fixed_sequence_type(F_vector2._retn()) == 0)  break;

                    if (validate_fixed_sequence_type(F_ref = F_vector2) == 0)   break;

                    VSTestFixedTypeVector F(F_vector1 + F_vector2); ACE_UNUSED_ARG(F);

                    if (validate_fixed_vector_type(F_one = F_val) == 0) break;

                    F_one += F_val;
                }
                if (validate_fixed_sequence_type(F_val = F_ref) == 0)   break;

                F_ref = 0;
            }
            if (validate_fixed_sequence_type(F_val) == 0)   break;
        }

        {
            CRT_MEMORY_DIFF_MONITOR(variable_mon);

            DAF::HighResTimer timer("VSTestVariableTypeVector \t"); ACE_UNUSED_ARG(timer);

            VSTestVariableTypeVector::_seq_type V_val;
            {
                test::VSTestVariableTypeSeq_var     V_ref;
                {
                    VSTestVariableTypeVector                   V_empty;
                    V_val = V_empty.in(); if (V_val.length()) break;

                    V_val.length(1); V_val[0].s1 = "Hello"; V_val[0].s2 = "ByeBye";
                    VSTestVariableTypeVector                   V_one(V_val);
                    if (V_one.size() == 0) break;

                    VSTestVariableTypeVector    V_vector(generate_corba_variable_type());

                    if (validate_variable_vector_type(V_vector) == 0)   break;
                    if (validate_variable_sequence_type(V_val = V_ref = V_vector) == 0)    break;
                    VSTestVariableTypeVector    V_vector1(V_ref);
                    if (validate_variable_vector_type(V_vector1) == 0)  break;
                    VSTestVariableTypeVector    V_vector2(V_ref);
                    if (validate_variable_vector_type(V_vector2) == 0)  break;
                    test::VSTestVariableTypeSeq_var    V_ref1, V_ref2;
                    if (validate_variable_sequence_type(V_ref1 = V_vector1) == 0)   break;
                    if (validate_variable_sequence_type(V_ref2 = V_vector2) == 0)   break;
                    if (validate_variable_sequence_type(V_val = V_vector1.in()) == 0)   break;
                    if (validate_variable_sequence_type(V_vector2.in()) == 0)   break;
                    if (validate_variable_sequence_type(V_vector1._retn()) == 0)   break;
                    if (validate_variable_sequence_type(V_vector2._retn()) == 0)   break;

                    if (validate_variable_sequence_type(V_ref = V_vector2) == 0)  break;

                    VSTestVariableTypeVector V(V_vector1 + V_vector2); ACE_UNUSED_ARG(V);

                    if (validate_variable_vector_type(V_one = V_val) == 0) break;
                }
                if (validate_variable_sequence_type(V_val = V_ref) == 0) break;

                V_ref = 0;
            }
            if (validate_variable_sequence_type(V_val) == 0) break;
        }

        ACE_ERROR_RETURN((LM_INFO, ACE_TEXT("VectorSequenceTest - PASSED!!\n")), 0);

    } catch (const char *s) {
        if (s) {
            ACE_ERROR_RETURN((LM_INFO, ACE_TEXT("VectorSequenceTest - FAILED with error '%s'!!\n"), s), -1);
        }
    } DAF_CATCH_ALL {
    } while (false);

    ACE_ERROR_RETURN((LM_INFO, ACE_TEXT("VectorSequenceTest - FAILED!!\n")), -1);
}
