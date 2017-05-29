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
#define TAF_MULTICAST_DISCOVERY_UTILITY_CPP

#include <taf/TAF.h>
#include <taf/ORBManager.h>
#include <taf/RepoQOS.h>
#include <taf/NamingContext.h>
#include <taf/TAFServerC.h>
#include <taf/PropertyServerC.h>
#include <taf/VectorSequence_T.h>

#include <daf/ShutdownHandler.h>
#include <daf/PropertyManager.h>
#include <daf/DateTime.h>
#include <daf/DAFDebug.h>

#include <tao/Stub.h>
#include <tao/Profile.h>

#include <tao/AnyTypeCode/Objref_TypeCode_Static.h>

#include <ace/Arg_Shifter.h>
#include <ace/Get_Opt.h>

#include <sstream>

#if defined(TAF_HAS_DISCOVERY)
# include <taf/extensions/discovery/DiscoveryHandler.h>
#else
# error "TAFDiscovery not supported by Extensions configuration"
#endif

typedef TAF::VectorSequence_T<taf::IORReplySeq>         IORReplyVector;
typedef TAF::VectorSequence_T<taf::EntityDescriptorSeq> EntityDescriptorVector;

namespace {

    int     debug_(0), verbose_(0);
    bool    ior_tags_(false), properties_(false), ior_profiles_(false);
    ACE_Time_Value    DISCOVER_TIMOUT(10);

    int debug(void)     { return debug_; }
    int verbose(void) { return verbose_; }

    int parse_args(int argc, ACE_TCHAR *argv[])
    {
        ACE_Get_Opt get_opts(argc, argv, ACE_TEXT("l:v::ptiz::?"));
        get_opts.long_option("looptime", 'l', ACE_Get_Opt::ARG_REQUIRED);
        get_opts.long_option("verbose", 'v', ACE_Get_Opt::ARG_OPTIONAL);
        get_opts.long_option("properties", 'p', ACE_Get_Opt::NO_ARG);
        get_opts.long_option("tags", 't', ACE_Get_Opt::NO_ARG);
        get_opts.long_option("debug", 'z', ACE_Get_Opt::ARG_OPTIONAL);
        get_opts.long_option("help", '?', ACE_Get_Opt::NO_ARG);  // Help

        for (;;) switch (get_opts()) {
        case EOF: return 0; // Indicates sucessful parsing of the command line

            // SHOW IORTags
        case 't':   ior_tags_ = true; break;
            // SHOW Properties
        case 'p':   properties_ = true; break;
            // SHOW IOR Profiles
        case 'i':   ior_profiles_ = true; break;

            // LOOPTIME
        case 'l':
            for (const ACE_TCHAR *looptime_val = get_opts.opt_arg(); looptime_val;) {
                if (::isdigit(int(*looptime_val))) {
                    DISCOVER_TIMOUT.set(ace_range(2, 120, DAF_OS::atoi(looptime_val)), 0);
                }
                break;
            } break;

            // VERBOSE level
        case 'v':   verbose_ = 1;
            for (const ACE_TCHAR *verbose_val = get_opts.opt_arg(); verbose_val;) {
                if (::isdigit(int(*verbose_val))) {
                    verbose_ = ace_range(1, 10, DAF_OS::atoi(verbose_val));
                }
                break;
            } break;

        case 'z':   debug_ = 1;
            for (const ACE_TCHAR *debug_val = get_opts.opt_arg(); debug_val;) {
                if (::isdigit(int(*debug_val))) {
                    debug_ = ace_range(1, 10, DAF_OS::atoi(debug_val));
                }
                break;
            } break;

        case '?':
            ACE_DEBUG((LM_INFO,
                "usage:  %s\n"
                "-l LoopTime [sec]\n"
                "-p Show Properties\n"
                "-t Show IOR Tags\n"
                "-i Show IOR Profiles\n"
                "-v Verbose ON[level]\n"
                "-z Debug ON[level]\n"
                "\n", argv[0]
                )); break;
        }

        return 0;
    }

    std::string svc_flags_to_string(unsigned flags)
    {
        std::string svc_flags;

        if (flags & (1U << taf::SVC_EXECUTE)) {
            svc_flags.append("Executable ");
        }

        if (flags & (1U << taf::SVC_STATIC)) {
            svc_flags.append("Static");
        }
        else if (flags & (1U << taf::SVC_DYNAMIC)) {
            svc_flags.append("Dynamic");
        }

        if (flags & (1U << taf::SVC_MODULE)) {
            svc_flags.append(" Module");
        }
        else if (flags & (1U << taf::SVC_STREAM)) {
            svc_flags.append(" Stream");
        }
        else if (flags & (1U << taf::SVC_OBJECT)) {
            svc_flags.append(" Service");
        }

        if (flags & (1U << taf::SVC_FINIED)) {
            svc_flags.append(" - finished");
        }
        else if (flags & (1U << taf::SVC_ACTIVE)) {
            svc_flags.append(" - active");
        }
        else svc_flags.append(" - suspended");

        return svc_flags;
    }

