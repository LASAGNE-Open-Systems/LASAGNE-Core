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
#ifndef TAFDDS_DDSDEFS_H
#define TAFDDS_DDSDEFS_H

#include "DDSPubSub_export.h"

#include <daf/DAF.h>
#include <daf/RefCount.h>

#include <ace/CDR_Base.h>

#if !defined(ACE_WIN32)
#  if !defined(TAF_USES_DDS_NAMESPACE)
// DISABLED until further notice - JAB
//#    define TAF_USES_DDS_NAMESPACE 1
#  endif // !TAF_USES_DDS_NAMESPACE
#endif // !ACE_WIN32

/**
 * TAF_USES_DDS_NAMESPACE macros
 * Allows certain symbols to have completely different symbols for different
 * DDS implementations. This aids when the dynamic linker is working via
 * symbol resolution.
 */
#if defined(TAF_USES_DDS_NAMESPACE) && TAF_USES_DDS_NAMESPACE == 1

# if   defined(TAF_USES_OPENDDS)
#   define TAF_DDS_NAMESPACE_NAME TAFDDS_ODDS
# elif defined(TAF_USES_NDDS)
#   define TAF_DDS_NAMESPACE_NAME TAFDDS_NDDS
# elif defined(TAF_USES_CDDS)
#   define TAF_DDS_NAMESPACE_NAME TAFDDS_CDDS
# elif defined(TAF_USES_OPENSPLICE)
#   define TAF_DDS_NAMESPACE_NAME TAFDDS_OSPL
# else
#   define TAF_DDS_NAMESPACE_NAME TAFDDS_DDS
# endif // TAF_DDS_NAMESPACE_NAME

// Main Macros to be used
# define TAF_BEGIN_DDS_NAMESPACE_DECL namespace TAF_DDS_NAMESPACE_NAME {
# define TAF_END_DDS_NAMESPACE_DECL } \
    using namespace ::TAF_DDS_NAMESPACE_NAME;

#else // TAF_USES_DDS_NAMESPACE

#  define TAF_DDS_NAMESPACE_NAME
#  define TAF_BEGIN_DDS_NAMESPACE_DECL
#  define TAF_END_DDS_NAMESPACE_DECL

#endif // TAF_USES_DDS_NAMESPACE

#if defined(TAF_USES_OPENDDS)

# if defined(_MSC_VER)
#  pragma message("------> TAF_USES_OPENDDS (+CORBA) defined")
# endif

# define DDS_IMPLEMENTATION_NAME        ACE_TEXT("OpenDDS")
# define OPENDDS_RUNTIME_DLLNAME        ACE_TEXT("OpenDDS_Dcps")

# define OPENDDS_MONITOR_FACORY         ACE_TEXT("OpenDDS_Monitor")
# define OPENDDS_MONITOR_FACORY_DEFAULT ACE_TEXT("OpenDDS_Monitor_Default")

# include <dds/DCPS/Service_Participant.h>
# include <dds/DCPS/Marked_Default_Qos.h>
# include <dds/DCPS/Registered_Data_Types.h>
# include <dds/Version.h>

namespace OpenDDS {
    namespace DCPS {
        DAF_UNUSED_STATIC(OpenDDS_Requires_MonitorFactory_Initializer) /* Stops Warnings */
    }
}

// Seems to fix problem with Take crashing on exit
//# define OPENDDS_USES_TAKE_NEXT_SAMPLE  1

#if !defined(OPENDDS_USES_TAKE_NEXT_SAMPLE)
# if (DDS_MAJOR_VERSION > 3)
#  define OPENDDS_USES_TAKE_NEXT_SAMPLE 0
# elif (DDS_MAJOR_VERSION == 3) && (DDS_MINOR_VERSION >= 7)
#  define OPENDDS_USES_TAKE_NEXT_SAMPLE 0
# else
#  define OPENDDS_USES_TAKE_NEXT_SAMPLE 1
# endif
#endif

namespace DDS {

