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

#define TAF_XMPL_TEXTPARSERSERVICE_CPP

#include "TextParserService.h"

#include "taf/ORBManager.h"

#include <ace/Get_Opt.h>

#include <map>
#include <list>
#include <algorithm>
#include <functional>

#include <fstream>

ACE_FACTORY_DEFINE(TextParserService, TAF_XMPL_TextParserService);
ACE_STATIC_SVC_DEFINE(TAF_XMPL_TextParserService
    , TAF_XMPL_TextParserService::svc_ident()
    , ACE_SVC_OBJ_T
    , &ACE_SVC_NAME(TAF_XMPL_TextParserService)
    , (ACE_Service_Type::DELETE_THIS | ACE_Service_Type::DELETE_OBJ)
    , false // Service not initially active
    );

namespace  // Ananomous namespace
{
    typedef std::map<std::string, CORBA::ULong>             WORDS_maptype;
    typedef std::list<taf_xmpl::WORDType>                   WORDS_listype;

    typedef std::binary_function < taf_xmpl::WORDType, taf_xmpl::WORDType, bool >   PREDICATE_type;

    /***********************************************************************************/

    class AlphabeticalPredicate : PREDICATE_type
    {
        bool ascending_;
    public:
        AlphabeticalPredicate(bool ascending) : ascending_(ascending) {}
        bool operator () (const first_argument_type &l, const second_argument_type &r) const {
            return DAF_OS::strcmp(r.word_, l.word_) > 0 ? this->ascending_ : !this->ascending_;
        }
    };

    class InstancePredicate : PREDICATE_type
    {
        bool ascending_;
    public:
        InstancePredicate(bool ascending) : ascending_(ascending) {}
        bool operator () (const first_argument_type &l, const second_argument_type &r) const {
            if (r.count_ > l.count_) {
                return this->ascending_;
            }
            else if (r.count_ == l.count_) {
                return AlphabeticalPredicate(true)(l, r);
            }
            return !this->ascending_;
        }
    };

    class CallbackAdapterPredicate : PREDICATE_type
    {
        const taf_xmpl::SortPredicate_ptr predicate_;
    public:
        CallbackAdapterPredicate(const taf_xmpl::SortPredicate_ptr predicate)
            : predicate_(predicate)
        {
            if (CORBA::is_nil(this->predicate_)) {
                throw CORBA::BAD_PARAM();
            }
        }
        bool operator () (const first_argument_type &l, const second_argument_type &r) const {
            try { return bool(this->predicate_->callback_op(l, r)); } DAF_CATCH_ALL {} return false;
        }
    };

    /***********************************************************************************/

    size_t  parseWORDText(const char *text, WORDS_listype &wordlist)
    {
        wordlist.clear();

        if (text ? DAF_OS::strlen(text) == 0 : true) {
            throw CORBA::BAD_PARAM();
        }

        std::string s(text);

        for (int pos = int(s.length()); pos--;) {

            const char c = s[pos];
            if (::isalpha(int(c))) {      // Only alpha characters
                if (::isupper(int(c))) {
                    s[pos] = ::tolower(int(c));  // Convert all to lower case
                }
            } else if (c == '\'') {  // remove apostrafised text.
                if (pos > 0 ? s[pos - 1] == ' ' : true) {
                    s[pos] = ' '; // Just remove if preceeded by space or first character in string
                } else {
                    int end = int(s.find_first_of(' ', pos)); if (end > 0) {
                        s.erase(pos, (end - pos));
                    } else s.erase(pos);
                }
            } else s[pos] = ' ';  // Replace Character
        }

        WORDS_maptype words;  // Map of all the unique words with their instance counts

        for (int end_x = 0, pos_x = 0; end_x != -1; ++pos_x += end_x) {
            std::string word;

            if ((end_x = int(s.find_first_of(' ', pos_x))) == -1) {
                if (word.assign(s.substr(pos_x)).length() == 0) {
                    continue;
                }
            } else if ((end_x -= pos_x) > 0) {
                word.assign(s.substr(pos_x, end_x));
            } else continue;

            WORDS_maptype::iterator it = words.find(word);
            if (it == words.end()) {
                if (words.insert(WORDS_maptype::value_type(word, 1)).second == 0) {
                    break;
                }
            } else ++it->second;
        }

        for (WORDS_maptype::iterator it = words.begin(); it != words.end(); it++) {
            taf_xmpl::WORDType t;
            t.word_ = it->first.c_str();
            t.count_ = it->second;
            wordlist.push_back(t);
        }

        return size_t(wordlist.size());
    }

    taf_xmpl::WORDTypeSeq *  seqWORDList(WORDS_listype &wordList)
    {
        for (CORBA::ULong index = 0, words = CORBA::ULong(wordList.size()); words;) {

            taf_xmpl::WORDTypeSeq_var wt_seq(new taf_xmpl::WORDTypeSeq(words));

            for (WORDS_listype::const_iterator it = wordList.begin(); it != wordList.end(); it++) {
                if (words--) { wt_seq->length(index + 1); wt_seq[index++] = *it; } else break;
            }

            return wt_seq._retn();
        }

        throw CORBA::BAD_PARAM();
    }
} // Ananomous namespace