    class TAFServerFormatter : public std::stringstream
    {
    public:

        TAFServerFormatter(const taf::TAFServer_var &taf_server, size_t index);

        friend std::ostream & operator << (std::ostream &os, const TAFServerFormatter &formatter)
        {
            return os << formatter.str();
        }

    private:

        void insert_descriptor(const taf::EntityDescriptor &, size_t index);
        void insert_properties(const taf::PropertyValueSequence_var &properties);
        void insert_profile(const CORBA::Object_var &obj);
    };

    void
    TAFServerFormatter::insert_profile(const CORBA::Object_var &obj)
    {
        if (obj) {

            ACE_INT32 tm_usec; const std::string id = TAF::getRepoQOS(obj, tm_usec);
            *this << '\t' << std::setw(8) << "ID:" << id << " [" << tm_usec << "usec]" << std::endl;

            TAO_Stub *stub = obj->_stubobj();

            if (ior_profiles_ && stub) {

                TAO_MProfile &mp = stub->base_profiles();

                for (CORBA::ULong i = 0; i < mp.profile_count(); i++) {
                    TAO_Profile *profile(mp.get_profile(i));
                    if (profile) {
                        char iors[16]; DAF_OS::sprintf(iors, "IOR[%d]:", int(i));
                        *this << '\t' << std::setw(8) << iors << CORBA::String_var(profile->to_string()).in() << std::endl;

                        const IOP::MultipleComponentProfile &mcp(profile->tagged_components().components());

                        if (ior_tags_ || verbose() > 2) for (CORBA::ULong j = 0; j < mcp.length(); j++) {
                            const IOP::TaggedComponent &tc(mcp[j]);
                            char tags[16]; DAF_OS::sprintf(tags, "-Tag[%03x]:", int(tc.tag));
                            *this << '\t' << std::setw(10) << tags;
                            const size_t tc_data_len = size_t(tc.component_data.length()); if (tc_data_len) {
                                *this << DAF::hex_dump_data(tc.component_data.get_buffer(), tc_data_len, (tc_data_len % 32));
                            }
                            *this << std::endl;
                        }
                    }
                }
            }
        }
    }

    void
    TAFServerFormatter::insert_descriptor(const taf::EntityDescriptor &ed, size_t index)
    {
        const ACE_Time_Value start_time(time_t(ed.loadTime_.sec), suseconds_t(ed.loadTime_.usec));
        const ACE_Time_Value run_time(ACE_Time_Value(DAF_Date_Time::GMTime()) - start_time);

        unsigned run_min = unsigned(run_time.sec() / 60U), run_hrs = unsigned(run_min / 60U);

        *this << std::fixed << std::left;

        const char * ident_format((ed.flags_ & (1U << taf::SVC_EXECUTE)) ? ACE_TEXT("[%02d] %s\t[%s]\n") : ACE_TEXT("-->[%02d] %s\t[%s]\n"));
        {
            char sIdent[256]; DAF_OS::sprintf(sIdent, ident_format
                , int(index), ed.ident_.in(), svc_flags_to_string(unsigned(ed.flags_)).c_str());
            *this << sIdent;
        }

        *this << '\t' << std::setw(8) << "LOC:" << '[' << ed.libpathname_.in() << " : " << ed.objectclass_.in() << ']' << std::endl;

        if (ed.parameters_) {
            *this << '\t' << std::setw(8) << "ARGS:" << '\"' << ed.parameters_.in() << '\"' << std::endl;
        }

        {
            char sTime[64]; DAF_OS::sprintf(sTime, " - [elapsed %d:%02d Hrs]", run_hrs, unsigned(run_min % 60U));
            *this << '\t' << std::setw(8) << "START:" << DAF_Date_Time(start_time) << " GMT" << sTime << std::endl;
        }

        if (ed.info_) {
            *this << '\t' << std::setw(8) << "DESC:" << '\"' << ed.info_.in() << '\"' << std::endl;
        }

        try {
            this->insert_profile(ed.obj_);
        }
        catch (const CORBA::Exception &ex) {
            *this << '\t' << std::setw(8) << "ID:" << "EXCEPTION: \"" << ex._rep_id() << '\"' << std::endl;
        } DAF_CATCH_ALL {
            *this << '\t' << std::setw(8) << "ID:" << "INVALID Object Reference" << std::endl;
        }
    }

    void
    TAFServerFormatter::insert_properties(const taf::PropertyValueSequence_var &properties)
    {
        if (CORBA::is_nil(properties) ? false : properties->length() > 0) {
            *this << "\tProperties:" << std::endl;
            for (CORBA::ULong i = 0; i < properties->length(); i++) {
                *this   << "\t  " << std::setw(20) << properties[i].ident.in()
                        << "\t= " << properties[i].value.in() << std::endl;
            }
        }
    }