    typedef ACE_CDR::Boolean                Boolean;
    typedef ACE_CDR::Octet                  Octet;
    typedef ACE_CDR::Char                   Char;
    typedef ACE_CDR::WChar                  WChar;
    typedef ACE_CDR::Short                  Short;
    typedef ACE_CDR::UShort                 UnsignedShort;
    typedef ACE_CDR::Long                   Long;
    typedef ACE_CDR::ULong                  UnsignedLong;
    typedef ACE_CDR::LongLong               LongLong;
    typedef ACE_CDR::ULongLong              UnsignedLongLong;
    typedef ACE_CDR::Float                  Float;
    typedef ACE_CDR::Double                 Double;
    typedef ACE_CDR::LongDouble             LongDouble;

    typedef const DDS::Char*                String_ptr;
    typedef const DDS::WChar*               WString_ptr;

    typedef CORBA::String_var               String_ref;
    typedef CORBA::WString_var              WString_ref;

    typedef DomainParticipantFactory_var    DomainParticipantFactory_ref;
    typedef DomainParticipant_var           DomainParticipant_ref;
    typedef Publisher_var                   Publisher_ref;
    typedef Subscriber_var                  Subscriber_ref;
    typedef DataReader_var                  DataReader_ref;
    typedef DataWriter_var                  DataWriter_ref;
    typedef Topic_var                       Topic_ref;

    const DDS::StatusMask                   STATUS_MASK_ALL(-1);
    const DDS::StatusMask                   STATUS_MASK_NONE(0);
} // namespace DDS

/* Cludge for OpenDDS.. to ensure against leakage */
# define TheDomainParticipantFactory \
    DDS::DomainParticipantFactory_ref(TheParticipantFactory)
# define TheDomainParticipantFactoryWithArgs(argc,argv) \
    DDS::DomainParticipantFactory_ref(TheParticipantFactoryWithArgs(argc,argv))

#elif defined(TAF_USES_NDDS)

# if defined(_MSC_VER)
#  if defined(TAF_USES_DDSCORBA)
#   pragma message("------> TAF_USES_NDDS (+CORBA) defined")
#  else
#   pragma message("------> TAF_USES_NDDS defined")
#  endif
# endif

# define DDS_IMPLEMENTATION_NAME    ACE_TEXT("RTI-DDS")

# include <ndds/ndds_namespace_cpp.h>
# include <ndds/ndds_version.h>

# define DDS_MAJOR_VERSION  RTI_DDS_VERSION_MAJOR
# define DDS_MINOR_VERSION  RTI_DDS_VERSION_MINOR
# define DDS_MICRO_VERSION  RTI_DDS_VERSION_RELEASE

namespace DDS {

    typedef const char*                         String_ptr;

    typedef DomainParticipantFactory*           DomainParticipantFactory_ptr;
    typedef DomainParticipant*                  DomainParticipant_ptr;
    typedef Publisher*                          Publisher_ptr;
    typedef Subscriber*                         Subscriber_ptr;
    typedef DataReader*                         DataReader_ptr;
    typedef DataWriter*                         DataWriter_ptr;
    typedef Topic*                              Topic_ptr;

    typedef String_ptr                          String_ref;

    typedef DomainParticipantFactory_ptr        DomainParticipantFactory_ref;
    typedef DomainParticipant_ptr               DomainParticipant_ref;
    typedef Publisher_ptr                       Publisher_ref;
    typedef Subscriber_ptr                      Subscriber_ref;
    typedef DataReader_ptr                      DataReader_ref;
    typedef DataWriter_ptr                      DataWriter_ref;
    typedef Topic_ptr                           Topic_ref;

    /* These are not put into DDS Namespace by RTI ??? */

    typedef DDS_DomainParticipantQos            DomainParticipantQos;
    typedef DDS_TopicQos                        TopicQos;
    typedef DDS_PublisherQos                    PublisherQos;
    typedef DDS_SubscriberQos                   SubscriberQos;
    typedef DDS_DataWriterQos                   DataWriterQos;
    typedef DDS_DataReaderQos                   DataReaderQos;
} // namespace DDS

# define TheDomainParticipantFactory \
    DDS::DomainParticipantFactory_ref(TheParticipantFactory)
# define TheDomainParticipantFactoryWithArgs(argc,argv) \
    DDS::DomainParticipantFactory_ref(TheParticipantFactory)

#elif defined(TAF_USES_COREDX)

