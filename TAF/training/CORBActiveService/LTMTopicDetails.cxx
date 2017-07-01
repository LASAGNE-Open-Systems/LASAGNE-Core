

/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from LTMTopicDetails.idl using "rtiddsgen".
The rtiddsgen tool is part of the RTI Connext distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the RTI Connext manual.
*/

#ifndef NDDS_STANDALONE_TYPE
#ifndef ndds_cpp_h
#include "ndds/ndds_cpp.h"
#endif
#ifndef dds_c_log_impl_h              
#include "dds_c/dds_c_log_impl.h"                                
#endif        

#ifndef cdr_type_h
#include "cdr/cdr_type.h"
#endif    

#ifndef osapi_heap_h
#include "osapi/osapi_heap.h" 
#endif
#else
#include "ndds_standalone_type.h"
#endif

#include "LTMTopicDetails.h"

namespace ltm_dds {

    /* ========================================================================= */
    const char *LTMTimestampTYPENAME = "ltm_dds::LTMTimestamp";

    DDS_TypeCode* LTMTimestamp_get_typecode()
    {
        static RTIBool is_initialized = RTI_FALSE;

        static DDS_TypeCode_Member LTMTimestamp_g_tc_members[2]=
        {

            {
                (char *)"sec_",/* Member name */
                {
                    0,/* Representation ID */          
                    DDS_BOOLEAN_FALSE,/* Is a pointer? */
                    -1, /* Bitfield bits */
                    NULL/* Member type code is assigned later */
                },
                0, /* Ignored */
                0, /* Ignored */
                0, /* Ignored */
                NULL, /* Ignored */
                RTI_CDR_REQUIRED_MEMBER, /* Is a key? */
                DDS_PUBLIC_MEMBER,/* Member visibility */
                1,
                NULL/* Ignored */
            }, 
            {
                (char *)"usec_",/* Member name */
                {
                    1,/* Representation ID */          
                    DDS_BOOLEAN_FALSE,/* Is a pointer? */
                    -1, /* Bitfield bits */
                    NULL/* Member type code is assigned later */
                },
                0, /* Ignored */
                0, /* Ignored */
                0, /* Ignored */
                NULL, /* Ignored */
                RTI_CDR_REQUIRED_MEMBER, /* Is a key? */
                DDS_PUBLIC_MEMBER,/* Member visibility */
                1,
                NULL/* Ignored */
            }
        };

        static DDS_TypeCode LTMTimestamp_g_tc =
        {{
                DDS_TK_STRUCT,/* Kind */
                DDS_BOOLEAN_FALSE, /* Ignored */
                -1, /*Ignored*/
                (char *)"ltm_dds::LTMTimestamp", /* Name */
                NULL, /* Ignored */      
                0, /* Ignored */
                0, /* Ignored */
                NULL, /* Ignored */
                2, /* Number of members */
                LTMTimestamp_g_tc_members, /* Members */
                DDS_VM_NONE  /* Ignored */         
            }}; /* Type code for LTMTimestamp*/

        if (is_initialized) {
            return &LTMTimestamp_g_tc;
        }

        LTMTimestamp_g_tc_members[0]._representation._typeCode = (RTICdrTypeCode *)&DDS_g_tc_ulonglong;

        LTMTimestamp_g_tc_members[1]._representation._typeCode = (RTICdrTypeCode *)&DDS_g_tc_ulong;

        is_initialized = RTI_TRUE;

        return &LTMTimestamp_g_tc;
    }

    RTIBool LTMTimestamp_initialize(
        LTMTimestamp* sample) {
        return ltm_dds::LTMTimestamp_initialize_ex(sample,RTI_TRUE,RTI_TRUE);
    }

