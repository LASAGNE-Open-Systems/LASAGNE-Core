#ifndef LTM_CORBACTIVEDATASUPPORT_H
#define LTM_CORBACTIVEDATASUPPORT_H

#include "CORBActiveServiceC.h"

#if defined(TAF_USES_DDSCORBA)

# include "CORBActiveServiceS.h"

#elif defined(TAF_USES_NDDS)

# include "CORBActiveService.h"

#elif defined(TAF_USES_COREDX)

# include "CORBActiveService.hh"

#elif defined(TAF_USES_OPENSPLICE)

# include "ccpp_CORBActiveService.h"

#endif

#if defined(TAF_USES_OPENDDS)

# include "CORBActiveServiceTypeSupportC.h"
# include "CORBActiveServiceTypeSupportS.h"
# include "CORBActiveServiceTypeSupportImpl.h"

#elif defined(TAF_USES_NDDS)

# include "CORBActiveServiceSupport.h"
# include "CORBActiveServicePlugin.h"

#elif defined(TAF_USES_COREDX)

# include "CORBActiveServiceTypeSupport.hh"
# include "CORBActiveServiceDataReader.hh"
# include "CORBActiveServiceDataWriter.hh"

#elif defined(TAF_USES_OPENSPLICE)

#if defined(TAF_USES_DDSCORBA)
# include "CORBActiveServiceDcpsC.h"
#else
# include "CORBActiveServiceDcps.h"
#endif

# include "CORBActiveServiceDcps_impl.h"

#endif

#include "dds/DDSPubSub.h"

namespace LTM
{
#if !defined(DEFINE_DDS_TYPESUPPORT_ltm_LTMTopicDetails)
#   define  DEFINE_DDS_TYPESUPPORT_ltm_LTMTopicDetails
# if defined(TAF_USES_DDSCORBA)
    typedef DEFINE_DDS_TYPESUPPORT(ltm, LTMTopicDetails)        LTMTopicDetailsDataSupport;
    typedef LTMTopicDetailsDataSupport::_data_holder_type       LTMTopicDetailsHolderType;
    typedef LTMTopicDetailsDataSupport::_data_type              LTMTopicDetailsTopicType;
    typedef LTMTopicDetailsDataSupport::_data_type              LTMTopicDetailsCORBAType;
    typedef LTMTopicDetailsTopicType                            LTMTopicDetailsTopicAdapter;
    typedef LTMTopicDetailsCORBAType                            LTMTopicDetailsCORBAdapter;
# else
    typedef DEFINE_DDS_TYPESUPPORT(ltm_dds, LTMTopicDetails)    LTMTopicDetailsDataSupport;
    typedef LTMTopicDetailsDataSupport::_data_holder_type       LTMTopicDetailsHolderType;
    typedef LTMTopicDetailsDataSupport::_data_type              LTMTopicDetailsTopicType;
    typedef ltm::LTMTopicDetails                                LTMTopicDetailsCORBAType;
    struct CORBActiveService_Export LTMTopicDetailsTopicAdapter : LTMTopicDetailsHolderType {
        LTMTopicDetailsTopicAdapter(void)  {}
        LTMTopicDetailsTopicAdapter(const LTMTopicDetailsCORBAType &);
    };
    struct CORBActiveService_Export LTMTopicDetailsCORBAdapter  : LTMTopicDetailsCORBAType  {
        LTMTopicDetailsCORBAdapter(void)   {}
        LTMTopicDetailsCORBAdapter(const LTMTopicDetailsTopicType &);
    };
# endif
#endif
}

#endif // LTM_CORBACTIVEDATASUPPORT_H