# if defined(_MSC_VER)
#  if defined(TAF_USES_DDSCORBA)
#   pragma message("------> TAF_USES_COREDX (+CORBA) defined")
#  else
#   pragma message("------> TAF_USES_COREDX defined")
#  endif
# endif

# define DDS_IMPLEMENTATION_NAME    ACE_TEXT("Core-DX")

# include <dds/dds.hh>
# include <dds/coredx_version.h>

# define DDS_MAJOR_VERSION  COREDX_DDS_VERSION_MAJOR
# define DDS_MINOR_VERSION  COREDX_DDS_VERSION_MINOR
# define DDS_MICRO_VERSION  COREDX_DDS_VERSION_PATCH

namespace DDS {

    typedef ACE_CDR::Boolean                Boolean;
    typedef ACE_CDR::Octet                  Octet;
    typedef ACE_CDR::Char                   Char;
    typedef ACE_CDR::WChar                  WChar;
    typedef ACE_CDR::Short                  Short;
    typedef ACE_CDR::UShort                 UnsignedShort;
    typedef ACE_CDR::Long                   Long;
    typedef ACE_CDR::ULong                  UnsignedLong;
    typedef ACE_CDR::LongLong               LongLong;
    typedef ACE_CDR::ULongLong              UnsignedLongLong;
    typedef ACE_CDR::Float                  Float;
    typedef ACE_CDR::Double                 Double;
    typedef ACE_CDR::LongDouble             LongDouble;

    typedef DomainParticipantFactory*       DomainParticipantFactory_ptr;
    typedef DomainParticipant*              DomainParticipant_ptr;
    typedef Publisher*                      Publisher_ptr;
    typedef Subscriber*                     Subscriber_ptr;
    typedef DataReader*                     DataReader_ptr;
    typedef DataWriter*                     DataWriter_ptr;
    typedef Topic*                          Topic_ptr;

    typedef const DDS::Char*                String_ptr;
    typedef const DDS::WChar*               WString_ptr;

    typedef String_ptr                      String_ref;
    typedef WString_ptr                     WString_ref;

    typedef DomainParticipantFactory_ptr    DomainParticipantFactory_ref;
    typedef DomainParticipant_ptr           DomainParticipant_ref;
    typedef Publisher_ptr                   Publisher_ref;
    typedef Subscriber_ptr                  Subscriber_ref;
    typedef DataReader_ptr                  DataReader_ref;
    typedef DataWriter_ptr                  DataWriter_ref;
    typedef Topic_ptr                       Topic_ref;

    const DDS::StatusMask                   STATUS_MASK_ALL(-1);
    const DDS::StatusMask                   STATUS_MASK_NONE(0);
} // namespace DDS

# if !defined(TheParticipantFactory)
#  define TheParticipantFactory  (DDS::DomainParticipantFactory::get_instance())
# endif

# define TheDomainParticipantFactory \
    DDS::DomainParticipantFactory_ref(TheParticipantFactory)
# define TheDomainParticipantFactoryWithArgs(argc, argv) \
    DDS::DomainParticipantFactory_ref(TheParticipantFactory)

#elif defined(TAF_USES_OPENSPLICE)

# if defined(_MSC_VER)
#  if defined(TAF_USES_DDSCORBA)
#   pragma message("------> TAF_USES_OPENSPLICE (+CORBA) defined")
#  else
#   pragma message("------> TAF_USES_OPENSPLICE defined")
#  endif
# endif

# define DDS_IMPLEMENTATION_NAME    ACE_TEXT("OpenSplice")

// Sourced via the $OSPL_HOME/etc/RELEASEINFO and environment variable
# define DDS_MAJOR_VERSION  OSPL_MAJOR_VERSION
# define DDS_MINOR_VERSION  OSPL_MINOR_VERSION
# define DDS_MICRO_VERSION  OSPL_MAINT_VERSION

#include "ccpp_dds_dcps.h"

#if defined(TAF_USES_DDSCORBA)

namespace DDS {

    typedef DDS::ULong                      UnsignedLong;
    typedef DDS::ULongLong                  UnsignedLongLong;

    typedef const char*                     String_ptr;
    typedef CORBA::WChar*                   WString_ptr;

    typedef CORBA::String_var               String_ref;
    typedef CORBA::WString_var              WString_ref;

