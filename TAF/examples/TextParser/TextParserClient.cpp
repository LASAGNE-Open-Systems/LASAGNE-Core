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

#define TAF_XPML_TEXTPARSERCLIENT_CPP

#include "daf/DAF.h"
#include "daf/ShutdownHandler.h"

#include "taf/ORBManager.h"
#include "taf/ObjectStubRef_T.h"

#include "TextParserC.cpp"
#include "TextParserS.cpp"

#include "daf/PropertyManager.h"

#include <ace/Get_Opt.h>

static bool     _shutdown       = false;
static unsigned _max_columns    = 3;
static unsigned _loop_delay     = 2; // seconds
static bool     _use_naming     = false;
static int      _debug          = 0;  // Currently not used

static std::string _TXT_FILENAME("SOA_Abstract.txt");

static std::string _ior_filename("file://TAF_XMPL_TextParserService.ior");

namespace {
    int parse_args(int argc, char *argv[])
    {
        ACE_Get_Opt get_opts(argc, argv, ACE_TEXT("c:f:w:n::z::h"), 0);
        get_opts.long_option("columns", 'c', ACE_Get_Opt::ARG_REQUIRED);
        get_opts.long_option("filename", 'f', ACE_Get_Opt::ARG_REQUIRED);
        get_opts.long_option("waitime", 'w', ACE_Get_Opt::ARG_REQUIRED);
        get_opts.long_option("naming", 'n', ACE_Get_Opt::ARG_OPTIONAL);
        get_opts.long_option("debug", 'z', ACE_Get_Opt::ARG_OPTIONAL);
        get_opts.long_option("help", 'h', ACE_Get_Opt::NO_ARG);  // Help

        for (int arg = 0;;) switch (arg = get_opts()) {
        case EOF: return 0; // Indicates successful parsing of the command line

        case 'c':
            for (const ACE_TCHAR *max_columns = get_opts.opt_arg(); max_columns;) {
                _max_columns = unsigned(ace_range(1, 10, DAF_OS::atoi(max_columns))); break;
            } break;

        case 'f':
            _TXT_FILENAME.assign(DAF::trim_string(get_opts.opt_arg())); break;

        case 'w':
            for (const ACE_TCHAR *loop_delay = get_opts.opt_arg(); loop_delay;) {
                _loop_delay = unsigned(ace_range(1, 30, DAF_OS::atoi(loop_delay))); break;
            } break;

        case 'n': _use_naming = true;
            for (const ACE_TCHAR *naming = get_opts.opt_arg(); naming;) {
                _use_naming = (ace_range(0, 1, DAF_OS::atoi(naming)) ? true : false); break;
            } break; // Turn on Naming optionally.

        case 'z': _debug = 1;
            for (const ACE_TCHAR *debug_lvl = get_opts.opt_arg(); debug_lvl;) {
                if (::isdigit(int(*debug_lvl))) {
                    _debug = ace_range(1, 10, DAF_OS::atoi(debug_lvl));
                } break;
            } break; // Turn on Debug optionally at a level

        case 'h':
            ACE_DEBUG((LM_INFO,
                "usage:  %s\n"
                "-c Columns to print output (1-10 ->Default=3)\n"
                "-w Inter-Loop delay seconds (1-30 ->Default=2)\n"
                "-f Textfile to parse\n"
                "-d Loop delay time (sec)\n"
                "-z Testing Debug ON(level)\n"
                "\n",
                argv[0])); break;
        }

        return 0;
    }

    taf_xmpl::TextParser_ptr locate_server(CORBA::ORB_ptr orb)
    {
        static size_t retry(1);

        for (taf_xmpl::TextParser_var objRef; !_shutdown; DAF_OS::sleep(2)) {
            try {
                CORBA::Object_var server_proxy;
                do {
                    if (_use_naming) try {
                        server_proxy = TheTAFBaseContext().resolve_name(taf_xmpl::TEXTPARSER_SERVICE_OID); break;
                    } DAF_CATCH_ALL{}
                    server_proxy = orb->resolve_initial_references(taf_xmpl::TEXTPARSER_SERVICE_OID);
                } while (false);

                if (CORBA::is_nil((objRef = taf_xmpl::TextParser::_narrow(server_proxy)).in())) {
                    throw CORBA::OBJECT_NOT_EXIST();
                }

                return objRef._retn();
            }
            catch (const CORBA::Exception &ex) {
                ACE_DEBUG((LM_ERROR, "CLIENT: TextParser Object reference is invalid:\n - Exception=%s; Retry(%d).\n"
                    , ex._rep_id(), retry++));
            }
        }

        return taf_xmpl::TextParser::_nil();
    }

