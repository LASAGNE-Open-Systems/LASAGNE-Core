
/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from LTMTopicDetails.idl using "rtiddsgen".
The rtiddsgen tool is part of the RTI Connext distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the RTI Connext manual.
*/

#include "LTMTopicDetailsSupport.h"
#include "LTMTopicDetailsPlugin.h"

#ifndef dds_c_log_impl_h              
#include "dds_c/dds_c_log_impl.h"                                
#endif        

namespace ltm_dds {

    /* ========================================================================= */
    /**
    <<IMPLEMENTATION>>

    Defines:   TData,
    TDataWriter,
    TDataReader,
    TTypeSupport

    Configure and implement 'LTMTimestamp' support classes.

    Note: Only the #defined classes get defined
    */

    /* ----------------------------------------------------------------- */
    /* DDSDataWriter
    */

    /**
    <<IMPLEMENTATION >>

    Defines:   TDataWriter, TData
    */

    /* Requires */
    #define TTYPENAME   LTMTimestampTYPENAME

    /* Defines */
    #define TDataWriter LTMTimestampDataWriter
    #define TData       ltm_dds::LTMTimestamp

    #include "dds_cpp/generic/dds_cpp_data_TDataWriter.gen"

    #undef TDataWriter
    #undef TData

    #undef TTYPENAME

    /* ----------------------------------------------------------------- */
    /* DDSDataReader
    */

    /**
    <<IMPLEMENTATION >>

    Defines:   TDataReader, TDataSeq, TData
    */

    /* Requires */
    #define TTYPENAME   LTMTimestampTYPENAME

    /* Defines */
    #define TDataReader LTMTimestampDataReader
    #define TDataSeq    LTMTimestampSeq
    #define TData       ltm_dds::LTMTimestamp

    #include "dds_cpp/generic/dds_cpp_data_TDataReader.gen"

    #undef TDataReader
    #undef TDataSeq
    #undef TData

    #undef TTYPENAME

    /* ----------------------------------------------------------------- */
    /* TypeSupport

    <<IMPLEMENTATION >>

    Requires:  TTYPENAME,
    TPlugin_new
    TPlugin_delete
    Defines:   TTypeSupport, TData, TDataReader, TDataWriter
    */

    /* Requires */
    #define TTYPENAME    LTMTimestampTYPENAME
    #define TPlugin_new  ltm_dds::LTMTimestampPlugin_new
    #define TPlugin_delete  ltm_dds::LTMTimestampPlugin_delete

    /* Defines */
    #define TTypeSupport LTMTimestampTypeSupport
    #define TData        ltm_dds::LTMTimestamp
    #define TDataReader  LTMTimestampDataReader
    #define TDataWriter  LTMTimestampDataWriter
    #define TGENERATE_SER_CODE
    #define TGENERATE_TYPECODE

    #include "dds_cpp/generic/dds_cpp_data_TTypeSupport.gen"

    #undef TTypeSupport
    #undef TData
    #undef TDataReader
    #undef TDataWriter
    #undef TGENERATE_TYPECODE
    #undef TGENERATE_SER_CODE
    #undef TTYPENAME
    #undef TPlugin_new
    #undef TPlugin_delete

    /* ========================================================================= */
    /**
    <<IMPLEMENTATION>>

    Defines:   TData,
    TDataWriter,
    TDataReader,
    TTypeSupport

    Configure and implement 'LTMTopicDetails' support classes.

    Note: Only the #defined classes get defined
    */

    /* ----------------------------------------------------------------- */
    /* DDSDataWriter
    */

    /**
    <<IMPLEMENTATION >>

    Defines:   TDataWriter, TData
    */

    /* Requires */
    #define TTYPENAME   LTMTopicDetailsTYPENAME

    /* Defines */
    #define TDataWriter LTMTopicDetailsDataWriter
    #define TData       ltm_dds::LTMTopicDetails

    #include "dds_cpp/generic/dds_cpp_data_TDataWriter.gen"

    #undef TDataWriter
    #undef TData

    #undef TTYPENAME

    /* ----------------------------------------------------------------- */
    /* DDSDataReader
    */

    /**
    <<IMPLEMENTATION >>

    Defines:   TDataReader, TDataSeq, TData
    */

    /* Requires */
    #define TTYPENAME   LTMTopicDetailsTYPENAME

    /* Defines */
    #define TDataReader LTMTopicDetailsDataReader
    #define TDataSeq    LTMTopicDetailsSeq
    #define TData       ltm_dds::LTMTopicDetails

    #include "dds_cpp/generic/dds_cpp_data_TDataReader.gen"

    #undef TDataReader
    #undef TDataSeq
    #undef TData

    #undef TTYPENAME

    /* ----------------------------------------------------------------- */
    /* TypeSupport

    <<IMPLEMENTATION >>

    Requires:  TTYPENAME,
    TPlugin_new
    TPlugin_delete
    Defines:   TTypeSupport, TData, TDataReader, TDataWriter
    */

    /* Requires */
    #define TTYPENAME    LTMTopicDetailsTYPENAME
    #define TPlugin_new  ltm_dds::LTMTopicDetailsPlugin_new
    #define TPlugin_delete  ltm_dds::LTMTopicDetailsPlugin_delete

    /* Defines */
    #define TTypeSupport LTMTopicDetailsTypeSupport
    #define TData        ltm_dds::LTMTopicDetails
    #define TDataReader  LTMTopicDetailsDataReader
    #define TDataWriter  LTMTopicDetailsDataWriter
    #define TGENERATE_SER_CODE
    #define TGENERATE_TYPECODE

    #include "dds_cpp/generic/dds_cpp_data_TTypeSupport.gen"

    #undef TTypeSupport
    #undef TData
    #undef TDataReader
    #undef TDataWriter
    #undef TGENERATE_TYPECODE
    #undef TGENERATE_SER_CODE
    #undef TTYPENAME
    #undef TPlugin_new
    #undef TPlugin_delete

} /* namespace ltm_dds  */