    RTIBool LTMTimestamp_initialize_ex(
        LTMTimestamp* sample,RTIBool allocatePointers, RTIBool allocateMemory)
    {

        struct DDS_TypeAllocationParams_t allocParams =
        DDS_TYPE_ALLOCATION_PARAMS_DEFAULT;

        allocParams.allocate_pointers =  (DDS_Boolean)allocatePointers;
        allocParams.allocate_memory = (DDS_Boolean)allocateMemory;

        return ltm_dds::LTMTimestamp_initialize_w_params(
            sample,&allocParams);

    }

    RTIBool LTMTimestamp_initialize_w_params(
        LTMTimestamp* sample, const struct DDS_TypeAllocationParams_t * allocParams)
    {

        if (allocParams) {} /* To avoid warnings */

        if (!RTICdrType_initUnsignedLongLong(&sample->sec_)) {
            return RTI_FALSE;
        }     

        if (!RTICdrType_initUnsignedLong(&sample->usec_)) {
            return RTI_FALSE;
        }     

        return RTI_TRUE;
    }

    void LTMTimestamp_finalize(
        LTMTimestamp* sample)
    {

        ltm_dds::LTMTimestamp_finalize_ex(sample,RTI_TRUE);
    }

    void LTMTimestamp_finalize_ex(
        LTMTimestamp* sample,RTIBool deletePointers)
    {
        struct DDS_TypeDeallocationParams_t deallocParams =
        DDS_TYPE_DEALLOCATION_PARAMS_DEFAULT;

        if (sample==NULL) {
            return;
        } 

        deallocParams.delete_pointers = (DDS_Boolean)deletePointers;

        ltm_dds::LTMTimestamp_finalize_w_params(
            sample,&deallocParams);
    }

    void LTMTimestamp_finalize_w_params(
        LTMTimestamp* sample,const struct DDS_TypeDeallocationParams_t * deallocParams)
    {

        if (sample==NULL) {
            return;
        }
        if (deallocParams) {} /* To avoid warnings */

    }

    void LTMTimestamp_finalize_optional_members(
        LTMTimestamp* sample, RTIBool deletePointers)
    {
        struct DDS_TypeDeallocationParams_t deallocParamsTmp =
        DDS_TYPE_DEALLOCATION_PARAMS_DEFAULT;
        struct DDS_TypeDeallocationParams_t * deallocParams =
        &deallocParamsTmp;

        if (sample==NULL) {
            return;
        } 
        if (deallocParams) {} /* To avoid warnings */

        deallocParamsTmp.delete_pointers = (DDS_Boolean)deletePointers;
        deallocParamsTmp.delete_optional_members = DDS_BOOLEAN_TRUE;

    }

    RTIBool LTMTimestamp_copy(
        LTMTimestamp* dst,
        const LTMTimestamp* src)
    {

        if (!RTICdrType_copyUnsignedLongLong (
            &dst->sec_, &src->sec_)) { 
            return RTI_FALSE;
        }
        if (!RTICdrType_copyUnsignedLong (
            &dst->usec_, &src->usec_)) { 
            return RTI_FALSE;
        }

        return RTI_TRUE;
    }

    /**
    * <<IMPLEMENTATION>>
    *
    * Defines:  TSeq, T
    *
    * Configure and implement 'LTMTimestamp' sequence class.
    */
    #define T LTMTimestamp
    #define TSeq LTMTimestampSeq
    #define T_initialize_w_params ltm_dds::LTMTimestamp_initialize_w_params
    #define T_finalize_w_params   ltm_dds::LTMTimestamp_finalize_w_params
    #define T_copy       ltm_dds::LTMTimestamp_copy

    #ifndef NDDS_STANDALONE_TYPE
    #include "dds_c/generic/dds_c_sequence_TSeq.gen"
    #include "dds_cpp/generic/dds_cpp_sequence_TSeq.gen"
    #else
    #include "dds_c_sequence_TSeq.gen"
    #include "dds_cpp_sequence_TSeq.gen"
    #endif

    #undef T_copy
    #undef T_finalize_w_params
    #undef T_initialize_w_params
    #undef TSeq
    #undef T