    size_t read_textfile(const std::string &filename, char *&iov_ptr)
    {
        std::ifstream ifile(filename.c_str(), std::ios::in | std::ios::ate);

        for (size_t iov_len = 0; ifile;) try {
            if ((iov_len = static_cast<size_t>(ifile.tellg())) == 0) {
                break;
            }

            iov_ptr = new char[iov_len + 1]; // Allocate to CORBA::String_var

            ifile.seekg(0, std::ios::beg).read(iov_ptr, iov_len);

            for (int i = 0; i < int(iov_len); i++) {
                if (iov_ptr[i] == 0) iov_ptr[i] = ' '; // Ensure we have no embedded nulls
            }

            iov_ptr[iov_len] = 0; // Ensure Null terminated

            return iov_len;
        } DAF_CATCH_ALL { /* return with error */ }

        ACE_ERROR_RETURN((LM_ERROR,
            ACE_TEXT("Unable to open and read file '%s' for processing.\n")
            , filename.c_str()),-1);
    }

    class AlphabeticalPredicate_impl : public POA_taf_xmpl::SortPredicate {
        bool    ascending_;
        size_t  callbacks_;
    public:
        AlphabeticalPredicate_impl(bool ascending) : ascending_(ascending), callbacks_(0) {}
        size_t  get_callback_count(void) const { return this->callbacks_; }
    private:
        virtual CORBA::Boolean callback_op(const taf_xmpl::WORDType &l, const taf_xmpl::WORDType &r);
    };

    CORBA::Boolean
    AlphabeticalPredicate_impl::callback_op(const taf_xmpl::WORDType &l, const taf_xmpl::WORDType &r)
    {
        callbacks_++; return DAF_OS::strcmp(r.word_, l.word_) > 0 ? this->ascending_ : !this->ascending_;
    }

    typedef TAF::ObjectStubRef<AlphabeticalPredicate_impl>  AlphabeticalPredicateStub_ref;

    class WordLengthPredicate_impl : public POA_taf_xmpl::SortPredicate {
        bool    ascending_;
        size_t  callbacks_;
    public:
        WordLengthPredicate_impl(bool ascending) : ascending_(ascending), callbacks_(0) {}
        size_t  get_callback_count(void) const { return this->callbacks_; }
    private:
        virtual CORBA::Boolean callback_op(const taf_xmpl::WORDType &l, const taf_xmpl::WORDType &r);
    };

    CORBA::Boolean
    WordLengthPredicate_impl::callback_op(const taf_xmpl::WORDType &l, const taf_xmpl::WORDType &r)
    {
        callbacks_++;
        int r_len = int(r.word_ ? DAF_OS::strlen(r.word_) : 0U);
        int l_len = int(l.word_ ? DAF_OS::strlen(l.word_) : 0U);
        if (r_len == l_len) {
            if (r.count_ == l.count_) {
                return DAF_OS::strcmp(r.word_, l.word_) > 0 ? this->ascending_ : !this->ascending_;
            }
            return (r.count_ > l.count_) ? this->ascending_ : !this->ascending_;
        }
        return (r_len > l_len) ? this->ascending_ : !this->ascending_;
    }

    typedef TAF::ObjectStubRef<WordLengthPredicate_impl>    WordLengthPredicateStub_ref;

} // namespace


