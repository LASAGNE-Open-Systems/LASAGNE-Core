#ifndef LTM_DDSACTIVEDATASUPPORT_H
#define LTM_DDSACTIVEDATASUPPORT_H

#include "LTMTopicDetailsC.h"

#if defined(TAF_USES_DDSCORBA)

# include "LTMTopicDetailsS.h"

#elif defined(TAF_USES_NDDS)

# include "LTMTopicDetails.h"

#elif defined(TAF_USES_COREDX)

# include "LTMTopicDetails.hh"

#elif defined(TAF_USES_OPENSPLICE)

# include "ccpp_LTMTopicDetails.h"

#endif

#if defined(TAF_USES_OPENDDS)

# include "LTMTopicDetailsTypeSupportC.h"
# include "LTMTopicDetailsTypeSupportS.h"
# include "LTMTopicDetailsTypeSupportImpl.h"

#elif defined(TAF_USES_NDDS)

# include "LTMTopicDetailsSupport.h"
# include "LTMTopicDetailsPlugin.h"

#elif defined(TAF_USES_COREDX)

# include "LTMTopicDetailsTypeSupport.hh"
# include "LTMTopicDetailsDataReader.hh"
# include "LTMTopicDetailsDataWriter.hh"

#elif defined(TAF_USES_OPENSPLICE)

#if defined(TAF_USES_DDSCORBA)
# include "LTMTopicDetailsDcpsC.h"
#else
# include "LTMTopicDetailsDcps.h"
#endif

# include LTMTopicDetailsDcps_impl.h"

#endif

#include "dds/DDSPubSub.h"

namespace LTM
{
#if !defined(DEFINE_DDS_TYPESUPPORT_ltm_LTMTopicDetails)
#   define  DEFINE_DDS_TYPESUPPORT_ltm_LTMTopicDetails
    typedef ltm::LTMTopicDetails                                LTMTopicDetailsCORBAType;
# if defined(TAF_USES_DDSCORBA)
    typedef DEFINE_DDS_TYPESUPPORT(ltm, LTMTopicDetails)        LTMTopicDetailsDataSupport;
    typedef LTMTopicDetailsDataSupport::_data_holder_type       LTMTopicDetailsHolderType;
    typedef LTMTopicDetailsDataSupport::_data_type              LTMTopicDetailsTopicType;
    typedef LTMTopicDetailsTopicType                            LTMTopicDetailsTopicAdapter;
    typedef LTMTopicDetailsCORBAType                            LTMTopicDetailsCORBAdapter;
# else
    typedef DEFINE_DDS_TYPESUPPORT(ltm_dds, LTMTopicDetails)    LTMTopicDetailsDataSupport;
    typedef LTMTopicDetailsDataSupport::_data_holder_type       LTMTopicDetailsHolderType;
    typedef LTMTopicDetailsDataSupport::_data_type              LTMTopicDetailsTopicType;
    struct DDSActiveService_Export LTMTopicDetailsTopicAdapter : LTMTopicDetailsHolderType {
        LTMTopicDetailsTopicAdapter(void)  {}
        LTMTopicDetailsTopicAdapter(const LTMTopicDetailsCORBAType &);
    };
    struct DDSActiveService_Export LTMTopicDetailsCORBAdapter  : LTMTopicDetailsCORBAType {
        LTMTopicDetailsCORBAdapter(void)   {}
        LTMTopicDetailsCORBAdapter(const LTMTopicDetailsTopicType &);
    };
# endif
#endif
}

#endif // LTM_DDSACTIVEDATASUPPORT_H