    /* ========================================================================= */
    const char *LTMTopicDetailsTYPENAME = "ltm_dds::LTMTopicDetails";

    DDS_TypeCode* LTMTopicDetails_get_typecode()
    {
        static RTIBool is_initialized = RTI_FALSE;

        static DDS_TypeCode LTMTopicDetails_g_tc_topicData__string = DDS_INITIALIZE_STRING_TYPECODE((255));
        static DDS_TypeCode_Member LTMTopicDetails_g_tc_members[3]=
        {

            {
                (char *)"topicTime_",/* Member name */
                {
                    0,/* Representation ID */          
                    DDS_BOOLEAN_FALSE,/* Is a pointer? */
                    -1, /* Bitfield bits */
                    NULL/* Member type code is assigned later */
                },
                0, /* Ignored */
                0, /* Ignored */
                0, /* Ignored */
                NULL, /* Ignored */
                RTI_CDR_REQUIRED_MEMBER, /* Is a key? */
                DDS_PUBLIC_MEMBER,/* Member visibility */
                1,
                NULL/* Ignored */
            }, 
            {
                (char *)"topicID_",/* Member name */
                {
                    1,/* Representation ID */          
                    DDS_BOOLEAN_FALSE,/* Is a pointer? */
                    -1, /* Bitfield bits */
                    NULL/* Member type code is assigned later */
                },
                0, /* Ignored */
                0, /* Ignored */
                0, /* Ignored */
                NULL, /* Ignored */
                RTI_CDR_KEY_MEMBER , /* Is a key? */
                DDS_PUBLIC_MEMBER,/* Member visibility */
                1,
                NULL/* Ignored */
            }, 
            {
                (char *)"topicData_",/* Member name */
                {
                    2,/* Representation ID */          
                    DDS_BOOLEAN_FALSE,/* Is a pointer? */
                    -1, /* Bitfield bits */
                    NULL/* Member type code is assigned later */
                },
                0, /* Ignored */
                0, /* Ignored */
                0, /* Ignored */
                NULL, /* Ignored */
                RTI_CDR_REQUIRED_MEMBER, /* Is a key? */
                DDS_PUBLIC_MEMBER,/* Member visibility */
                1,
                NULL/* Ignored */
            }
        };

        static DDS_TypeCode LTMTopicDetails_g_tc =
        {{
                DDS_TK_STRUCT,/* Kind */
                DDS_BOOLEAN_FALSE, /* Ignored */
                -1, /*Ignored*/
                (char *)"ltm_dds::LTMTopicDetails", /* Name */
                NULL, /* Ignored */      
                0, /* Ignored */
                0, /* Ignored */
                NULL, /* Ignored */
                3, /* Number of members */
                LTMTopicDetails_g_tc_members, /* Members */
                DDS_VM_NONE  /* Ignored */         
            }}; /* Type code for LTMTopicDetails*/

        if (is_initialized) {
            return &LTMTopicDetails_g_tc;
        }

        LTMTopicDetails_g_tc_members[0]._representation._typeCode = (RTICdrTypeCode *)ltm_dds::LTMTimestamp_get_typecode();

        LTMTopicDetails_g_tc_members[1]._representation._typeCode = (RTICdrTypeCode *)&DDS_g_tc_ulong;

        LTMTopicDetails_g_tc_members[2]._representation._typeCode = (RTICdrTypeCode *)&LTMTopicDetails_g_tc_topicData__string;

        is_initialized = RTI_TRUE;

        return &LTMTopicDetails_g_tc;
    }

    RTIBool LTMTopicDetails_initialize(
        LTMTopicDetails* sample) {
        return ltm_dds::LTMTopicDetails_initialize_ex(sample,RTI_TRUE,RTI_TRUE);
    }