int main(int argc, char *argv[])
{
    struct _Shutdown : DAF::ShutdownHandler {
        virtual int handle_shutdown(int sig = 0) {
            _shutdown = true; return DAF::ShutdownHandler::handle_shutdown(sig);
        }
    } shutdown_;

    try {
        TAF::ORBManager orb(argc, argv); orb.run(3);

        const std::string svc_args(ThePropertyRepository()->get_property(TAF_SERVICE_ARGS));

        ACE_ARGV args(svc_args.c_str());

        for (int argc_ = args.argc(); ::parse_args(argc_, args.argv());) {
            ACE_ERROR_RETURN((LM_DEBUG, ACE_TEXT("(%04P|%04t) ERROR: TextParser Client is unable to parse arguments.\n")),-1);
        }

        CORBA::String_var io_buff;

        size_t iov_len = read_textfile(_TXT_FILENAME, io_buff.out());

        if (iov_len == size_t(-1) || DAF_OS::strlen(io_buff.in()) != iov_len) {
            return -1;
        }

        taf_xmpl::TextParser_var server_objRef(taf_xmpl::TextParser::_nil());

        for (int i = 0, j = 0; !_shutdown; i++) {
            if (CORBA::is_nil(server_objRef.in())) {
                server_objRef = locate_server(orb); continue;
            }

            try {
                taf_xmpl::WORDTypeSeq_var seq;

                {
                    static bool ascending = false;

                    char timerMsg[128];
                    switch (j++ % 6) {
                    case 0: ascending = false;
                    case 1:
                        {
                            DAF_OS::sprintf(timerMsg, ACE_TEXT("\n%04d - TextClient->InstanceRequest - Instance %s :")
                                , j, (ascending ? "Ascending" : "Decending"));
                            {
                                DAF::HighResTimer _(timerMsg);
                                seq = server_objRef->parseText(io_buff.in(), taf_xmpl::INSTANCE_SORT, ascending);
                            }
                        } break;

                    case 2: ascending = true;
                    case 3:
                        {
                            const AlphabeticalPredicateStub_ref predicate(new AlphabeticalPredicate_impl(ascending));

                            DAF_OS::sprintf(timerMsg, ACE_TEXT("\n%04d - TextClient->PredicateRequest - Alphabetical %s :")
                                , j, (ascending ? "Ascending" : "Decending"));
                            {
                                DAF::HighResTimer _(timerMsg);
                                seq = server_objRef->parseTextWithPredicate(io_buff.in(), predicate);
                            }
                            if (_debug) {
                                ACE_DEBUG((LM_INFO, ACE_TEXT("-->INFO: Predicate callback invocation count=%u\n")
                                    , predicate.get_impl()->get_callback_count()));
                            }
                        }
                        break;

                    case 4: ascending = true;
                    case 5:
                        {
                            const WordLengthPredicateStub_ref predicate(new WordLengthPredicate_impl(ascending));

                            DAF_OS::sprintf(timerMsg, ACE_TEXT("\n%04d - TextClient->PredicateRequest - WordLength %s :")
                                , j, (ascending ? "Ascending" : "Decending"));
                            {
                                DAF::HighResTimer _(timerMsg);
                                seq = server_objRef->parseTextWithPredicate(io_buff.in(), predicate);
                            }
                            if (_debug) {
                                ACE_DEBUG((LM_INFO, ACE_TEXT("-->INFO: Predicate callback invocation count=%u\n")
                                    , predicate.get_impl()->get_callback_count()));
                            }
                        }
                        break;
                    }
                    ascending ^= true;
                }

                {
                    size_t index    = 0, word_count = 0;
                    size_t seq_len  = size_t(seq->length());
                    size_t col_inc  = size_t((seq_len + _max_columns - 1) / _max_columns);

                    if (seq_len) for (CORBA::ULong seq_index = 0;;) {
                        std::cout   << std::setw(3) << std::left << int(seq_index + 1)
                                    << "(" << std::setw(2) << std::right << seq[seq_index].count_ << ") "
                                    << std::setw(18) << std::left << seq[seq_index].word_.in();
                        word_count += size_t(seq[seq_index].count_);
                        seq_index += CORBA::ULong(col_inc);
                        if (seq_index >= seq_len) {
                            index++;
                            if (index >= col_inc) {
                                break;
                            } else {
                                std::cout << std::endl; seq_index = CORBA::ULong(index);
                            }
                        }
                    }
                    std::cout << std::endl << std::flush;
                    ACE_DEBUG((LM_INFO, ACE_TEXT("-->INFO: Total Word Count = %d.\n"), word_count));
                }
            } catch(const CORBA::Exception &ex) {
                ex._tao_print_exception("<?> CLIENT: Server Failed - begin retry for Server IOR");
                server_objRef = taf_xmpl::TextParser::_nil();
            }

            DAF_OS::sleep(_loop_delay);
        }
    } catch (const CORBA::Exception& ex) {
        ex._tao_print_exception ("TextParser CLIENT: UNEXPECTED exception caught - "); return -1;
    } DAF_CATCH_ALL {
    }

    return 0;
}