namespace TAF_XMPL
{
    TextParserService::TextParserService(void) : TextParserInterfaceHandler()
        , use_naming_   (false)
        , debug_        (0)
    {
    }

    TextParserService::~TextParserService(void)
    {
    }

    int
    TextParserService::parse_args(int argc, char *argv[])
    {
        ACE_Get_Opt get_opts(argc, argv, ACE_TEXT("o:n::z::?"), 0);
        get_opts.long_option("iorfile", 'o', ACE_Get_Opt::ARG_REQUIRED);
        get_opts.long_option("naming",  'n', ACE_Get_Opt::ARG_OPTIONAL);
        get_opts.long_option("debug",   'z', ACE_Get_Opt::ARG_OPTIONAL);
        get_opts.long_option("help",    'h', ACE_Get_Opt::NO_ARG);  // Help

        for(;;) switch (get_opts()) {
        case -1: return 0; // Indicates sucessful parsing of the command line

        case 'n': this->use_naming_ = true; for (const ACE_TCHAR *naming = get_opts.opt_arg(); naming;) {
            this->use_naming_ = (ace_range(0, 1, DAF_OS::atoi(naming)) ? true : false); break;
        } break; // Turn on Naming optionally.

        case 'o': for (const ACE_TCHAR * iorFile = get_opts.opt_arg(); iorFile;) {
            if (DAF_OS::strlen(iorFile) > 0) { this->ior_file_.assign(iorFile); } break;
        } break; // Turn on IOR File write.

        case 'z': this->debug_ = 1;
            for (const ACE_TCHAR *debug_lvl = get_opts.opt_arg(); debug_lvl;) {
                if (::isdigit(int(*debug_lvl))) {
                    this->debug_ = ace_range(0, 10, DAF_OS::atoi(debug_lvl));
                } break;
            } break; // Turn on Debug at level optionally.

        default: if (DAF::debug()) {
        case 'h':
            ACE_DEBUG((LM_INFO,
                "usage:  %s\n"
                "-o ior filename\n"
                "-n Use Naming (0,1)\n"
                "-z Debug ON[level]\n"
                "\n", TextParserService::svc_ident()
                ));
            } break;
        }

        return 0;
    }

    /// Initializes object when dynamic linking occurs.
    int
    TextParserService::init(int argc, ACE_TCHAR *argv[])
    {
        if (this->parse_args(argc, argv) == 0) try {

            this->addIORBinding(new TAF::IORFileBinder());

            if (this->use_naming_) try {
                this->addIORBinding(new TAF::NamingServiceBinder(TheTAFBaseContext()));
            } catch (const CORBA::Exception &ex) {
                ACE_DEBUG((LM_ERROR, ACE_TEXT("(%04P|%04t) Unable to bind %s to the Naming Service\n - %s\n")
                    , TextParserService::svc_ident(), ex._rep_id())); this->use_naming_ = false;
            }

            if (this->init_bind(TextParserService::svc_ident()) == 0) {
                return 0;
            }

        } catch (const CORBA::Exception &ex) {
            ex._tao_print_exception(TextParserService::svc_ident());
        } DAF_CATCH_ALL{ /* Drop through to return error */ }

        ACE_ERROR_RETURN((LM_ERROR,
            ACE_TEXT("ERROR: %s - Unable to successfully initialize service.\n")
            , TextParserService::svc_ident()), -1);
    }

    int
    TextParserService::suspend(void)
    {
        DAF_OS::last_error(ENOTSUP); return -1;
    }

    int
    TextParserService::resume(void)
    {
        DAF_OS::last_error(ENOTSUP); return -1;
    }

    /// Terminates object when dynamic unlinking occurs.
    int
    TextParserService::fini(void)
    {
        this->fini_bind(); return 0;
    }

    /// Returns information on a service object.
    int
    TextParserService::info(ACE_TCHAR **info_string, size_t length) const
    {
        static const char *info_desc =
        {
            "The TextParserService is a simple passive service that allows the counting of words in some text."
        };

        return (info_string ? (*info_string = ACE::strnnew(info_desc, length), 0) : -1);
    }

    /********************* IDL Interface Methods *********************************/

    taf_xmpl::WORDTypeSeq *
    TextParserService::parseText(const char *text, taf_xmpl::ORDER sort_order, bool ascending)
    {
        for (WORDS_listype wordList; parseWORDText(text, wordList);) {
            switch (sort_order) {
            case taf_xmpl::ALPHABETICAL_SORT:   wordList.sort(AlphabeticalPredicate(ascending));    break;
            case taf_xmpl::INSTANCE_SORT:       wordList.sort(InstancePredicate(ascending));        break;
            default: throw CORBA::BAD_PARAM();
            }
            return seqWORDList(wordList);
        }
        throw CORBA::BAD_PARAM();
    }

    taf_xmpl::WORDTypeSeq *
    TextParserService::parseTextWithPredicate(const char *text, taf_xmpl::SortPredicate_ptr sort_predicate)
    {
        for (WORDS_listype wordList; parseWORDText(text, wordList);) {
            wordList.sort(CallbackAdapterPredicate(sort_predicate)); return seqWORDList(wordList);
        }
        throw CORBA::BAD_PARAM();
    }

}
