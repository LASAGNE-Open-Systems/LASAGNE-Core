

/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from LTMTopicDetails.idl using "rtiddsgen".
The rtiddsgen tool is part of the RTI Connext distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the RTI Connext manual.
*/

#ifndef LTMTopicDetailsPlugin_1412816756_h
#define LTMTopicDetailsPlugin_1412816756_h

#include "LTMTopicDetails.h"

struct RTICdrStream;

#ifndef pres_typePlugin_h
#include "pres/pres_typePlugin.h"
#endif

#if (defined(RTI_WIN32) || defined (RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, start exporting symbols.
*/
#undef NDDSUSERDllExport
#define NDDSUSERDllExport __declspec(dllexport)
#endif

namespace ltm_dds {

    #define LTMTimestampPlugin_get_sample PRESTypePluginDefaultEndpointData_getSample 
    #define LTMTimestampPlugin_get_buffer PRESTypePluginDefaultEndpointData_getBuffer 
    #define LTMTimestampPlugin_return_buffer PRESTypePluginDefaultEndpointData_returnBuffer 

    #define LTMTimestampPlugin_create_sample PRESTypePluginDefaultEndpointData_createSample 
    #define LTMTimestampPlugin_destroy_sample PRESTypePluginDefaultEndpointData_deleteSample 

    /* --------------------------------------------------------------------------------------
    Support functions:
    * -------------------------------------------------------------------------------------- */

    NDDSUSERDllExport extern LTMTimestamp*
    LTMTimestampPluginSupport_create_data_w_params(
        const struct DDS_TypeAllocationParams_t * alloc_params);

    NDDSUSERDllExport extern LTMTimestamp*
    LTMTimestampPluginSupport_create_data_ex(RTIBool allocate_pointers);

    NDDSUSERDllExport extern LTMTimestamp*
    LTMTimestampPluginSupport_create_data(void);

    NDDSUSERDllExport extern RTIBool 
    LTMTimestampPluginSupport_copy_data(
        LTMTimestamp *out,
        const LTMTimestamp *in);

    NDDSUSERDllExport extern void 
    LTMTimestampPluginSupport_destroy_data_w_params(
        LTMTimestamp *sample,
        const struct DDS_TypeDeallocationParams_t * dealloc_params);

    NDDSUSERDllExport extern void 
    LTMTimestampPluginSupport_destroy_data_ex(
        LTMTimestamp *sample,RTIBool deallocate_pointers);

    NDDSUSERDllExport extern void 
    LTMTimestampPluginSupport_destroy_data(
        LTMTimestamp *sample);

    NDDSUSERDllExport extern void 
    LTMTimestampPluginSupport_print_data(
        const LTMTimestamp *sample,
        const char *desc,
        unsigned int indent);

    /* ----------------------------------------------------------------------------
    Callback functions:
    * ---------------------------------------------------------------------------- */

    NDDSUSERDllExport extern PRESTypePluginParticipantData 
    LTMTimestampPlugin_on_participant_attached(
        void *registration_data, 
        const struct PRESTypePluginParticipantInfo *participant_info,
        RTIBool top_level_registration, 
        void *container_plugin_context,
        RTICdrTypeCode *typeCode);

    NDDSUSERDllExport extern void 
    LTMTimestampPlugin_on_participant_detached(
        PRESTypePluginParticipantData participant_data);

    NDDSUSERDllExport extern PRESTypePluginEndpointData 
    LTMTimestampPlugin_on_endpoint_attached(
        PRESTypePluginParticipantData participant_data,
        const struct PRESTypePluginEndpointInfo *endpoint_info,
        RTIBool top_level_registration, 
        void *container_plugin_context);

    NDDSUSERDllExport extern void 
    LTMTimestampPlugin_on_endpoint_detached(
        PRESTypePluginEndpointData endpoint_data);

    NDDSUSERDllExport extern void    
    LTMTimestampPlugin_return_sample(
        PRESTypePluginEndpointData endpoint_data,
        LTMTimestamp *sample,
        void *handle);    

    NDDSUSERDllExport extern RTIBool 
    LTMTimestampPlugin_copy_sample(
        PRESTypePluginEndpointData endpoint_data,
        LTMTimestamp *out,
        const LTMTimestamp *in);

    /* ----------------------------------------------------------------------------
    (De)Serialize functions:
    * ------------------------------------------------------------------------- */