    RTIBool LTMTopicDetails_initialize_ex(
        LTMTopicDetails* sample,RTIBool allocatePointers, RTIBool allocateMemory)
    {

        struct DDS_TypeAllocationParams_t allocParams =
        DDS_TYPE_ALLOCATION_PARAMS_DEFAULT;

        allocParams.allocate_pointers =  (DDS_Boolean)allocatePointers;
        allocParams.allocate_memory = (DDS_Boolean)allocateMemory;

        return ltm_dds::LTMTopicDetails_initialize_w_params(
            sample,&allocParams);

    }

    RTIBool LTMTopicDetails_initialize_w_params(
        LTMTopicDetails* sample, const struct DDS_TypeAllocationParams_t * allocParams)
    {

        if (allocParams) {} /* To avoid warnings */

        if (!ltm_dds::LTMTimestamp_initialize_w_params(&sample->topicTime_,
        allocParams)) {
            return RTI_FALSE;
        }

        if (!RTICdrType_initUnsignedLong(&sample->topicID_)) {
            return RTI_FALSE;
        }     

        if (allocParams->allocate_memory){
            sample->topicData_= DDS_String_alloc ((255));
            if (sample->topicData_ == NULL) {
                return RTI_FALSE;
            }

        } else {
            if (sample->topicData_!= NULL) { 
                sample->topicData_[0] = '\0';
            }
        }

        return RTI_TRUE;
    }

    void LTMTopicDetails_finalize(
        LTMTopicDetails* sample)
    {

        ltm_dds::LTMTopicDetails_finalize_ex(sample,RTI_TRUE);
    }

    void LTMTopicDetails_finalize_ex(
        LTMTopicDetails* sample,RTIBool deletePointers)
    {
        struct DDS_TypeDeallocationParams_t deallocParams =
        DDS_TYPE_DEALLOCATION_PARAMS_DEFAULT;

        if (sample==NULL) {
            return;
        } 

        deallocParams.delete_pointers = (DDS_Boolean)deletePointers;

        ltm_dds::LTMTopicDetails_finalize_w_params(
            sample,&deallocParams);
    }

    void LTMTopicDetails_finalize_w_params(
        LTMTopicDetails* sample,const struct DDS_TypeDeallocationParams_t * deallocParams)
    {

        if (sample==NULL) {
            return;
        }
        if (deallocParams) {} /* To avoid warnings */

        ltm_dds::LTMTimestamp_finalize_w_params(&sample->topicTime_,deallocParams);

        if (sample->topicData_ != NULL) {
            DDS_String_free(sample->topicData_);
            sample->topicData_=NULL;

        }
    }

    void LTMTopicDetails_finalize_optional_members(
        LTMTopicDetails* sample, RTIBool deletePointers)
    {
        struct DDS_TypeDeallocationParams_t deallocParamsTmp =
        DDS_TYPE_DEALLOCATION_PARAMS_DEFAULT;
        struct DDS_TypeDeallocationParams_t * deallocParams =
        &deallocParamsTmp;

        if (sample==NULL) {
            return;
        } 
        if (deallocParams) {} /* To avoid warnings */

        deallocParamsTmp.delete_pointers = (DDS_Boolean)deletePointers;
        deallocParamsTmp.delete_optional_members = DDS_BOOLEAN_TRUE;

        ltm_dds::LTMTimestamp_finalize_optional_members(&sample->topicTime_, deallocParams->delete_pointers);
    }

    RTIBool LTMTopicDetails_copy(
        LTMTopicDetails* dst,
        const LTMTopicDetails* src)
    {

        if (!ltm_dds::LTMTimestamp_copy(
            &dst->topicTime_, &src->topicTime_)) {
            return RTI_FALSE;
        } 
        if (!RTICdrType_copyUnsignedLong (
            &dst->topicID_, &src->topicID_)) { 
            return RTI_FALSE;
        }
        if (!RTICdrType_copyStringEx (
            &dst->topicData_, src->topicData_, 
            (255) + 1, RTI_FALSE)){
            return RTI_FALSE;
        }

        return RTI_TRUE;
    }