    typedef DomainParticipantFactoryInterface_var    DomainParticipantFactory_ref;
    typedef DomainParticipant_var           DomainParticipant_ref;
    typedef Publisher_var                   Publisher_ref;
    typedef Subscriber_var                  Subscriber_ref;
    typedef DataReader_var                  DataReader_ref;
    typedef DataWriter_var                  DataWriter_ref;
    typedef Topic_var                       Topic_ref;

    const DDS::StatusMask                   STATUS_MASK_ALL(0xFE7);
} // namespace DDS

#else //TAF_USES_DDSCORBA

#include "dds_dcps.h"

namespace DDS {

    typedef DDS::ULong                      UnsignedLong;
    typedef DDS::ULongLong                  UnsignedLongLong;

    typedef const char *                    String_ptr;
    typedef String_var                      String_ref;

    typedef DomainParticipantFactory_ptr    DomainParticipantFactory_ref;
    typedef DomainParticipant_ptr           DomainParticipant_ref;
    typedef Publisher_ptr                   Publisher_ref;
    typedef Subscriber_ptr                  Subscriber_ref;
    typedef DataReader_ptr                  DataReader_ref;
    typedef DataWriter_ptr                  DataWriter_ref;
    typedef Topic_ptr                       Topic_ref;

    const DDS::StatusMask                   STATUS_MASK_ALL(0xFE7);
} // namespace DDS

#endif // TAF_USESS_DDSCORBA

# define TheDomainParticipantFactory \
    DDS::DomainParticipantFactory_ref(TheParticipantFactory)
# define TheDomainParticipantFactoryWithArgs(argc,argv) \
    DDS::DomainParticipantFactory_ref(TheParticipantFactory)

#else

# error "Unknown DDS Version?"

#endif

TAF_BEGIN_DDS_NAMESPACE_DECL

namespace TAFDDS
{
    const DDS::DomainId_t   DEFAULT_DOMAIN(0);

    const struct Duration_t : DDS::Duration_t {
        Duration_t(DDS::Long sec, DDS::UnsignedLong nanosec = 0) {
            this->sec = sec; this->nanosec = nanosec;
        }
        Duration_t(const ACE_Time_Value &tm) {
            this->sec = DDS::Long(tm.sec()); this->nanosec = DDS::UnsignedLong(tm.usec() * 1000UL);
        }
    } Duration_ZERO(ACE_Time_Value::zero), Duration_INFINITE(ACE_Time_Value::max_time);

    class DDSPubSub_Export TypeSupportOperations
#if defined(TAF_USES_OPENDDS)
        : virtual public OpenDDS::DCPS::TypeSupport, ACE_Copy_Disabled
#else
        : virtual public DAF::RefCount
#endif
    {
    public:
        DAF_DEFINE_REFCOUNTABLE(TypeSupportOperations);
        virtual ~TypeSupportOperations(void) { /* force propper distruction */ }
        virtual DDS::ReturnCode_t   registerTypename(DDS::DomainParticipant_ptr) = 0;
        virtual DDS::ReturnCode_t   unregisterTypename(DDS::DomainParticipant_ptr) = 0;
        virtual DDS::String_ptr     getTypename(void) const = 0;
    };
    DAF_DECLARE_REFCOUNTABLE(TypeSupportOperations);
} // namespace TAFDDS

TAF_END_DDS_NAMESPACE_DECL

typedef class TAF_DDS_NAMESPACE_NAME::TAFDDS::TypeSupportOperations TAFDDS_TypeSupportOperations;

namespace DDS {

    typedef TAFDDS::TypeSupportOperations_ref   TypeSupport_ref;

#if defined(TAF_USES_DDSCORBA) && !defined(TAF_USES_NDDS)
    inline char* String_dup(const char *p) {
        return p ? CORBA::string_dup(p) : 0;
    }
#elif defined(TAF_USES_COREDX)
    inline char* String_dup(const char *p) {
        return p ? ACE::strnew(p) : 0;
    }
    inline char* String_replace(char **dst, const char *src) {
        if (dst) { delete [] *dst; return *dst = DDS::String_dup(src); } return 0;
    }
#elif defined(TAF_USES_OPENSPLICE)
    inline char* String_dup(const char *p) {
        return ::DDS::string_dup(p);
    }