    NDDSUSERDllExport extern RTIBool 
    LTMTimestampPlugin_serialize(
        PRESTypePluginEndpointData endpoint_data,
        const LTMTimestamp *sample,
        struct RTICdrStream *stream, 
        RTIBool serialize_encapsulation,
        RTIEncapsulationId encapsulation_id,
        RTIBool serialize_sample, 
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern RTIBool 
    LTMTimestampPlugin_deserialize_sample(
        PRESTypePluginEndpointData endpoint_data,
        LTMTimestamp *sample, 
        struct RTICdrStream *stream,
        RTIBool deserialize_encapsulation,
        RTIBool deserialize_sample, 
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern RTIBool
    LTMTimestampPlugin_serialize_to_cdr_buffer(
        char * buffer,
        unsigned int * length,
        const LTMTimestamp *sample); 

    NDDSUSERDllExport extern RTIBool 
    LTMTimestampPlugin_deserialize(
        PRESTypePluginEndpointData endpoint_data,
        LTMTimestamp **sample, 
        RTIBool * drop_sample,
        struct RTICdrStream *stream,
        RTIBool deserialize_encapsulation,
        RTIBool deserialize_sample, 
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern RTIBool
    LTMTimestampPlugin_deserialize_from_cdr_buffer(
        LTMTimestamp *sample,
        const char * buffer,
        unsigned int length);    

    NDDSUSERDllExport extern RTIBool
    LTMTimestampPlugin_skip(
        PRESTypePluginEndpointData endpoint_data,
        struct RTICdrStream *stream, 
        RTIBool skip_encapsulation,  
        RTIBool skip_sample, 
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern unsigned int 
    LTMTimestampPlugin_get_serialized_sample_max_size_ex(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool * overflow,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment);    

    NDDSUSERDllExport extern unsigned int 
    LTMTimestampPlugin_get_serialized_sample_max_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment);

    NDDSUSERDllExport extern unsigned int 
    LTMTimestampPlugin_get_serialized_sample_min_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment);

    NDDSUSERDllExport extern unsigned int
    LTMTimestampPlugin_get_serialized_sample_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment,
        const LTMTimestamp * sample);

    /* --------------------------------------------------------------------------------------
    Key Management functions:
    * -------------------------------------------------------------------------------------- */
    NDDSUSERDllExport extern PRESTypePluginKeyKind 
    LTMTimestampPlugin_get_key_kind(void);

    NDDSUSERDllExport extern unsigned int 
    LTMTimestampPlugin_get_serialized_key_max_size_ex(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool * overflow,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment);

    NDDSUSERDllExport extern unsigned int 
    LTMTimestampPlugin_get_serialized_key_max_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment);