    /**
    * <<IMPLEMENTATION>>
    *
    * Defines:  TSeq, T
    *
    * Configure and implement 'LTMTopicDetails' sequence class.
    */
    #define T LTMTopicDetails
    #define TSeq LTMTopicDetailsSeq
    #define T_initialize_w_params ltm_dds::LTMTopicDetails_initialize_w_params
    #define T_finalize_w_params   ltm_dds::LTMTopicDetails_finalize_w_params
    #define T_copy       ltm_dds::LTMTopicDetails_copy

    #ifndef NDDS_STANDALONE_TYPE
    #include "dds_c/generic/dds_c_sequence_TSeq.gen"
    #include "dds_cpp/generic/dds_cpp_sequence_TSeq.gen"
    #else
    #include "dds_c_sequence_TSeq.gen"
    #include "dds_cpp_sequence_TSeq.gen"
    #endif

    #undef T_copy
    #undef T_finalize_w_params
    #undef T_initialize_w_params
    #undef TSeq
    #undef T

    /* ========================================================================= */

    DDS_TypeCode* LTMTopicDetailsList_get_typecode()
    {
        static RTIBool is_initialized = RTI_FALSE;

        static DDS_TypeCode LTMTopicDetailsList_g_tc_sequence = DDS_INITIALIZE_SEQUENCE_TYPECODE((10),NULL);

        static DDS_TypeCode LTMTopicDetailsList_g_tc =
        {{
                DDS_TK_ALIAS, /* Kind*/
                DDS_BOOLEAN_FALSE,/* Is a pointer? */
                -1, /* Ignored */
                (char *)"ltm_dds::LTMTopicDetailsList", /* Name */
                NULL, /* Content type code is assigned later */
                0, /* Ignored */
                0, /* Ignored */
                NULL, /* Ignored */
                0, /* Ignored */
                NULL, /* Ignored */
                DDS_VM_NONE /* Ignored */
            }}; /* Type code for  LTMTopicDetailsList */

        if (is_initialized) {
            return &LTMTopicDetailsList_g_tc;
        }

        LTMTopicDetailsList_g_tc_sequence._data._typeCode = (RTICdrTypeCode *)ltm_dds::LTMTopicDetails_get_typecode();

        LTMTopicDetailsList_g_tc._data._typeCode =  (RTICdrTypeCode *)& LTMTopicDetailsList_g_tc_sequence;

        is_initialized = RTI_TRUE;

        return &LTMTopicDetailsList_g_tc;
    }

    RTIBool LTMTopicDetailsList_initialize(
        LTMTopicDetailsList* sample) {
        return ltm_dds::LTMTopicDetailsList_initialize_ex(sample,RTI_TRUE,RTI_TRUE);
    }

    RTIBool LTMTopicDetailsList_initialize_ex(
        LTMTopicDetailsList* sample,RTIBool allocatePointers, RTIBool allocateMemory)
    {

        struct DDS_TypeAllocationParams_t allocParams =
        DDS_TYPE_ALLOCATION_PARAMS_DEFAULT;

        allocParams.allocate_pointers =  (DDS_Boolean)allocatePointers;
        allocParams.allocate_memory = (DDS_Boolean)allocateMemory;

        return ltm_dds::LTMTopicDetailsList_initialize_w_params(
            sample,&allocParams);

    }

    RTIBool LTMTopicDetailsList_initialize_w_params(
        LTMTopicDetailsList* sample, const struct DDS_TypeAllocationParams_t * allocParams)
    {

        void* buffer = NULL;
        if (buffer) {} /* To avoid warnings */

        if (allocParams) {} /* To avoid warnings */

        if (allocParams->allocate_memory) {
            ltm_dds::LTMTopicDetailsSeq_initialize(sample );
            ltm_dds::LTMTopicDetailsSeq_set_element_allocation_params(sample ,allocParams);
            if (!ltm_dds::LTMTopicDetailsSeq_set_maximum(sample, (10))) {
                return RTI_FALSE;
            }
        } else { 
            ltm_dds::LTMTopicDetailsSeq_set_length(sample, 0);
        }
        return RTI_TRUE;
    }