    inline char* String_replace(char **dst, const char *src) {
        if (dst) { delete [] *dst; return *dst = DDS::String_dup(src); }
        return 0;
    }

    inline char* String_replace(::DDS::String_mgr *dst, const char *src) {
        if (dst) { return *dst = DDS::String_dup(src); }
        return 0;
    }
#endif
}  // namespace DDS

#if defined(TAF_USES_DDSCORBA)
# define DEFINE_DDS_CLASSSUPPORT(CLS,TYP) CLS::TYP
#elif defined(TAF_USES_NDDS)
# define DEFINE_DDS_CLASSSUPPORT(CLS,TYP)                                   \
struct _##CLS##_##TYP##ClassSupport : CLS::TYP {                            \
    _##CLS##_##TYP##ClassSupport(void)                                      \
        { CLS::TYP##_initialize(this); }                                    \
    _##CLS##_##TYP##ClassSupport(const CLS::TYP &t)                         \
        { CLS::TYP##_initialize(this);CLS::TYP##_copy(this, &t); }          \
    _##CLS##_##TYP##ClassSupport(const _##CLS##_##TYP##ClassSupport &t)     \
        { CLS::TYP##_initialize(this);CLS::TYP##_copy(this, &t); }          \
    ~_##CLS##_##TYP##ClassSupport(void) { CLS::TYP##_finalize(this); }      \
    _##CLS##_##TYP##ClassSupport& operator = (const CLS::TYP &t)            \
        { if (&t != this) { CLS::TYP##_finalize(this);CLS::TYP##_initialize(this);CLS::TYP##_copy(this, &t); } return *this; } \
    _##CLS##_##TYP##ClassSupport& operator = (const _##CLS##_##TYP##ClassSupport &t) \
        { if (&t != this) { CLS::TYP##_finalize(this);CLS::TYP##_initialize(this);CLS::TYP##_copy(this, &t); } return *this; } \
} /* Note!!! No closing ';' to force user to close macro usage */
#elif defined(TAF_USES_COREDX)
# define DEFINE_DDS_CLASSSUPPORT(CLS,TYP) CLS::TYP
#elif defined(TAF_USES_OPENSPLICE)
# define DEFINE_DDS_CLASSSUPPORT(CLS,TYP) CLS::TYP
#endif

#if defined(TAF_USES_OPENDDS)

/***********************************************************************************************/
#define DEFINE_DDS_TYPESUPPORT(CLS,TYP)                                                         \
class _##CLS##_##TYP##TypeSupport   : virtual public TAFDDS_TypeSupportOperations               \
                                    , virtual public CLS::TYP##TypeSupportImpl                  \
{   const std::string type_name_;                                                               \
public:                                                                                         \
    typedef CLS::TYP                                        _data_type;                         \
    typedef CLS::TYP##Seq                                   _data_seq_type;                     \
    typedef DEFINE_DDS_CLASSSUPPORT(CLS,TYP)                _data_holder_type;                  \
    typedef CLS::TYP##DataReader                            _data_reader_stub_type;             \
    typedef _data_reader_stub_type::_ptr_type               _data_reader_stub_type_ptr;         \
    typedef _data_reader_stub_type::_var_type               _data_reader_stub_type_ref;         \
    typedef CLS::TYP##DataWriter                            _data_writer_stub_type;             \
    typedef _data_writer_stub_type::_ptr_type               _data_writer_stub_type_ptr;         \
    typedef _data_writer_stub_type::_var_type               _data_writer_stub_type_ref;         \
    typedef CLS::TYP##TypeSupportImpl                       _support_impl_type;                 \
    typedef _##CLS##_##TYP##TypeSupport                     _support_type;                      \
    _##CLS##_##TYP##TypeSupport(DDS::String_ptr type_name = 0)                                  \
        : type_name_(type_name ? type_name : "_" #CLS "_" #TYP) {}                              \
    static void printData(const _data_type&) { /* Not Supported */ }                            \
    static _data_reader_stub_type_ptr narrow(DDS::DataReader_ptr p)                             \
        { return _data_reader_stub_type::_narrow(p); }                                          \
    static _data_writer_stub_type_ptr narrow(DDS::DataWriter_ptr p)                             \
        { return _data_writer_stub_type::_narrow(p); }                                          \
    virtual DDS::String_ptr getTypename(void) const                                             \
        { return this->type_name_.c_str(); }                                                    \
private:                                                                                        \
    virtual DDS::ReturnCode_t registerTypename(DDS::DomainParticipant_ptr participant)          \
        { return _support_impl_type::register_type(participant, this->getTypename()); }         \
    virtual DDS::ReturnCode_t unregisterTypename(DDS::DomainParticipant_ptr)                    \
        { return DDS::RETCODE_OK; }                                                             \
} /* Note!!! No closing ';' to force user to close macro usage */
/***********************************************************************************************/

#elif defined(TAF_USES_NDDS)

/***********************************************************************************************/
#define DEFINE_DDS_TYPESUPPORT(CLS,TYP)                                                         \
class _##CLS##_##TYP##TypeSupport : virtual public TAFDDS_TypeSupportOperations                 \
{   const std::string type_name_;                                                               \
public:                                                                                         \
    typedef CLS::TYP                                    _data_type;                             \
    typedef CLS::TYP##Seq                               _data_seq_type;                         \
    typedef DEFINE_DDS_CLASSSUPPORT(CLS,TYP)            _data_holder_type;                      \
    typedef CLS::TYP##DataReader                        _data_reader_stub_type;                 \
    typedef CLS::TYP##DataReader*                       _data_reader_stub_type_ptr;             \
    typedef CLS::TYP##DataReader*                       _data_reader_stub_type_ref;             \
    typedef CLS::TYP##DataWriter                        _data_writer_stub_type;                 \
    typedef CLS::TYP##DataWriter*                       _data_writer_stub_type_ptr;             \
    typedef CLS::TYP##DataWriter*                       _data_writer_stub_type_ref;             \
    typedef CLS::TYP##TypeSupport                       _support_impl_type;                     \
    typedef _##CLS##_##TYP##TypeSupport                 _support_type;                          \
    _##CLS##_##TYP##TypeSupport(DDS::String_ptr type_name = 0)                                  \
        : type_name_(type_name ? type_name : "_" #CLS "_" #TYP) {}                              \
    static void printData(const _data_type &dt)                                                 \
        { _support_impl_type::print_data(&dt); }                                                \
    static _data_reader_stub_type_ptr narrow(DDS::DataReader_ptr p)                             \
        { return _data_reader_stub_type::narrow(p); }                                           \
    static _data_writer_stub_type_ptr narrow(DDS::DataWriter_ptr p)                             \
        { return _data_writer_stub_type::narrow(p); }                                           \
    virtual DDS::String_ptr getTypename(void) const                                             \
        { return this->type_name_.c_str(); }                                                    \
private:                                                                                        \
    virtual DDS::ReturnCode_t registerTypename(DDS::DomainParticipant_ptr participant)          \
        { return _support_impl_type::register_type(participant, this->getTypename()); }         \
    virtual DDS::ReturnCode_t unregisterTypename(DDS::DomainParticipant_ptr participant)        \
        { return _support_impl_type::unregister_type(participant, this->getTypename()); }       \
} /* Note!!! No closing ';' to force user to close macro usage */
/***********************************************************************************************/

#elif defined(TAF_USES_COREDX)

/***********************************************************************************************/
#define DEFINE_DDS_TYPESUPPORT(CLS,TYP)                                                         \
class _##CLS##_##TYP##TypeSupport : virtual public TAFDDS_TypeSupportOperations                 \
{   const std::string type_name_;                                                               \
public:                                                                                         \
    typedef CLS::TYP                                    _data_type;                             \
    typedef CLS::TYP##PtrSeq                            _data_seq_type;                         \
    typedef DEFINE_DDS_CLASSSUPPORT(CLS,TYP)            _data_holder_type;                      \
    typedef CLS::TYP##DataReader                        _data_reader_stub_type;                 \
    typedef CLS::TYP##DataReader*                       _data_reader_stub_type_ptr;             \
    typedef CLS::TYP##DataReader*                       _data_reader_stub_type_ref;             \
    typedef CLS::TYP##DataWriter                        _data_writer_stub_type;                 \
    typedef CLS::TYP##DataWriter*                       _data_writer_stub_type_ptr;             \
    typedef CLS::TYP##DataWriter*                       _data_writer_stub_type_ref;             \
    typedef CLS::TYP##TypeSupport                       _support_impl_type;                     \
    typedef _##CLS##_##TYP##TypeSupport                 _support_type;                          \
    _##CLS##_##TYP##TypeSupport(DDS::String_ptr type_name = 0)                                  \
        : type_name_(type_name ? type_name : "_" #CLS "_" #TYP) {}                              \
    static void printData(const _data_type&) { /* Not Supported */ }                            \
    static _data_reader_stub_type_ptr narrow(DDS::DataReader_ptr p)                             \
        { return _data_reader_stub_type::narrow(p); }                                           \
    static _data_writer_stub_type_ptr narrow(DDS::DataWriter_ptr p)                             \
        { return _data_writer_stub_type::narrow(p); }                                           \
    virtual DDS::String_ptr getTypename(void) const                                             \
        { return this->type_name_.c_str(); }                                                    \
private:                                                                                        \
    virtual DDS::ReturnCode_t registerTypename(DDS::DomainParticipant_ptr participant)          \
        { return _support_impl_type::register_type(participant, this->getTypename()); }         \
    virtual DDS::ReturnCode_t unregisterTypename(DDS::DomainParticipant_ptr)                    \
        { return DDS::RETCODE_OK; }                                                             \
} /* Note!!! No closing ';' to force user to close macro usage */
/***********************************************************************************************/

#elif defined(TAF_USES_OPENSPLICE)

/***********************************************************************************************/
#define DEFINE_DDS_TYPESUPPORT(CLS,TYP)                                                         \
class _##CLS##_##TYP##TypeSupport   : virtual public TAFDDS_TypeSupportOperations               \
                                    , virtual public CLS::TYP##TypeSupport                      \
{   const std::string type_name_;                                                               \
public:                                                                                         \
    typedef CLS::TYP                                        _data_type;                         \
    typedef CLS::TYP##Seq                                   _data_seq_type;                     \
    typedef DEFINE_DDS_CLASSSUPPORT(CLS,TYP)                _data_holder_type;                  \
    typedef CLS::TYP##DataReader                            _data_reader_stub_type;             \
    typedef _data_reader_stub_type::_ptr_type               _data_reader_stub_type_ptr;         \
    typedef _data_reader_stub_type::_var_type               _data_reader_stub_type_ref;         \
    typedef CLS::TYP##DataWriter                            _data_writer_stub_type;             \
    typedef _data_writer_stub_type::_ptr_type               _data_writer_stub_type_ptr;         \
    typedef _data_writer_stub_type::_var_type               _data_writer_stub_type_ref;         \
    typedef CLS::TYP##TypeSupport                           _support_impl_type;                 \
    typedef _##CLS##_##TYP##TypeSupport                     _support_type;                      \
    _##CLS##_##TYP##TypeSupport(DDS::String_ptr type_name = 0)                                  \
        : type_name_(type_name ? type_name : "_" #CLS "_" #TYP) {}                              \
    static void printData(const _data_type&) { /* Not Supported */ }                            \
    static _data_reader_stub_type_ptr narrow(DDS::DataReader_ptr p)                             \
        { return _data_reader_stub_type::_narrow(p); }                                          \
    static _data_writer_stub_type_ptr narrow(DDS::DataWriter_ptr p)                             \
        { return _data_writer_stub_type::_narrow(p); }                                          \
    virtual DDS::String_ptr getTypename(void) const                                             \
        { return this->type_name_.c_str(); }                                                    \
private:                                                                                        \
    virtual DDS::ReturnCode_t registerTypename(DDS::DomainParticipant_ptr participant)          \
        { return _support_impl_type::register_type(participant, this->getTypename()); }         \
    virtual DDS::ReturnCode_t unregisterTypename(DDS::DomainParticipant_ptr)                    \
        { return DDS::RETCODE_OK; }                                                             \
} /* Note!!! No closing ';' to force user to close macro usage */
/***********************************************************************************************/

#endif

#endif // TAFDDS_DDSDEFS_H