    NDDSUSERDllExport extern RTIBool 
    LTMTimestampPlugin_serialize_key(
        PRESTypePluginEndpointData endpoint_data,
        const LTMTimestamp *sample,
        struct RTICdrStream *stream,
        RTIBool serialize_encapsulation,
        RTIEncapsulationId encapsulation_id,
        RTIBool serialize_key,
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern RTIBool 
    LTMTimestampPlugin_deserialize_key_sample(
        PRESTypePluginEndpointData endpoint_data,
        LTMTimestamp * sample,
        struct RTICdrStream *stream,
        RTIBool deserialize_encapsulation,
        RTIBool deserialize_key,
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern RTIBool 
    LTMTimestampPlugin_deserialize_key(
        PRESTypePluginEndpointData endpoint_data,
        LTMTimestamp ** sample,
        RTIBool * drop_sample,
        struct RTICdrStream *stream,
        RTIBool deserialize_encapsulation,
        RTIBool deserialize_key,
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern RTIBool
    LTMTimestampPlugin_serialized_sample_to_key(
        PRESTypePluginEndpointData endpoint_data,
        LTMTimestamp *sample,
        struct RTICdrStream *stream, 
        RTIBool deserialize_encapsulation,  
        RTIBool deserialize_key, 
        void *endpoint_plugin_qos);

    /* Plugin Functions */
    NDDSUSERDllExport extern struct PRESTypePlugin*
    LTMTimestampPlugin_new(void);

    NDDSUSERDllExport extern void
    LTMTimestampPlugin_delete(struct PRESTypePlugin *);

    /* The type used to store keys for instances of type struct
    * AnotherSimple.
    *
    * By default, this type is struct LTMTopicDetails
    * itself. However, if for some reason this choice is not practical for your
    * system (e.g. if sizeof(struct LTMTopicDetails)
    * is very large), you may redefine this typedef in terms of another type of
    * your choosing. HOWEVER, if you define the KeyHolder type to be something
    * other than struct AnotherSimple, the
    * following restriction applies: the key of struct
    * LTMTopicDetails must consist of a
    * single field of your redefined KeyHolder type and that field must be the
    * first field in struct LTMTopicDetails.
    */
    typedef  class LTMTopicDetails LTMTopicDetailsKeyHolder;

    #define LTMTopicDetailsPlugin_get_sample PRESTypePluginDefaultEndpointData_getSample 
    #define LTMTopicDetailsPlugin_get_buffer PRESTypePluginDefaultEndpointData_getBuffer 
    #define LTMTopicDetailsPlugin_return_buffer PRESTypePluginDefaultEndpointData_returnBuffer 

    #define LTMTopicDetailsPlugin_get_key PRESTypePluginDefaultEndpointData_getKey 
    #define LTMTopicDetailsPlugin_return_key PRESTypePluginDefaultEndpointData_returnKey

    #define LTMTopicDetailsPlugin_create_sample PRESTypePluginDefaultEndpointData_createSample 
    #define LTMTopicDetailsPlugin_destroy_sample PRESTypePluginDefaultEndpointData_deleteSample 

    /* --------------------------------------------------------------------------------------
    Support functions:
    * -------------------------------------------------------------------------------------- */

    NDDSUSERDllExport extern LTMTopicDetails*
    LTMTopicDetailsPluginSupport_create_data_w_params(
        const struct DDS_TypeAllocationParams_t * alloc_params);

    NDDSUSERDllExport extern LTMTopicDetails*
    LTMTopicDetailsPluginSupport_create_data_ex(RTIBool allocate_pointers);

    NDDSUSERDllExport extern LTMTopicDetails*
    LTMTopicDetailsPluginSupport_create_data(void);

    NDDSUSERDllExport extern RTIBool 
    LTMTopicDetailsPluginSupport_copy_data(
        LTMTopicDetails *out,
        const LTMTopicDetails *in);

    NDDSUSERDllExport extern void 
    LTMTopicDetailsPluginSupport_destroy_data_w_params(
        LTMTopicDetails *sample,
        const struct DDS_TypeDeallocationParams_t * dealloc_params);

    NDDSUSERDllExport extern void 
    LTMTopicDetailsPluginSupport_destroy_data_ex(
        LTMTopicDetails *sample,RTIBool deallocate_pointers);

    NDDSUSERDllExport extern void 
    LTMTopicDetailsPluginSupport_destroy_data(
        LTMTopicDetails *sample);

    NDDSUSERDllExport extern void 
    LTMTopicDetailsPluginSupport_print_data(
        const LTMTopicDetails *sample,
        const char *desc,
        unsigned int indent);

    NDDSUSERDllExport extern LTMTopicDetails*
    LTMTopicDetailsPluginSupport_create_key_ex(RTIBool allocate_pointers);

    NDDSUSERDllExport extern LTMTopicDetails*
    LTMTopicDetailsPluginSupport_create_key(void);

    NDDSUSERDllExport extern void 
    LTMTopicDetailsPluginSupport_destroy_key_ex(
        LTMTopicDetailsKeyHolder *key,RTIBool deallocate_pointers);

    NDDSUSERDllExport extern void 
    LTMTopicDetailsPluginSupport_destroy_key(
        LTMTopicDetailsKeyHolder *key);

    /* ----------------------------------------------------------------------------
    Callback functions:
    * ---------------------------------------------------------------------------- */

    NDDSUSERDllExport extern PRESTypePluginParticipantData 
    LTMTopicDetailsPlugin_on_participant_attached(
        void *registration_data, 
        const struct PRESTypePluginParticipantInfo *participant_info,
        RTIBool top_level_registration, 
        void *container_plugin_context,
        RTICdrTypeCode *typeCode);

    NDDSUSERDllExport extern void 
    LTMTopicDetailsPlugin_on_participant_detached(
        PRESTypePluginParticipantData participant_data);

    NDDSUSERDllExport extern PRESTypePluginEndpointData 
    LTMTopicDetailsPlugin_on_endpoint_attached(
        PRESTypePluginParticipantData participant_data,
        const struct PRESTypePluginEndpointInfo *endpoint_info,
        RTIBool top_level_registration, 
        void *container_plugin_context);

    NDDSUSERDllExport extern void 
    LTMTopicDetailsPlugin_on_endpoint_detached(
        PRESTypePluginEndpointData endpoint_data);

    NDDSUSERDllExport extern void    
    LTMTopicDetailsPlugin_return_sample(
        PRESTypePluginEndpointData endpoint_data,
        LTMTopicDetails *sample,
        void *handle);    

    NDDSUSERDllExport extern RTIBool 
    LTMTopicDetailsPlugin_copy_sample(
        PRESTypePluginEndpointData endpoint_data,
        LTMTopicDetails *out,
        const LTMTopicDetails *in);

    /* ----------------------------------------------------------------------------
    (De)Serialize functions:
    * ------------------------------------------------------------------------- */

    NDDSUSERDllExport extern RTIBool 
    LTMTopicDetailsPlugin_serialize(
        PRESTypePluginEndpointData endpoint_data,
        const LTMTopicDetails *sample,
        struct RTICdrStream *stream, 
        RTIBool serialize_encapsulation,
        RTIEncapsulationId encapsulation_id,
        RTIBool serialize_sample, 
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern RTIBool 
    LTMTopicDetailsPlugin_deserialize_sample(
        PRESTypePluginEndpointData endpoint_data,
        LTMTopicDetails *sample, 
        struct RTICdrStream *stream,
        RTIBool deserialize_encapsulation,
        RTIBool deserialize_sample, 
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern RTIBool
    LTMTopicDetailsPlugin_serialize_to_cdr_buffer(
        char * buffer,
        unsigned int * length,
        const LTMTopicDetails *sample); 

    NDDSUSERDllExport extern RTIBool 
    LTMTopicDetailsPlugin_deserialize(
        PRESTypePluginEndpointData endpoint_data,
        LTMTopicDetails **sample, 
        RTIBool * drop_sample,
        struct RTICdrStream *stream,
        RTIBool deserialize_encapsulation,
        RTIBool deserialize_sample, 
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern RTIBool
    LTMTopicDetailsPlugin_deserialize_from_cdr_buffer(
        LTMTopicDetails *sample,
        const char * buffer,
        unsigned int length);    

    NDDSUSERDllExport extern RTIBool
    LTMTopicDetailsPlugin_skip(
        PRESTypePluginEndpointData endpoint_data,
        struct RTICdrStream *stream, 
        RTIBool skip_encapsulation,  
        RTIBool skip_sample, 
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern unsigned int 
    LTMTopicDetailsPlugin_get_serialized_sample_max_size_ex(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool * overflow,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment);    

    NDDSUSERDllExport extern unsigned int 
    LTMTopicDetailsPlugin_get_serialized_sample_max_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment);

    NDDSUSERDllExport extern unsigned int 
    LTMTopicDetailsPlugin_get_serialized_sample_min_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment);

    NDDSUSERDllExport extern unsigned int
    LTMTopicDetailsPlugin_get_serialized_sample_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment,
        const LTMTopicDetails * sample);

    /* --------------------------------------------------------------------------------------
    Key Management functions:
    * -------------------------------------------------------------------------------------- */
    NDDSUSERDllExport extern PRESTypePluginKeyKind 
    LTMTopicDetailsPlugin_get_key_kind(void);

    NDDSUSERDllExport extern unsigned int 
    LTMTopicDetailsPlugin_get_serialized_key_max_size_ex(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool * overflow,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment);

    NDDSUSERDllExport extern unsigned int 
    LTMTopicDetailsPlugin_get_serialized_key_max_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment);

    NDDSUSERDllExport extern RTIBool 
    LTMTopicDetailsPlugin_serialize_key(
        PRESTypePluginEndpointData endpoint_data,
        const LTMTopicDetails *sample,
        struct RTICdrStream *stream,
        RTIBool serialize_encapsulation,
        RTIEncapsulationId encapsulation_id,
        RTIBool serialize_key,
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern RTIBool 
    LTMTopicDetailsPlugin_deserialize_key_sample(
        PRESTypePluginEndpointData endpoint_data,
        LTMTopicDetails * sample,
        struct RTICdrStream *stream,
        RTIBool deserialize_encapsulation,
        RTIBool deserialize_key,
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern RTIBool 
    LTMTopicDetailsPlugin_deserialize_key(
        PRESTypePluginEndpointData endpoint_data,
        LTMTopicDetails ** sample,
        RTIBool * drop_sample,
        struct RTICdrStream *stream,
        RTIBool deserialize_encapsulation,
        RTIBool deserialize_key,
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern RTIBool
    LTMTopicDetailsPlugin_serialized_sample_to_key(
        PRESTypePluginEndpointData endpoint_data,
        LTMTopicDetails *sample,
        struct RTICdrStream *stream, 
        RTIBool deserialize_encapsulation,  
        RTIBool deserialize_key, 
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern RTIBool 
    LTMTopicDetailsPlugin_instance_to_key(
        PRESTypePluginEndpointData endpoint_data,
        LTMTopicDetailsKeyHolder *key, 
        const LTMTopicDetails *instance);

    NDDSUSERDllExport extern RTIBool 
    LTMTopicDetailsPlugin_key_to_instance(
        PRESTypePluginEndpointData endpoint_data,
        LTMTopicDetails *instance, 
        const LTMTopicDetailsKeyHolder *key);

    NDDSUSERDllExport extern RTIBool 
    LTMTopicDetailsPlugin_instance_to_keyhash(
        PRESTypePluginEndpointData endpoint_data,
        DDS_KeyHash_t *keyhash,
        const LTMTopicDetails *instance);

    NDDSUSERDllExport extern RTIBool 
    LTMTopicDetailsPlugin_serialized_sample_to_keyhash(
        PRESTypePluginEndpointData endpoint_data,
        struct RTICdrStream *stream, 
        DDS_KeyHash_t *keyhash,
        RTIBool deserialize_encapsulation,
        void *endpoint_plugin_qos); 

    /* Plugin Functions */
    NDDSUSERDllExport extern struct PRESTypePlugin*
    LTMTopicDetailsPlugin_new(void);

    NDDSUSERDllExport extern void
    LTMTopicDetailsPlugin_delete(struct PRESTypePlugin *);

    #define LTMTopicDetailsListPlugin_get_sample PRESTypePluginDefaultEndpointData_getSample 
    #define LTMTopicDetailsListPlugin_get_buffer PRESTypePluginDefaultEndpointData_getBuffer 
    #define LTMTopicDetailsListPlugin_return_buffer PRESTypePluginDefaultEndpointData_returnBuffer 

    #define LTMTopicDetailsListPlugin_create_sample PRESTypePluginDefaultEndpointData_createSample 
    #define LTMTopicDetailsListPlugin_destroy_sample PRESTypePluginDefaultEndpointData_deleteSample 

    /* --------------------------------------------------------------------------------------
    Support functions:
    * -------------------------------------------------------------------------------------- */

    NDDSUSERDllExport extern LTMTopicDetailsList*
    LTMTopicDetailsListPluginSupport_create_data_w_params(
        const struct DDS_TypeAllocationParams_t * alloc_params);

    NDDSUSERDllExport extern LTMTopicDetailsList*
    LTMTopicDetailsListPluginSupport_create_data_ex(RTIBool allocate_pointers);

    NDDSUSERDllExport extern LTMTopicDetailsList*
    LTMTopicDetailsListPluginSupport_create_data(void);

    NDDSUSERDllExport extern RTIBool 
    LTMTopicDetailsListPluginSupport_copy_data(
        LTMTopicDetailsList *out,
        const LTMTopicDetailsList *in);

    NDDSUSERDllExport extern void 
    LTMTopicDetailsListPluginSupport_destroy_data_w_params(
        LTMTopicDetailsList *sample,
        const struct DDS_TypeDeallocationParams_t * dealloc_params);

    NDDSUSERDllExport extern void 
    LTMTopicDetailsListPluginSupport_destroy_data_ex(
        LTMTopicDetailsList *sample,RTIBool deallocate_pointers);

    NDDSUSERDllExport extern void 
    LTMTopicDetailsListPluginSupport_destroy_data(
        LTMTopicDetailsList *sample);

    NDDSUSERDllExport extern void 
    LTMTopicDetailsListPluginSupport_print_data(
        const LTMTopicDetailsList *sample,
        const char *desc,
        unsigned int indent);

    /* ----------------------------------------------------------------------------
    Callback functions:
    * ---------------------------------------------------------------------------- */

    NDDSUSERDllExport extern RTIBool 
    LTMTopicDetailsListPlugin_copy_sample(
        PRESTypePluginEndpointData endpoint_data,
        LTMTopicDetailsList *out,
        const LTMTopicDetailsList *in);

    /* ----------------------------------------------------------------------------
    (De)Serialize functions:
    * ------------------------------------------------------------------------- */

    NDDSUSERDllExport extern RTIBool 
    LTMTopicDetailsListPlugin_serialize(
        PRESTypePluginEndpointData endpoint_data,
        const LTMTopicDetailsList *sample,
        struct RTICdrStream *stream, 
        RTIBool serialize_encapsulation,
        RTIEncapsulationId encapsulation_id,
        RTIBool serialize_sample, 
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern RTIBool 
    LTMTopicDetailsListPlugin_deserialize_sample(
        PRESTypePluginEndpointData endpoint_data,
        LTMTopicDetailsList *sample, 
        struct RTICdrStream *stream,
        RTIBool deserialize_encapsulation,
        RTIBool deserialize_sample, 
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern RTIBool
    LTMTopicDetailsListPlugin_skip(
        PRESTypePluginEndpointData endpoint_data,
        struct RTICdrStream *stream, 
        RTIBool skip_encapsulation,  
        RTIBool skip_sample, 
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern unsigned int 
    LTMTopicDetailsListPlugin_get_serialized_sample_max_size_ex(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool * overflow,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment);    

    NDDSUSERDllExport extern unsigned int 
    LTMTopicDetailsListPlugin_get_serialized_sample_max_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment);

    NDDSUSERDllExport extern unsigned int 
    LTMTopicDetailsListPlugin_get_serialized_sample_min_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment);

    NDDSUSERDllExport extern unsigned int
    LTMTopicDetailsListPlugin_get_serialized_sample_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment,
        const LTMTopicDetailsList * sample);

    /* --------------------------------------------------------------------------------------
    Key Management functions:
    * -------------------------------------------------------------------------------------- */
    NDDSUSERDllExport extern PRESTypePluginKeyKind 
    LTMTopicDetailsListPlugin_get_key_kind(void);

    NDDSUSERDllExport extern unsigned int 
    LTMTopicDetailsListPlugin_get_serialized_key_max_size_ex(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool * overflow,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment);

    NDDSUSERDllExport extern unsigned int 
    LTMTopicDetailsListPlugin_get_serialized_key_max_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment);

    NDDSUSERDllExport extern RTIBool 
    LTMTopicDetailsListPlugin_serialize_key(
        PRESTypePluginEndpointData endpoint_data,
        const LTMTopicDetailsList *sample,
        struct RTICdrStream *stream,
        RTIBool serialize_encapsulation,
        RTIEncapsulationId encapsulation_id,
        RTIBool serialize_key,
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern RTIBool 
    LTMTopicDetailsListPlugin_deserialize_key_sample(
        PRESTypePluginEndpointData endpoint_data,
        LTMTopicDetailsList * sample,
        struct RTICdrStream *stream,
        RTIBool deserialize_encapsulation,
        RTIBool deserialize_key,
        void *endpoint_plugin_qos);

    NDDSUSERDllExport extern RTIBool
    LTMTopicDetailsListPlugin_serialized_sample_to_key(
        PRESTypePluginEndpointData endpoint_data,
        LTMTopicDetailsList *sample,
        struct RTICdrStream *stream, 
        RTIBool deserialize_encapsulation,  
        RTIBool deserialize_key, 
        void *endpoint_plugin_qos);

} /* namespace ltm_dds  */

#if (defined(RTI_WIN32) || defined (RTI_WINCE)) && defined(NDDS_USER_DLL_EXPORT)
/* If the code is building on Windows, stop exporting symbols.
*/
#undef NDDSUSERDllExport
#define NDDSUSERDllExport
#endif

#endif /* LTMTopicDetailsPlugin_1412816756_h */

