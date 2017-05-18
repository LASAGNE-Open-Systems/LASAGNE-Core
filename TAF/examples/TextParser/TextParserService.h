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

#ifndef TAF_XMPL_TEXTPARSERSERVICE_H
#define TAF_XMPL_TEXTPARSERSERVICE_H

#include "TextParserService_export.h"

#include "taf/CORBAInterfaceHandler_T.h"

#include <ace/Service_Object.h>
#include <ace/Service_Config.h>

#include "TextParserS.h"

namespace TAF_XMPL
{
    typedef class TAF::CORBAInterfaceHandler_T<POA_taf_xmpl::TextParser>    TextParserInterfaceHandler;

    class TextParserService_Export TextParserService : virtual public TextParserInterfaceHandler
        , public ACE_Service_Object
    {
        int  parse_args(int argc, char *argv[]);

    public:

        TextParserService(void);
        virtual ~TextParserService(void);

        static const ACE_TCHAR  *svc_ident(void)
        {
            return taf_xmpl::TEXTPARSER_SERVICE_OID;
        }

    protected: /* IDL methods */

        virtual taf_xmpl::WORDTypeSeq * parseText(const char *text, taf_xmpl::ORDER sort_order, bool ascending);
        virtual taf_xmpl::WORDTypeSeq * parseTextWithPredicate(const char *text, taf_xmpl::SortPredicate_ptr sort_predicate);

    protected:

        /// Initializes object when dynamic linking occurs.
        virtual int init(int argc, ACE_TCHAR *argv[]);

        /// Suspends object.
        virtual int suspend(void);

        /// Resume object.
        virtual int resume(void);

        /// Terminates object when dynamic unlinking occurs.
        virtual int fini(void);

        /// Returns information on a service object.
        virtual int info(ACE_TCHAR **info_string, size_t length = 0) const;

    private:

        std::string ior_file_;
        bool        use_naming_;
        int         debug_;
    };

} // namespace TAF_XMPL

typedef class TAF_XMPL::TextParserService   TAF_XMPL_TextParserService; // Typedef out namespace for service macros

ACE_FACTORY_DECLARE(TextParserService, TAF_XMPL_TextParserService);
ACE_STATIC_SVC_DECLARE_EXPORT(TextParserService, TAF_XMPL_TextParserService);

#endif // TAF_XMPL_TEXTPARSERSERVICE_H