    TAFServerFormatter::TAFServerFormatter(const taf::TAFServer_var &taf_server, size_t index)
    {
        if (CORBA::is_nil(taf_server.in())) {
            throw CORBA::BAD_PARAM();
        }

        CORBA::String_var svc_name(taf_server->svc_name());

        if (svc_name && DAF_OS::strlen(svc_name.in())) {

            char msg[BUFSIZ];

            DAF_OS::sprintf(msg, ACE_TEXT("Hi there %s, Discovery Utility on '%s' can see you!!")
                , svc_name.in(), DAF_OS::gethostname().c_str());

            taf_server->sendConsoleMsg(msg);
        }

        this->insert_descriptor(taf::EntityDescriptor_var(taf_server->entity_descriptor()), index);

        if (properties_ || verbose() > 1) try {
            this->insert_properties(taf_server->list_properties());
        } DAF_CATCH_ALL {}


        {
            EntityDescriptorVector ed(taf_server->listServices());

            if (ed.size()) for (size_t i = 0; i < ed.size(); i++) try {
                this->insert_descriptor(ed[i], i + 1);
            } DAF_CATCH_ALL{}
        }

        std::ends(*this);
    }
}

int main(int argc, char *argv[])
{
    const struct _Shutdown : DAF::ShutdownHandler {
        virtual int handle_shutdown(int sig = 0) {
            return DAF::ShutdownHandler::handle_shutdown(sig);
        }
    } shutdown_;

    if (parse_args(argc, argv) == 0) try {

        TAF::ORBManager orb(argc, argv); orb.run(2);

        TAF::IORQueryReplyHandler replyHandler; // IOR Reply Handler

        ACE_DEBUG((LM_INFO, ACE_TEXT("********** Multicast Discovery Utility awaiting initial response - %D **********\n")));

        const std::string mcast_address(DAF::get_property(TAF_DISCOVERYENDPOINT, TAF_DEFAULT_DISCOVERY_ENDPOINT, true));

//        const ACE_INET_Addr MCAST_ADDRESS(Thestd::string
        for (int z = 0; !shutdown_ ; z++) {

            if (TAFDiscoveryHandler(ACE_INET_Addr(mcast_address.c_str())).sendIORQuery(replyHandler, taf::_tc_TAFServer->id()) == 0) {

                IORReplyVector      ior_seq(replyHandler.getIORReply(DISCOVER_TIMOUT));

                ACE_DEBUG((LM_INFO, ACE_TEXT("********* [%D] **********\n")));

                if (ior_seq.size()) {

                    if (!shutdown_) for (size_t i = 0; i < ior_seq.size(); i++) {

                        CORBA::Object_var   tafServerObj(ior_seq[i].svc_obj);
                        const std::string   ident(ior_seq[i].svc_ident.in());

                        try {
                            std::cout << TAFServerFormatter(taf::TAFServer::_narrow(tafServerObj), i) << std::endl;
                        }
                        catch (const CORBA::NO_PERMISSION &) {
                            if (debug()) {
                                ACE_DEBUG((LM_WARNING,
                                    ACE_TEXT("WARNING: NO_PERMISSION; Attempted security permission violation [ident=%s].\n")
                                    , ident.c_str()));
                            }
                        }
                        catch (const CORBA::INV_POLICY &) {
                            if (debug()) {
                                ACE_DEBUG((LM_WARNING,
                                    ACE_TEXT("WARNING: CORBA::INV_POLICY; Attempted security policy violation [ident=%s].\n")
                                    , ident.c_str()));
                            }
                        }
                        catch (const CORBA::OBJECT_NOT_EXIST &) {
                            if (debug()) {
                                ACE_DEBUG((LM_WARNING,
                                    ACE_TEXT("WARNING: CORBA::OBJECT_NOT_EXIST; Unable to locate instance [ident=%s].\n")
                                    , ident.c_str()));
                            }
                        }
                        catch (const CORBA::TRANSIENT &) {
                            if (debug()) {
                                ACE_DEBUG((LM_WARNING,
                                    ACE_TEXT("WARNING: CORBA::TRANSIENT; Unable to locate instance [ident=%s].\n")
                                    , ident.c_str()));
                            }
                        }
                        catch (const CORBA::Exception &ex) {
                            if (debug_) { // Ignore and keep going - TAFServer probably shutdown
                                ex._tao_print_exception("WE BROKE - Inner Loop.");
                            }
                        }
                        DAF_CATCH_ALL {
                            break;
                        }
                    }
                    continue;
                }

                ACE_DEBUG((LM_DEBUG, ACE_TEXT("Discovery Query[%04d]: No response received.\n"), z));
            }
        }
    } catch(const CORBA::Exception &ex) {
        ex._tao_print_exception("WE BROKE - Outer Loop."); return -1;
    } DAF_CATCH_ALL {
        return -1;
    }

    return 0;
}