    void LTMTopicDetailsList_finalize(
        LTMTopicDetailsList* sample)
    {

        ltm_dds::LTMTopicDetailsList_finalize_ex(sample,RTI_TRUE);
    }

    void LTMTopicDetailsList_finalize_ex(
        LTMTopicDetailsList* sample,RTIBool deletePointers)
    {
        struct DDS_TypeDeallocationParams_t deallocParams =
        DDS_TYPE_DEALLOCATION_PARAMS_DEFAULT;

        if (sample==NULL) {
            return;
        } 

        deallocParams.delete_pointers = (DDS_Boolean)deletePointers;

        ltm_dds::LTMTopicDetailsList_finalize_w_params(
            sample,&deallocParams);
    }

    void LTMTopicDetailsList_finalize_w_params(
        LTMTopicDetailsList* sample,const struct DDS_TypeDeallocationParams_t * deallocParams)
    {

        if (sample==NULL) {
            return;
        }
        if (deallocParams) {} /* To avoid warnings */

        ltm_dds::LTMTopicDetailsSeq_set_element_deallocation_params(
            sample,deallocParams);
        ltm_dds::LTMTopicDetailsSeq_finalize(sample);

    }

    void LTMTopicDetailsList_finalize_optional_members(
        LTMTopicDetailsList* sample, RTIBool deletePointers)
    {
        struct DDS_TypeDeallocationParams_t deallocParamsTmp =
        DDS_TYPE_DEALLOCATION_PARAMS_DEFAULT;
        struct DDS_TypeDeallocationParams_t * deallocParams =
        &deallocParamsTmp;

        if (sample==NULL) {
            return;
        } 
        if (deallocParams) {} /* To avoid warnings */

        deallocParamsTmp.delete_pointers = (DDS_Boolean)deletePointers;
        deallocParamsTmp.delete_optional_members = DDS_BOOLEAN_TRUE;

        {
            DDS_UnsignedLong i, length;
            length = ltm_dds::LTMTopicDetailsSeq_get_length(
                sample);

            for (i = 0; i < length; i++) {
                ltm_dds::LTMTopicDetails_finalize_optional_members(
                    ltm_dds::LTMTopicDetailsSeq_get_reference(
                        sample, i), deallocParams->delete_pointers);
            }
        }  

    }

    RTIBool LTMTopicDetailsList_copy(
        LTMTopicDetailsList* dst,
        const LTMTopicDetailsList* src)
    {

        if (!ltm_dds::LTMTopicDetailsSeq_copy(dst ,
        src )) {
            return RTI_FALSE;
        }

        return RTI_TRUE;
    }

    /**
    * <<IMPLEMENTATION>>
    *
    * Defines:  TSeq, T
    *
    * Configure and implement 'LTMTopicDetailsList' sequence class.
    */
    #define T LTMTopicDetailsList
    #define TSeq LTMTopicDetailsListSeq
    #define T_initialize_w_params ltm_dds::LTMTopicDetailsList_initialize_w_params
    #define T_finalize_w_params   ltm_dds::LTMTopicDetailsList_finalize_w_params
    #define T_copy       ltm_dds::LTMTopicDetailsList_copy

    #ifndef NDDS_STANDALONE_TYPE
    #include "dds_c/generic/dds_c_sequence_TSeq.gen"
    #include "dds_cpp/generic/dds_cpp_sequence_TSeq.gen"
    #else
    #include "dds_c_sequence_TSeq.gen"
    #include "dds_cpp_sequence_TSeq.gen"
    #endif

    #undef T_copy
    #undef T_finalize_w_params
    #undef T_initialize_w_params
    #undef TSeq
    #undef T
} /* namespace ltm_dds  */

