

/*
WARNING: THIS FILE IS AUTO-GENERATED. DO NOT MODIFY.

This file was generated from LTMTopicDetails.idl using "rtiddsgen".
The rtiddsgen tool is part of the RTI Connext distribution.
For more information, type 'rtiddsgen -help' at a command shell
or consult the RTI Connext manual.
*/

#include <string.h>

#ifndef ndds_cpp_h
#include "ndds/ndds_cpp.h"
#endif

#ifndef osapi_type_h
#include "osapi/osapi_type.h"
#endif
#ifndef osapi_heap_h
#include "osapi/osapi_heap.h"
#endif

#ifndef osapi_utility_h
#include "osapi/osapi_utility.h"
#endif

#ifndef cdr_type_h
#include "cdr/cdr_type.h"
#endif

#ifndef cdr_type_object_h
#include "cdr/cdr_typeObject.h"
#endif

#ifndef cdr_encapsulation_h
#include "cdr/cdr_encapsulation.h"
#endif

#ifndef cdr_stream_h
#include "cdr/cdr_stream.h"
#endif

#ifndef pres_typePlugin_h
#include "pres/pres_typePlugin.h"
#endif

#include "LTMTopicDetailsPlugin.h"

namespace ltm_dds {

    /* ----------------------------------------------------------------------------
    *  Type LTMTimestamp
    * -------------------------------------------------------------------------- */

    /* -----------------------------------------------------------------------------
    Support functions:
    * -------------------------------------------------------------------------- */

    LTMTimestamp*
    LTMTimestampPluginSupport_create_data_w_params(
        const struct DDS_TypeAllocationParams_t * alloc_params){
        LTMTimestamp *sample = NULL;

        RTIOsapiHeap_allocateStructure(
            &sample, LTMTimestamp);

        if(sample != NULL) {
            if (!ltm_dds::LTMTimestamp_initialize_w_params(sample,alloc_params)) {
                RTIOsapiHeap_freeStructure(sample);
                return NULL;
            }
        }        
        return sample; 
    } 

    LTMTimestamp *
    LTMTimestampPluginSupport_create_data_ex(RTIBool allocate_pointers){
        LTMTimestamp *sample = NULL;

        RTIOsapiHeap_allocateStructure(
            &sample, LTMTimestamp);

        if(sample != NULL) {
            if (!ltm_dds::LTMTimestamp_initialize_ex(sample,allocate_pointers, RTI_TRUE)) {
                RTIOsapiHeap_freeStructure(sample);
                return NULL;
            }
        }
        return sample; 
    }

    LTMTimestamp *
    LTMTimestampPluginSupport_create_data(void)
    {
        return ltm_dds::LTMTimestampPluginSupport_create_data_ex(RTI_TRUE);
    }

    void 
    LTMTimestampPluginSupport_destroy_data_w_params(
        LTMTimestamp *sample,
        const struct DDS_TypeDeallocationParams_t * dealloc_params) {

        ltm_dds::LTMTimestamp_finalize_w_params(sample,dealloc_params);

        RTIOsapiHeap_freeStructure(sample);
    }

    void 
    LTMTimestampPluginSupport_destroy_data_ex(
        LTMTimestamp *sample,RTIBool deallocate_pointers) {

        ltm_dds::LTMTimestamp_finalize_ex(sample,deallocate_pointers);

        RTIOsapiHeap_freeStructure(sample);
    }

    void 
    LTMTimestampPluginSupport_destroy_data(
        LTMTimestamp *sample) {

        ltm_dds::LTMTimestampPluginSupport_destroy_data_ex(sample,RTI_TRUE);

    }

    RTIBool 
    LTMTimestampPluginSupport_copy_data(
        LTMTimestamp *dst,
        const LTMTimestamp *src)
    {
        return ltm_dds::LTMTimestamp_copy(dst,src);
    }

    void 
    LTMTimestampPluginSupport_print_data(
        const LTMTimestamp *sample,
        const char *desc,
        unsigned int indent_level)
    {

        RTICdrType_printIndent(indent_level);

        if (desc != NULL) {
            RTILog_debug("%s:\n", desc);
        } else {
            RTILog_debug("\n");
        }

        if (sample == NULL) {
            RTILog_debug("NULL\n");
            return;
        }

        RTICdrType_printUnsignedLongLong(
            &sample->sec_, "sec_", indent_level + 1);    

        RTICdrType_printUnsignedLong(
            &sample->usec_, "usec_", indent_level + 1);    

    }

    /* ----------------------------------------------------------------------------
    Callback functions:
    * ---------------------------------------------------------------------------- */

    PRESTypePluginParticipantData 
    LTMTimestampPlugin_on_participant_attached(
        void *registration_data,
        const struct PRESTypePluginParticipantInfo *participant_info,
        RTIBool top_level_registration,
        void *container_plugin_context,
        RTICdrTypeCode *type_code)
    {
        if (registration_data) {} /* To avoid warnings */
        if (participant_info) {} /* To avoid warnings */
        if (top_level_registration) {} /* To avoid warnings */
        if (container_plugin_context) {} /* To avoid warnings */
        if (type_code) {} /* To avoid warnings */

        return PRESTypePluginDefaultParticipantData_new(participant_info);

    }

    void 
    LTMTimestampPlugin_on_participant_detached(
        PRESTypePluginParticipantData participant_data)
    {

        PRESTypePluginDefaultParticipantData_delete(participant_data);
    }

    PRESTypePluginEndpointData
    LTMTimestampPlugin_on_endpoint_attached(
        PRESTypePluginParticipantData participant_data,
        const struct PRESTypePluginEndpointInfo *endpoint_info,
        RTIBool top_level_registration, 
        void *containerPluginContext)
    {
        PRESTypePluginEndpointData epd = NULL;

        unsigned int serializedSampleMaxSize;

        if (top_level_registration) {} /* To avoid warnings */
        if (containerPluginContext) {} /* To avoid warnings */

        epd = PRESTypePluginDefaultEndpointData_new(
            participant_data,
            endpoint_info,
            (PRESTypePluginDefaultEndpointDataCreateSampleFunction)
            ltm_dds::LTMTimestampPluginSupport_create_data,
            (PRESTypePluginDefaultEndpointDataDestroySampleFunction)
            ltm_dds::LTMTimestampPluginSupport_destroy_data,
            NULL , NULL );

        if (epd == NULL) {
            return NULL;
        } 

        if (endpoint_info->endpointKind == PRES_TYPEPLUGIN_ENDPOINT_WRITER) {
            serializedSampleMaxSize = ltm_dds::LTMTimestampPlugin_get_serialized_sample_max_size(
                epd,RTI_FALSE,RTI_CDR_ENCAPSULATION_ID_CDR_BE,0);

            PRESTypePluginDefaultEndpointData_setMaxSizeSerializedSample(epd, serializedSampleMaxSize);

            if (PRESTypePluginDefaultEndpointData_createWriterPool(
                epd,
                endpoint_info,
                (PRESTypePluginGetSerializedSampleMaxSizeFunction)
                ltm_dds::LTMTimestampPlugin_get_serialized_sample_max_size, epd,
                (PRESTypePluginGetSerializedSampleSizeFunction)
                ltm_dds::LTMTimestampPlugin_get_serialized_sample_size,
                epd) == RTI_FALSE) {
                PRESTypePluginDefaultEndpointData_delete(epd);
                return NULL;
            }
        }

        return epd;    
    }

    void 
    LTMTimestampPlugin_on_endpoint_detached(
        PRESTypePluginEndpointData endpoint_data)
    {  

        PRESTypePluginDefaultEndpointData_delete(endpoint_data);
    }

    void    
    LTMTimestampPlugin_return_sample(
        PRESTypePluginEndpointData endpoint_data,
        LTMTimestamp *sample,
        void *handle)
    {

        LTMTimestamp_finalize_optional_members(sample, RTI_TRUE);

        PRESTypePluginDefaultEndpointData_returnSample(
            endpoint_data, sample, handle);
    }

    RTIBool 
    LTMTimestampPlugin_copy_sample(
        PRESTypePluginEndpointData endpoint_data,
        LTMTimestamp *dst,
        const LTMTimestamp *src)
    {
        if (endpoint_data) {} /* To avoid warnings */
        return ltm_dds::LTMTimestampPluginSupport_copy_data(dst,src);
    }

    /* ----------------------------------------------------------------------------
    (De)Serialize functions:
    * ------------------------------------------------------------------------- */
    unsigned int 
    LTMTimestampPlugin_get_serialized_sample_max_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment);

    RTIBool 
    LTMTimestampPlugin_serialize(
        PRESTypePluginEndpointData endpoint_data,
        const LTMTimestamp *sample, 
        struct RTICdrStream *stream,    
        RTIBool serialize_encapsulation,
        RTIEncapsulationId encapsulation_id,
        RTIBool serialize_sample, 
        void *endpoint_plugin_qos)
    {
        char * position = NULL;
        RTIBool retval = RTI_TRUE;

        if (endpoint_data) {} /* To avoid warnings */
        if (endpoint_plugin_qos) {} /* To avoid warnings */

        if(serialize_encapsulation) {
            if (!RTICdrStream_serializeAndSetCdrEncapsulation(stream , encapsulation_id)) {
                return RTI_FALSE;
            }

            position = RTICdrStream_resetAlignment(stream);
        }

        if(serialize_sample) {

            if (!RTICdrStream_serializeUnsignedLongLong(
                stream, &sample->sec_)) {
                return RTI_FALSE;
            }

            if (!RTICdrStream_serializeUnsignedLong(
                stream, &sample->usec_)) {
                return RTI_FALSE;
            }

        }

        if(serialize_encapsulation) {
            RTICdrStream_restoreAlignment(stream,position);
        }

        return retval;
    }

    RTIBool 
    LTMTimestampPlugin_deserialize_sample(
        PRESTypePluginEndpointData endpoint_data,
        LTMTimestamp *sample,
        struct RTICdrStream *stream,   
        RTIBool deserialize_encapsulation,
        RTIBool deserialize_sample, 
        void *endpoint_plugin_qos)
    {

        char * position = NULL;

        RTIBool done = RTI_FALSE;

        if (endpoint_data) {} /* To avoid warnings */
        if (endpoint_plugin_qos) {} /* To avoid warnings */
        if(deserialize_encapsulation) {

            if (!RTICdrStream_deserializeAndSetCdrEncapsulation(stream)) {
                return RTI_FALSE;
            }

            position = RTICdrStream_resetAlignment(stream);
        }
        if(deserialize_sample) {

            ltm_dds::LTMTimestamp_initialize_ex(sample, RTI_FALSE, RTI_FALSE);

            if (!RTICdrStream_deserializeUnsignedLongLong(
                stream, &sample->sec_)) {
                goto fin; 
            }
            if (!RTICdrStream_deserializeUnsignedLong(
                stream, &sample->usec_)) {
                goto fin; 
            }
        }

        done = RTI_TRUE;
      fin:
        if (done != RTI_TRUE && 
        RTICdrStream_getRemainder(stream) >=
        RTI_CDR_PARAMETER_HEADER_ALIGNMENT) {
            return RTI_FALSE;   
        }
        if(deserialize_encapsulation) {
            RTICdrStream_restoreAlignment(stream,position);
        }

        return RTI_TRUE;
    }

    RTIBool
    LTMTimestampPlugin_serialize_to_cdr_buffer(
        char * buffer,
        unsigned int * length,
        const LTMTimestamp *sample)
    {
        struct RTICdrStream stream;
        struct PRESTypePluginDefaultEndpointData epd;
        RTIBool result;

        if (length == NULL) {
            return RTI_FALSE;
        }

        epd._maxSizeSerializedSample =
        LTMTimestampPlugin_get_serialized_sample_max_size(
            NULL, RTI_TRUE, RTI_CDR_ENCAPSULATION_ID_CDR_NATIVE, 0);

        if (buffer == NULL) {
            *length = 
            LTMTimestampPlugin_get_serialized_sample_size(
                (PRESTypePluginEndpointData)&epd,
                RTI_TRUE,
                RTI_CDR_ENCAPSULATION_ID_CDR_NATIVE,
                0,
                sample);

            if (*length == 0) {
                return RTI_FALSE;
            }

            return RTI_TRUE;
        }    

        RTICdrStream_init(&stream);
        RTICdrStream_set(&stream, (char *)buffer, *length);

        result = ltm_dds::LTMTimestampPlugin_serialize(
            (PRESTypePluginEndpointData)&epd, sample, &stream, 
            RTI_TRUE, RTI_CDR_ENCAPSULATION_ID_CDR_NATIVE, 
            RTI_TRUE, NULL);  

        *length = RTICdrStream_getCurrentPositionOffset(&stream);
        return result;     
    }

    RTIBool
    LTMTimestampPlugin_deserialize_from_cdr_buffer(
        LTMTimestamp *sample,
        const char * buffer,
        unsigned int length)
    {
        struct RTICdrStream stream;

        RTICdrStream_init(&stream);
        RTICdrStream_set(&stream, (char *)buffer, length);

        return LTMTimestampPlugin_deserialize_sample( 
            NULL, sample,
            &stream, RTI_TRUE, RTI_TRUE, 
            NULL);
    }

    RTIBool 
    LTMTimestampPlugin_deserialize(
        PRESTypePluginEndpointData endpoint_data,
        LTMTimestamp **sample,
        RTIBool * drop_sample,
        struct RTICdrStream *stream,   
        RTIBool deserialize_encapsulation,
        RTIBool deserialize_sample, 
        void *endpoint_plugin_qos)
    {

        RTIBool result;
        if (drop_sample) {} /* To avoid warnings */

        stream->_xTypesState.unassignable = RTI_FALSE;
        result= ltm_dds::LTMTimestampPlugin_deserialize_sample( 
            endpoint_data, (sample != NULL)?*sample:NULL,
            stream, deserialize_encapsulation, deserialize_sample, 
            endpoint_plugin_qos);
        if (result) {
            if (stream->_xTypesState.unassignable) {
                result = RTI_FALSE;
            }
        }

        return result;

    }

    RTIBool LTMTimestampPlugin_skip(
        PRESTypePluginEndpointData endpoint_data,
        struct RTICdrStream *stream,   
        RTIBool skip_encapsulation,
        RTIBool skip_sample, 
        void *endpoint_plugin_qos)
    {
        char * position = NULL;

        RTIBool done = RTI_FALSE;

        if (endpoint_data) {} /* To avoid warnings */
        if (endpoint_plugin_qos) {} /* To avoid warnings */

        if(skip_encapsulation) {
            if (!RTICdrStream_skipEncapsulation(stream)) {
                return RTI_FALSE;
            }

            position = RTICdrStream_resetAlignment(stream);
        }

        if (skip_sample) {

            if (!RTICdrStream_skipUnsignedLongLong (stream)) {
                goto fin; 
            }
            if (!RTICdrStream_skipUnsignedLong (stream)) {
                goto fin; 
            }
        }

        done = RTI_TRUE;
      fin:
        if (done != RTI_TRUE && 
        RTICdrStream_getRemainder(stream) >=
        RTI_CDR_PARAMETER_HEADER_ALIGNMENT) {
            return RTI_FALSE;   
        }
        if(skip_encapsulation) {
            RTICdrStream_restoreAlignment(stream,position);
        }

        return RTI_TRUE;
    }

    unsigned int 
    LTMTimestampPlugin_get_serialized_sample_max_size_ex(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool * overflow,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment)
    {

        unsigned int initial_alignment = current_alignment;

        unsigned int encapsulation_size = current_alignment;

        if (endpoint_data) {} /* To avoid warnings */ 
        if (overflow) {} /* To avoid warnings */

        if (include_encapsulation) {

            if (!RTICdrEncapsulation_validEncapsulationId(encapsulation_id)) {
                return 1;
            }
            RTICdrStream_getEncapsulationSize(encapsulation_size);
            encapsulation_size -= current_alignment;
            current_alignment = 0;
            initial_alignment = 0;
        }

        current_alignment +=RTICdrType_getLongLongMaxSizeSerialized(
            current_alignment);

        current_alignment +=RTICdrType_getUnsignedLongMaxSizeSerialized(
            current_alignment);

        if (include_encapsulation) {
            current_alignment += encapsulation_size;
        }
        return  current_alignment - initial_alignment;
    }

    unsigned int 
    LTMTimestampPlugin_get_serialized_sample_max_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment)
    {
        unsigned int size;
        RTIBool overflow = RTI_FALSE;

        size = LTMTimestampPlugin_get_serialized_sample_max_size_ex(
            endpoint_data,&overflow,include_encapsulation,encapsulation_id,current_alignment);

        if (overflow) {
            size = RTI_CDR_MAX_SERIALIZED_SIZE;
        }

        return size;
    }

    unsigned int 
    LTMTimestampPlugin_get_serialized_sample_min_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment)
    {

        unsigned int initial_alignment = current_alignment;

        unsigned int encapsulation_size = current_alignment;

        if (endpoint_data) {} /* To avoid warnings */ 

        if (include_encapsulation) {

            if (!RTICdrEncapsulation_validEncapsulationId(encapsulation_id)) {
                return 1;
            }
            RTICdrStream_getEncapsulationSize(encapsulation_size);
            encapsulation_size -= current_alignment;
            current_alignment = 0;
            initial_alignment = 0;
        }

        current_alignment +=RTICdrType_getLongLongMaxSizeSerialized(
            current_alignment);
        current_alignment +=RTICdrType_getUnsignedLongMaxSizeSerialized(
            current_alignment);

        if (include_encapsulation) {
            current_alignment += encapsulation_size;
        }
        return  current_alignment - initial_alignment;
    }

    /* Returns the size of the sample in its serialized form (in bytes).
    * It can also be an estimation in excess of the real buffer needed 
    * during a call to the serialize() function.
    * The value reported does not have to include the space for the
    * encapsulation flags.
    */
    unsigned int
    LTMTimestampPlugin_get_serialized_sample_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment,
        const LTMTimestamp * sample) 
    {

        unsigned int initial_alignment = current_alignment;

        unsigned int encapsulation_size = current_alignment;

        if (endpoint_data) {} /* To avoid warnings */ 
        if (sample==NULL) {
            return 0;
        }

        if (include_encapsulation) {

            if (!RTICdrEncapsulation_validEncapsulationId(encapsulation_id)) {
                return 1;
            }
            RTICdrStream_getEncapsulationSize(encapsulation_size);
            encapsulation_size -= current_alignment;
            current_alignment = 0;
            initial_alignment = 0;
        }

        current_alignment += RTICdrType_getLongLongMaxSizeSerialized(
            current_alignment);
        current_alignment += RTICdrType_getUnsignedLongMaxSizeSerialized(
            current_alignment);

        if (include_encapsulation) {
            current_alignment += encapsulation_size;
        }
        return current_alignment - initial_alignment;
    }

    /* --------------------------------------------------------------------------------------
    Key Management functions:
    * -------------------------------------------------------------------------------------- */

    PRESTypePluginKeyKind 
    LTMTimestampPlugin_get_key_kind(void)
    {
        return PRES_TYPEPLUGIN_NO_KEY;
    }

    RTIBool 
    LTMTimestampPlugin_serialize_key(
        PRESTypePluginEndpointData endpoint_data,
        const LTMTimestamp *sample, 
        struct RTICdrStream *stream,    
        RTIBool serialize_encapsulation,
        RTIEncapsulationId encapsulation_id,
        RTIBool serialize_key,
        void *endpoint_plugin_qos)
    {
        char * position = NULL;

        if(serialize_encapsulation) {
            if (!RTICdrStream_serializeAndSetCdrEncapsulation(stream , encapsulation_id)) {
                return RTI_FALSE;
            }

            position = RTICdrStream_resetAlignment(stream);
        }

        if(serialize_key) {

            if (!ltm_dds::LTMTimestampPlugin_serialize(
                endpoint_data,
                sample,
                stream,
                RTI_FALSE, encapsulation_id,
                RTI_TRUE,
                endpoint_plugin_qos)) {
                return RTI_FALSE;
            }

        }

        if(serialize_encapsulation) {
            RTICdrStream_restoreAlignment(stream,position);
        }

        return RTI_TRUE;
    }

    RTIBool LTMTimestampPlugin_deserialize_key_sample(
        PRESTypePluginEndpointData endpoint_data,
        LTMTimestamp *sample, 
        struct RTICdrStream *stream,
        RTIBool deserialize_encapsulation,
        RTIBool deserialize_key,
        void *endpoint_plugin_qos)
    {
        char * position = NULL;

        if (endpoint_data) {} /* To avoid warnings */
        if (endpoint_plugin_qos) {} /* To avoid warnings */

        if(deserialize_encapsulation) {

            if (!RTICdrStream_deserializeAndSetCdrEncapsulation(stream)) {
                return RTI_FALSE;
            }

            position = RTICdrStream_resetAlignment(stream);
        }
        if (deserialize_key) {

            if (!ltm_dds::LTMTimestampPlugin_deserialize_sample(
                endpoint_data, sample, stream, 
                RTI_FALSE, RTI_TRUE, 
                endpoint_plugin_qos)) {
                return RTI_FALSE;
            }
        }

        if(deserialize_encapsulation) {
            RTICdrStream_restoreAlignment(stream,position);
        }

        return RTI_TRUE;
    }

    RTIBool LTMTimestampPlugin_deserialize_key(
        PRESTypePluginEndpointData endpoint_data,
        LTMTimestamp **sample, 
        RTIBool * drop_sample,
        struct RTICdrStream *stream,
        RTIBool deserialize_encapsulation,
        RTIBool deserialize_key,
        void *endpoint_plugin_qos)
    {
        RTIBool result;
        if (drop_sample) {} /* To avoid warnings */
        stream->_xTypesState.unassignable = RTI_FALSE;
        result= ltm_dds::LTMTimestampPlugin_deserialize_key_sample(
            endpoint_data, (sample != NULL)?*sample:NULL, stream,
            deserialize_encapsulation, deserialize_key, endpoint_plugin_qos);
        if (result) {
            if (stream->_xTypesState.unassignable) {
                result = RTI_FALSE;
            }
        }

        return result;    

    }

    unsigned int
    LTMTimestampPlugin_get_serialized_key_max_size_ex(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool * overflow,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment)
    {

        unsigned int initial_alignment = current_alignment;

        unsigned int encapsulation_size = current_alignment;

        if (endpoint_data) {} /* To avoid warnings */
        if (overflow) {} /* To avoid warnings */

        if (include_encapsulation) {

            if (!RTICdrEncapsulation_validEncapsulationId(encapsulation_id)) {
                return 1;
            }
            RTICdrStream_getEncapsulationSize(encapsulation_size);
            encapsulation_size -= current_alignment;
            current_alignment = 0;
            initial_alignment = 0;
        }

        current_alignment += ltm_dds::LTMTimestampPlugin_get_serialized_sample_max_size_ex(
            endpoint_data, overflow,RTI_FALSE, encapsulation_id, current_alignment);

        if (include_encapsulation) {
            current_alignment += encapsulation_size;
        }
        return current_alignment - initial_alignment;
    }

    unsigned int
    LTMTimestampPlugin_get_serialized_key_max_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment)
    {
        unsigned int size;
        RTIBool overflow = RTI_FALSE;

        size = LTMTimestampPlugin_get_serialized_key_max_size_ex(
            endpoint_data,&overflow,include_encapsulation,encapsulation_id,current_alignment);

        if (overflow) {
            size = RTI_CDR_MAX_SERIALIZED_SIZE;
        }

        return size;
    }

    RTIBool 
    LTMTimestampPlugin_serialized_sample_to_key(
        PRESTypePluginEndpointData endpoint_data,
        LTMTimestamp *sample,
        struct RTICdrStream *stream, 
        RTIBool deserialize_encapsulation,  
        RTIBool deserialize_key, 
        void *endpoint_plugin_qos)
    {
        char * position = NULL;

        RTIBool done = RTI_FALSE;
        RTIBool error = RTI_FALSE;

        if (stream == NULL) {
            error = RTI_TRUE;
            goto fin;
        }
        if(deserialize_encapsulation) {
            if (!RTICdrStream_deserializeAndSetCdrEncapsulation(stream)) {
                return RTI_FALSE;
            }
            position = RTICdrStream_resetAlignment(stream);
        }

        if (deserialize_key) {

            if (!ltm_dds::LTMTimestampPlugin_deserialize_sample(
                endpoint_data, sample, stream, RTI_FALSE, 
                RTI_TRUE, endpoint_plugin_qos)) {
                return RTI_FALSE;
            }

        }

        done = RTI_TRUE;
      fin:
        if(!error) {
            if (done != RTI_TRUE && 
            RTICdrStream_getRemainder(stream) >=
            RTI_CDR_PARAMETER_HEADER_ALIGNMENT) {
                return RTI_FALSE;   
            }
        } else {
            return error;
        }       

        if(deserialize_encapsulation) {
            RTICdrStream_restoreAlignment(stream,position);
        }

        return RTI_TRUE;
    }

    /* ------------------------------------------------------------------------
    * Plug-in Installation Methods
    * ------------------------------------------------------------------------ */
    struct PRESTypePlugin *LTMTimestampPlugin_new(void) 
    { 
        struct PRESTypePlugin *plugin = NULL;
        const struct PRESTypePluginVersion PLUGIN_VERSION = 
        PRES_TYPE_PLUGIN_VERSION_2_0;

        RTIOsapiHeap_allocateStructure(
            &plugin, struct PRESTypePlugin);
        if (plugin == NULL) {
            return NULL;
        }

        plugin->version = PLUGIN_VERSION;

        /* set up parent's function pointers */
        plugin->onParticipantAttached =
        (PRESTypePluginOnParticipantAttachedCallback)
        ltm_dds::LTMTimestampPlugin_on_participant_attached;
        plugin->onParticipantDetached =
        (PRESTypePluginOnParticipantDetachedCallback)
        ltm_dds::LTMTimestampPlugin_on_participant_detached;
        plugin->onEndpointAttached =
        (PRESTypePluginOnEndpointAttachedCallback)
        ltm_dds::LTMTimestampPlugin_on_endpoint_attached;
        plugin->onEndpointDetached =
        (PRESTypePluginOnEndpointDetachedCallback)
        ltm_dds::LTMTimestampPlugin_on_endpoint_detached;

        plugin->copySampleFnc =
        (PRESTypePluginCopySampleFunction)
        ltm_dds::LTMTimestampPlugin_copy_sample;
        plugin->createSampleFnc =
        (PRESTypePluginCreateSampleFunction)
        LTMTimestampPlugin_create_sample;
        plugin->destroySampleFnc =
        (PRESTypePluginDestroySampleFunction)
        LTMTimestampPlugin_destroy_sample;

        plugin->serializeFnc =
        (PRESTypePluginSerializeFunction)
        ltm_dds::LTMTimestampPlugin_serialize;
        plugin->deserializeFnc =
        (PRESTypePluginDeserializeFunction)
        ltm_dds::LTMTimestampPlugin_deserialize;
        plugin->getSerializedSampleMaxSizeFnc =
        (PRESTypePluginGetSerializedSampleMaxSizeFunction)
        ltm_dds::LTMTimestampPlugin_get_serialized_sample_max_size;
        plugin->getSerializedSampleMinSizeFnc =
        (PRESTypePluginGetSerializedSampleMinSizeFunction)
        ltm_dds::LTMTimestampPlugin_get_serialized_sample_min_size;

        plugin->getSampleFnc =
        (PRESTypePluginGetSampleFunction)
        LTMTimestampPlugin_get_sample;
        plugin->returnSampleFnc =
        (PRESTypePluginReturnSampleFunction)
        LTMTimestampPlugin_return_sample;

        plugin->getKeyKindFnc =
        (PRESTypePluginGetKeyKindFunction)
        ltm_dds::LTMTimestampPlugin_get_key_kind;

        /* These functions are only used for keyed types. As this is not a keyed
        type they are all set to NULL
        */
        plugin->serializeKeyFnc = NULL ;    
        plugin->deserializeKeyFnc = NULL;  
        plugin->getKeyFnc = NULL;
        plugin->returnKeyFnc = NULL;
        plugin->instanceToKeyFnc = NULL;
        plugin->keyToInstanceFnc = NULL;
        plugin->getSerializedKeyMaxSizeFnc = NULL;
        plugin->instanceToKeyHashFnc = NULL;
        plugin->serializedSampleToKeyHashFnc = NULL;
        plugin->serializedKeyToKeyHashFnc = NULL;    
        plugin->typeCode =  (struct RTICdrTypeCode *)ltm_dds::LTMTimestamp_get_typecode();

        plugin->languageKind = PRES_TYPEPLUGIN_CPP_LANG;

        /* Serialized buffer */
        plugin->getBuffer = 
        (PRESTypePluginGetBufferFunction)
        LTMTimestampPlugin_get_buffer;
        plugin->returnBuffer = 
        (PRESTypePluginReturnBufferFunction)
        LTMTimestampPlugin_return_buffer;
        plugin->getSerializedSampleSizeFnc =
        (PRESTypePluginGetSerializedSampleSizeFunction)
        ltm_dds::LTMTimestampPlugin_get_serialized_sample_size;

        plugin->endpointTypeName = LTMTimestampTYPENAME;

        return plugin;
    }

    void
    LTMTimestampPlugin_delete(struct PRESTypePlugin *plugin)
    {
        RTIOsapiHeap_freeStructure(plugin);
    } 

    /* ----------------------------------------------------------------------------
    *  Type LTMTopicDetails
    * -------------------------------------------------------------------------- */

    /* -----------------------------------------------------------------------------
    Support functions:
    * -------------------------------------------------------------------------- */

    LTMTopicDetails*
    LTMTopicDetailsPluginSupport_create_data_w_params(
        const struct DDS_TypeAllocationParams_t * alloc_params){
        LTMTopicDetails *sample = NULL;

        RTIOsapiHeap_allocateStructure(
            &sample, LTMTopicDetails);

        if(sample != NULL) {
            if (!ltm_dds::LTMTopicDetails_initialize_w_params(sample,alloc_params)) {
                RTIOsapiHeap_freeStructure(sample);
                return NULL;
            }
        }        
        return sample; 
    } 

    LTMTopicDetails *
    LTMTopicDetailsPluginSupport_create_data_ex(RTIBool allocate_pointers){
        LTMTopicDetails *sample = NULL;

        RTIOsapiHeap_allocateStructure(
            &sample, LTMTopicDetails);

        if(sample != NULL) {
            if (!ltm_dds::LTMTopicDetails_initialize_ex(sample,allocate_pointers, RTI_TRUE)) {
                RTIOsapiHeap_freeStructure(sample);
                return NULL;
            }
        }
        return sample; 
    }

    LTMTopicDetails *
    LTMTopicDetailsPluginSupport_create_data(void)
    {
        return ltm_dds::LTMTopicDetailsPluginSupport_create_data_ex(RTI_TRUE);
    }

    void 
    LTMTopicDetailsPluginSupport_destroy_data_w_params(
        LTMTopicDetails *sample,
        const struct DDS_TypeDeallocationParams_t * dealloc_params) {

        ltm_dds::LTMTopicDetails_finalize_w_params(sample,dealloc_params);

        RTIOsapiHeap_freeStructure(sample);
    }

    void 
    LTMTopicDetailsPluginSupport_destroy_data_ex(
        LTMTopicDetails *sample,RTIBool deallocate_pointers) {

        ltm_dds::LTMTopicDetails_finalize_ex(sample,deallocate_pointers);

        RTIOsapiHeap_freeStructure(sample);
    }

    void 
    LTMTopicDetailsPluginSupport_destroy_data(
        LTMTopicDetails *sample) {

        ltm_dds::LTMTopicDetailsPluginSupport_destroy_data_ex(sample,RTI_TRUE);

    }

    RTIBool 
    LTMTopicDetailsPluginSupport_copy_data(
        LTMTopicDetails *dst,
        const LTMTopicDetails *src)
    {
        return ltm_dds::LTMTopicDetails_copy(dst,src);
    }

    void 
    LTMTopicDetailsPluginSupport_print_data(
        const LTMTopicDetails *sample,
        const char *desc,
        unsigned int indent_level)
    {

        RTICdrType_printIndent(indent_level);

        if (desc != NULL) {
            RTILog_debug("%s:\n", desc);
        } else {
            RTILog_debug("\n");
        }

        if (sample == NULL) {
            RTILog_debug("NULL\n");
            return;
        }

        ltm_dds::LTMTimestampPluginSupport_print_data(
            &sample->topicTime_, "topicTime_", indent_level + 1);

        RTICdrType_printUnsignedLong(
            &sample->topicID_, "topicID_", indent_level + 1);    

        if (sample->topicData_==NULL) {
            RTICdrType_printString(
                NULL,"topicData_", indent_level + 1);
        } else {
            RTICdrType_printString(
                sample->topicData_,"topicData_", indent_level + 1);    
        }

    }
    LTMTopicDetails *
    LTMTopicDetailsPluginSupport_create_key_ex(RTIBool allocate_pointers){
        LTMTopicDetails *key = NULL;

        RTIOsapiHeap_allocateStructure(
            &key, LTMTopicDetailsKeyHolder);

        ltm_dds::LTMTopicDetails_initialize_ex(key,allocate_pointers, RTI_TRUE);
        return key;
    }

    LTMTopicDetails *
    LTMTopicDetailsPluginSupport_create_key(void)
    {
        return  ltm_dds::LTMTopicDetailsPluginSupport_create_key_ex(RTI_TRUE);
    }

    void 
    LTMTopicDetailsPluginSupport_destroy_key_ex(
        LTMTopicDetailsKeyHolder *key,RTIBool deallocate_pointers)
    {
        ltm_dds::LTMTopicDetails_finalize_ex(key,deallocate_pointers);

        RTIOsapiHeap_freeStructure(key);
    }

    void 
    LTMTopicDetailsPluginSupport_destroy_key(
        LTMTopicDetailsKeyHolder *key) {

        ltm_dds::LTMTopicDetailsPluginSupport_destroy_key_ex(key,RTI_TRUE);

    }

    /* ----------------------------------------------------------------------------
    Callback functions:
    * ---------------------------------------------------------------------------- */

    PRESTypePluginParticipantData 
    LTMTopicDetailsPlugin_on_participant_attached(
        void *registration_data,
        const struct PRESTypePluginParticipantInfo *participant_info,
        RTIBool top_level_registration,
        void *container_plugin_context,
        RTICdrTypeCode *type_code)
    {
        if (registration_data) {} /* To avoid warnings */
        if (participant_info) {} /* To avoid warnings */
        if (top_level_registration) {} /* To avoid warnings */
        if (container_plugin_context) {} /* To avoid warnings */
        if (type_code) {} /* To avoid warnings */

        return PRESTypePluginDefaultParticipantData_new(participant_info);

    }

    void 
    LTMTopicDetailsPlugin_on_participant_detached(
        PRESTypePluginParticipantData participant_data)
    {

        PRESTypePluginDefaultParticipantData_delete(participant_data);
    }

    PRESTypePluginEndpointData
    LTMTopicDetailsPlugin_on_endpoint_attached(
        PRESTypePluginParticipantData participant_data,
        const struct PRESTypePluginEndpointInfo *endpoint_info,
        RTIBool top_level_registration, 
        void *containerPluginContext)
    {
        PRESTypePluginEndpointData epd = NULL;

        unsigned int serializedSampleMaxSize;

        unsigned int serializedKeyMaxSize;

        if (top_level_registration) {} /* To avoid warnings */
        if (containerPluginContext) {} /* To avoid warnings */

        epd = PRESTypePluginDefaultEndpointData_new(
            participant_data,
            endpoint_info,
            (PRESTypePluginDefaultEndpointDataCreateSampleFunction)
            ltm_dds::LTMTopicDetailsPluginSupport_create_data,
            (PRESTypePluginDefaultEndpointDataDestroySampleFunction)
            ltm_dds::LTMTopicDetailsPluginSupport_destroy_data,
            (PRESTypePluginDefaultEndpointDataCreateKeyFunction)
            ltm_dds::LTMTopicDetailsPluginSupport_create_key ,            
            (PRESTypePluginDefaultEndpointDataDestroyKeyFunction)
            ltm_dds::LTMTopicDetailsPluginSupport_destroy_key);

        if (epd == NULL) {
            return NULL;
        } 
        serializedKeyMaxSize =  ltm_dds::LTMTopicDetailsPlugin_get_serialized_key_max_size(
            epd,RTI_FALSE,RTI_CDR_ENCAPSULATION_ID_CDR_BE,0);

        if(!PRESTypePluginDefaultEndpointData_createMD5StreamWithInfo(
            epd,endpoint_info,serializedKeyMaxSize))  
        {
            PRESTypePluginDefaultEndpointData_delete(epd);
            return NULL;
        }

        if (endpoint_info->endpointKind == PRES_TYPEPLUGIN_ENDPOINT_WRITER) {
            serializedSampleMaxSize = ltm_dds::LTMTopicDetailsPlugin_get_serialized_sample_max_size(
                epd,RTI_FALSE,RTI_CDR_ENCAPSULATION_ID_CDR_BE,0);

            PRESTypePluginDefaultEndpointData_setMaxSizeSerializedSample(epd, serializedSampleMaxSize);

            if (PRESTypePluginDefaultEndpointData_createWriterPool(
                epd,
                endpoint_info,
                (PRESTypePluginGetSerializedSampleMaxSizeFunction)
                ltm_dds::LTMTopicDetailsPlugin_get_serialized_sample_max_size, epd,
                (PRESTypePluginGetSerializedSampleSizeFunction)
                ltm_dds::LTMTopicDetailsPlugin_get_serialized_sample_size,
                epd) == RTI_FALSE) {
                PRESTypePluginDefaultEndpointData_delete(epd);
                return NULL;
            }
        }

        return epd;    
    }

    void 
    LTMTopicDetailsPlugin_on_endpoint_detached(
        PRESTypePluginEndpointData endpoint_data)
    {  

        PRESTypePluginDefaultEndpointData_delete(endpoint_data);
    }

    void    
    LTMTopicDetailsPlugin_return_sample(
        PRESTypePluginEndpointData endpoint_data,
        LTMTopicDetails *sample,
        void *handle)
    {

        LTMTopicDetails_finalize_optional_members(sample, RTI_TRUE);

        PRESTypePluginDefaultEndpointData_returnSample(
            endpoint_data, sample, handle);
    }

    RTIBool 
    LTMTopicDetailsPlugin_copy_sample(
        PRESTypePluginEndpointData endpoint_data,
        LTMTopicDetails *dst,
        const LTMTopicDetails *src)
    {
        if (endpoint_data) {} /* To avoid warnings */
        return ltm_dds::LTMTopicDetailsPluginSupport_copy_data(dst,src);
    }

    /* ----------------------------------------------------------------------------
    (De)Serialize functions:
    * ------------------------------------------------------------------------- */
    unsigned int 
    LTMTopicDetailsPlugin_get_serialized_sample_max_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment);

    RTIBool 
    LTMTopicDetailsPlugin_serialize(
        PRESTypePluginEndpointData endpoint_data,
        const LTMTopicDetails *sample, 
        struct RTICdrStream *stream,    
        RTIBool serialize_encapsulation,
        RTIEncapsulationId encapsulation_id,
        RTIBool serialize_sample, 
        void *endpoint_plugin_qos)
    {
        char * position = NULL;
        RTIBool retval = RTI_TRUE;

        if (endpoint_data) {} /* To avoid warnings */
        if (endpoint_plugin_qos) {} /* To avoid warnings */

        if(serialize_encapsulation) {
            if (!RTICdrStream_serializeAndSetCdrEncapsulation(stream , encapsulation_id)) {
                return RTI_FALSE;
            }

            position = RTICdrStream_resetAlignment(stream);
        }

        if(serialize_sample) {

            if(!ltm_dds::LTMTimestampPlugin_serialize(
                endpoint_data,
                &sample->topicTime_,
                stream,
                RTI_FALSE, encapsulation_id,
                RTI_TRUE,
                endpoint_plugin_qos)) {
                return RTI_FALSE;
            }

            if (!RTICdrStream_serializeUnsignedLong(
                stream, &sample->topicID_)) {
                return RTI_FALSE;
            }

            if (!RTICdrStream_serializeString(
                stream, sample->topicData_, (255) + 1)) {
                return RTI_FALSE;
            }

        }

        if(serialize_encapsulation) {
            RTICdrStream_restoreAlignment(stream,position);
        }

        return retval;
    }

    RTIBool 
    LTMTopicDetailsPlugin_deserialize_sample(
        PRESTypePluginEndpointData endpoint_data,
        LTMTopicDetails *sample,
        struct RTICdrStream *stream,   
        RTIBool deserialize_encapsulation,
        RTIBool deserialize_sample, 
        void *endpoint_plugin_qos)
    {

        char * position = NULL;

        RTIBool done = RTI_FALSE;

        if (endpoint_data) {} /* To avoid warnings */
        if (endpoint_plugin_qos) {} /* To avoid warnings */
        if(deserialize_encapsulation) {

            if (!RTICdrStream_deserializeAndSetCdrEncapsulation(stream)) {
                return RTI_FALSE;
            }

            position = RTICdrStream_resetAlignment(stream);
        }
        if(deserialize_sample) {

            ltm_dds::LTMTopicDetails_initialize_ex(sample, RTI_FALSE, RTI_FALSE);

            if(!ltm_dds::LTMTimestampPlugin_deserialize_sample(
                endpoint_data,
                &sample->topicTime_,
                stream,
                RTI_FALSE, RTI_TRUE,
                endpoint_plugin_qos)) {
                goto fin; 
            }
            if (!RTICdrStream_deserializeUnsignedLong(
                stream, &sample->topicID_)) {
                goto fin; 
            }
            if (!RTICdrStream_deserializeStringEx(
                stream,&sample->topicData_, (255) + 1, RTI_FALSE)) {
                goto fin; 
            }
        }

        done = RTI_TRUE;
      fin:
        if (done != RTI_TRUE && 
        RTICdrStream_getRemainder(stream) >=
        RTI_CDR_PARAMETER_HEADER_ALIGNMENT) {
            return RTI_FALSE;   
        }
        if(deserialize_encapsulation) {
            RTICdrStream_restoreAlignment(stream,position);
        }

        return RTI_TRUE;
    }

    RTIBool
    LTMTopicDetailsPlugin_serialize_to_cdr_buffer(
        char * buffer,
        unsigned int * length,
        const LTMTopicDetails *sample)
    {
        struct RTICdrStream stream;
        struct PRESTypePluginDefaultEndpointData epd;
        RTIBool result;

        if (length == NULL) {
            return RTI_FALSE;
        }

        epd._maxSizeSerializedSample =
        LTMTopicDetailsPlugin_get_serialized_sample_max_size(
            NULL, RTI_TRUE, RTI_CDR_ENCAPSULATION_ID_CDR_NATIVE, 0);

        if (buffer == NULL) {
            *length = 
            LTMTopicDetailsPlugin_get_serialized_sample_size(
                (PRESTypePluginEndpointData)&epd,
                RTI_TRUE,
                RTI_CDR_ENCAPSULATION_ID_CDR_NATIVE,
                0,
                sample);

            if (*length == 0) {
                return RTI_FALSE;
            }

            return RTI_TRUE;
        }    

        RTICdrStream_init(&stream);
        RTICdrStream_set(&stream, (char *)buffer, *length);

        result = ltm_dds::LTMTopicDetailsPlugin_serialize(
            (PRESTypePluginEndpointData)&epd, sample, &stream, 
            RTI_TRUE, RTI_CDR_ENCAPSULATION_ID_CDR_NATIVE, 
            RTI_TRUE, NULL);  

        *length = RTICdrStream_getCurrentPositionOffset(&stream);
        return result;     
    }

    RTIBool
    LTMTopicDetailsPlugin_deserialize_from_cdr_buffer(
        LTMTopicDetails *sample,
        const char * buffer,
        unsigned int length)
    {
        struct RTICdrStream stream;

        RTICdrStream_init(&stream);
        RTICdrStream_set(&stream, (char *)buffer, length);

        return LTMTopicDetailsPlugin_deserialize_sample( 
            NULL, sample,
            &stream, RTI_TRUE, RTI_TRUE, 
            NULL);
    }

    RTIBool 
    LTMTopicDetailsPlugin_deserialize(
        PRESTypePluginEndpointData endpoint_data,
        LTMTopicDetails **sample,
        RTIBool * drop_sample,
        struct RTICdrStream *stream,   
        RTIBool deserialize_encapsulation,
        RTIBool deserialize_sample, 
        void *endpoint_plugin_qos)
    {

        RTIBool result;
        if (drop_sample) {} /* To avoid warnings */

        stream->_xTypesState.unassignable = RTI_FALSE;
        result= ltm_dds::LTMTopicDetailsPlugin_deserialize_sample( 
            endpoint_data, (sample != NULL)?*sample:NULL,
            stream, deserialize_encapsulation, deserialize_sample, 
            endpoint_plugin_qos);
        if (result) {
            if (stream->_xTypesState.unassignable) {
                result = RTI_FALSE;
            }
        }

        return result;

    }

    RTIBool LTMTopicDetailsPlugin_skip(
        PRESTypePluginEndpointData endpoint_data,
        struct RTICdrStream *stream,   
        RTIBool skip_encapsulation,
        RTIBool skip_sample, 
        void *endpoint_plugin_qos)
    {
        char * position = NULL;

        RTIBool done = RTI_FALSE;

        if (endpoint_data) {} /* To avoid warnings */
        if (endpoint_plugin_qos) {} /* To avoid warnings */

        if(skip_encapsulation) {
            if (!RTICdrStream_skipEncapsulation(stream)) {
                return RTI_FALSE;
            }

            position = RTICdrStream_resetAlignment(stream);
        }

        if (skip_sample) {

            if (!ltm_dds::LTMTimestampPlugin_skip(
                endpoint_data,
                stream, 
                RTI_FALSE, RTI_TRUE, 
                endpoint_plugin_qos)) {
                goto fin; 
            }
            if (!RTICdrStream_skipUnsignedLong (stream)) {
                goto fin; 
            }
            if (!RTICdrStream_skipString (stream, (255)+1)) {
                goto fin; 
            }
        }

        done = RTI_TRUE;
      fin:
        if (done != RTI_TRUE && 
        RTICdrStream_getRemainder(stream) >=
        RTI_CDR_PARAMETER_HEADER_ALIGNMENT) {
            return RTI_FALSE;   
        }
        if(skip_encapsulation) {
            RTICdrStream_restoreAlignment(stream,position);
        }

        return RTI_TRUE;
    }

    unsigned int 
    LTMTopicDetailsPlugin_get_serialized_sample_max_size_ex(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool * overflow,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment)
    {

        unsigned int initial_alignment = current_alignment;

        unsigned int encapsulation_size = current_alignment;

        if (include_encapsulation) {

            if (!RTICdrEncapsulation_validEncapsulationId(encapsulation_id)) {
                return 1;
            }
            RTICdrStream_getEncapsulationSize(encapsulation_size);
            encapsulation_size -= current_alignment;
            current_alignment = 0;
            initial_alignment = 0;
        }

        current_alignment +=ltm_dds::LTMTimestampPlugin_get_serialized_sample_max_size_ex(
            endpoint_data, overflow, RTI_FALSE,encapsulation_id,current_alignment);

        current_alignment +=RTICdrType_getUnsignedLongMaxSizeSerialized(
            current_alignment);

        current_alignment +=RTICdrType_getStringMaxSizeSerialized(
            current_alignment, (255)+1);

        if (include_encapsulation) {
            current_alignment += encapsulation_size;
        }
        return  current_alignment - initial_alignment;
    }

    unsigned int 
    LTMTopicDetailsPlugin_get_serialized_sample_max_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment)
    {
        unsigned int size;
        RTIBool overflow = RTI_FALSE;

        size = LTMTopicDetailsPlugin_get_serialized_sample_max_size_ex(
            endpoint_data,&overflow,include_encapsulation,encapsulation_id,current_alignment);

        if (overflow) {
            size = RTI_CDR_MAX_SERIALIZED_SIZE;
        }

        return size;
    }

    unsigned int 
    LTMTopicDetailsPlugin_get_serialized_sample_min_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment)
    {

        unsigned int initial_alignment = current_alignment;

        unsigned int encapsulation_size = current_alignment;

        if (endpoint_data) {} /* To avoid warnings */ 

        if (include_encapsulation) {

            if (!RTICdrEncapsulation_validEncapsulationId(encapsulation_id)) {
                return 1;
            }
            RTICdrStream_getEncapsulationSize(encapsulation_size);
            encapsulation_size -= current_alignment;
            current_alignment = 0;
            initial_alignment = 0;
        }

        current_alignment +=ltm_dds::LTMTimestampPlugin_get_serialized_sample_min_size(
            endpoint_data,RTI_FALSE,encapsulation_id,current_alignment);
        current_alignment +=RTICdrType_getUnsignedLongMaxSizeSerialized(
            current_alignment);
        current_alignment +=RTICdrType_getStringMaxSizeSerialized(
            current_alignment, 1);

        if (include_encapsulation) {
            current_alignment += encapsulation_size;
        }
        return  current_alignment - initial_alignment;
    }

    /* Returns the size of the sample in its serialized form (in bytes).
    * It can also be an estimation in excess of the real buffer needed 
    * during a call to the serialize() function.
    * The value reported does not have to include the space for the
    * encapsulation flags.
    */
    unsigned int
    LTMTopicDetailsPlugin_get_serialized_sample_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment,
        const LTMTopicDetails * sample) 
    {

        unsigned int initial_alignment = current_alignment;

        unsigned int encapsulation_size = current_alignment;

        if (endpoint_data) {} /* To avoid warnings */ 
        if (sample==NULL) {
            return 0;
        }

        if (include_encapsulation) {

            if (!RTICdrEncapsulation_validEncapsulationId(encapsulation_id)) {
                return 1;
            }
            RTICdrStream_getEncapsulationSize(encapsulation_size);
            encapsulation_size -= current_alignment;
            current_alignment = 0;
            initial_alignment = 0;
        }

        current_alignment += ltm_dds::LTMTimestampPlugin_get_serialized_sample_size(
            endpoint_data,RTI_FALSE, encapsulation_id,
            current_alignment, &sample->topicTime_);
        current_alignment += RTICdrType_getUnsignedLongMaxSizeSerialized(
            current_alignment);
        current_alignment += RTICdrType_getStringSerializedSize(
            current_alignment, sample->topicData_);

        if (include_encapsulation) {
            current_alignment += encapsulation_size;
        }
        return current_alignment - initial_alignment;
    }

    /* --------------------------------------------------------------------------------------
    Key Management functions:
    * -------------------------------------------------------------------------------------- */

    PRESTypePluginKeyKind 
    LTMTopicDetailsPlugin_get_key_kind(void)
    {
        return PRES_TYPEPLUGIN_USER_KEY;
    }

    RTIBool 
    LTMTopicDetailsPlugin_serialize_key(
        PRESTypePluginEndpointData endpoint_data,
        const LTMTopicDetails *sample, 
        struct RTICdrStream *stream,    
        RTIBool serialize_encapsulation,
        RTIEncapsulationId encapsulation_id,
        RTIBool serialize_key,
        void *endpoint_plugin_qos)
    {
        char * position = NULL;

        if (endpoint_data) {} /* To avoid warnings */
        if (endpoint_plugin_qos) {} /* To avoid warnings */

        if(serialize_encapsulation) {
            if (!RTICdrStream_serializeAndSetCdrEncapsulation(stream , encapsulation_id)) {
                return RTI_FALSE;
            }

            position = RTICdrStream_resetAlignment(stream);
        }

        if(serialize_key) {

            if (!RTICdrStream_serializeUnsignedLong(
                stream, &sample->topicID_)) {
                return RTI_FALSE;
            }

        }

        if(serialize_encapsulation) {
            RTICdrStream_restoreAlignment(stream,position);
        }

        return RTI_TRUE;
    }

    RTIBool LTMTopicDetailsPlugin_deserialize_key_sample(
        PRESTypePluginEndpointData endpoint_data,
        LTMTopicDetails *sample, 
        struct RTICdrStream *stream,
        RTIBool deserialize_encapsulation,
        RTIBool deserialize_key,
        void *endpoint_plugin_qos)
    {
        char * position = NULL;

        if (endpoint_data) {} /* To avoid warnings */
        if (endpoint_plugin_qos) {} /* To avoid warnings */

        if(deserialize_encapsulation) {

            if (!RTICdrStream_deserializeAndSetCdrEncapsulation(stream)) {
                return RTI_FALSE;
            }

            position = RTICdrStream_resetAlignment(stream);
        }
        if (deserialize_key) {

            if (!RTICdrStream_deserializeUnsignedLong(
                stream, &sample->topicID_)) {
                return RTI_FALSE;
            }
        }

        if(deserialize_encapsulation) {
            RTICdrStream_restoreAlignment(stream,position);
        }

        return RTI_TRUE;
    }

    RTIBool LTMTopicDetailsPlugin_deserialize_key(
        PRESTypePluginEndpointData endpoint_data,
        LTMTopicDetails **sample, 
        RTIBool * drop_sample,
        struct RTICdrStream *stream,
        RTIBool deserialize_encapsulation,
        RTIBool deserialize_key,
        void *endpoint_plugin_qos)
    {
        RTIBool result;
        if (drop_sample) {} /* To avoid warnings */
        stream->_xTypesState.unassignable = RTI_FALSE;
        result= ltm_dds::LTMTopicDetailsPlugin_deserialize_key_sample(
            endpoint_data, (sample != NULL)?*sample:NULL, stream,
            deserialize_encapsulation, deserialize_key, endpoint_plugin_qos);
        if (result) {
            if (stream->_xTypesState.unassignable) {
                result = RTI_FALSE;
            }
        }

        return result;    

    }

    unsigned int
    LTMTopicDetailsPlugin_get_serialized_key_max_size_ex(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool * overflow,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment)
    {

        unsigned int initial_alignment = current_alignment;

        unsigned int encapsulation_size = current_alignment;

        if (endpoint_data) {} /* To avoid warnings */
        if (overflow) {} /* To avoid warnings */

        if (include_encapsulation) {

            if (!RTICdrEncapsulation_validEncapsulationId(encapsulation_id)) {
                return 1;
            }
            RTICdrStream_getEncapsulationSize(encapsulation_size);
            encapsulation_size -= current_alignment;
            current_alignment = 0;
            initial_alignment = 0;
        }

        current_alignment +=RTICdrType_getUnsignedLongMaxSizeSerialized(
            current_alignment);

        if (include_encapsulation) {
            current_alignment += encapsulation_size;
        }
        return current_alignment - initial_alignment;
    }

    unsigned int
    LTMTopicDetailsPlugin_get_serialized_key_max_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment)
    {
        unsigned int size;
        RTIBool overflow = RTI_FALSE;

        size = LTMTopicDetailsPlugin_get_serialized_key_max_size_ex(
            endpoint_data,&overflow,include_encapsulation,encapsulation_id,current_alignment);

        if (overflow) {
            size = RTI_CDR_MAX_SERIALIZED_SIZE;
        }

        return size;
    }

    RTIBool 
    LTMTopicDetailsPlugin_serialized_sample_to_key(
        PRESTypePluginEndpointData endpoint_data,
        LTMTopicDetails *sample,
        struct RTICdrStream *stream, 
        RTIBool deserialize_encapsulation,  
        RTIBool deserialize_key, 
        void *endpoint_plugin_qos)
    {
        char * position = NULL;

        RTIBool done = RTI_FALSE;
        RTIBool error = RTI_FALSE;

        if (endpoint_data) {} /* To avoid warnings */
        if (endpoint_plugin_qos) {} /* To avoid warnings */

        if (stream == NULL) {
            error = RTI_TRUE;
            goto fin;
        }
        if(deserialize_encapsulation) {
            if (!RTICdrStream_deserializeAndSetCdrEncapsulation(stream)) {
                return RTI_FALSE;
            }
            position = RTICdrStream_resetAlignment(stream);
        }

        if (deserialize_key) {

            if (!ltm_dds::LTMTimestampPlugin_skip(
                endpoint_data,
                stream, 
                RTI_FALSE, RTI_TRUE, 
                endpoint_plugin_qos)) {
                goto fin; 
            }

            if (!RTICdrStream_deserializeUnsignedLong(
                stream, &sample->topicID_)) {
                return RTI_FALSE;
            }
            if (!RTICdrStream_skipString (stream, (255)+1)) {
                goto fin; 
            }

        }

        done = RTI_TRUE;
      fin:
        if(!error) {
            if (done != RTI_TRUE && 
            RTICdrStream_getRemainder(stream) >=
            RTI_CDR_PARAMETER_HEADER_ALIGNMENT) {
                return RTI_FALSE;   
            }
        } else {
            return error;
        }       

        if(deserialize_encapsulation) {
            RTICdrStream_restoreAlignment(stream,position);
        }

        return RTI_TRUE;
    }

    RTIBool 
    LTMTopicDetailsPlugin_instance_to_key(
        PRESTypePluginEndpointData endpoint_data,
        LTMTopicDetailsKeyHolder *dst, 
        const LTMTopicDetails *src)
    {

        if (endpoint_data) {} /* To avoid warnings */   

        if (!RTICdrType_copyUnsignedLong (
            &dst->topicID_, &src->topicID_)) { 
            return RTI_FALSE;
        }
        return RTI_TRUE;
    }

    RTIBool 
    LTMTopicDetailsPlugin_key_to_instance(
        PRESTypePluginEndpointData endpoint_data,
        LTMTopicDetails *dst, const
        LTMTopicDetailsKeyHolder *src)
    {

        if (endpoint_data) {} /* To avoid warnings */   
        if (!RTICdrType_copyUnsignedLong (
            &dst->topicID_, &src->topicID_)) { 
            return RTI_FALSE;
        }
        return RTI_TRUE;
    }

    RTIBool 
    LTMTopicDetailsPlugin_instance_to_keyhash(
        PRESTypePluginEndpointData endpoint_data,
        DDS_KeyHash_t *keyhash,
        const LTMTopicDetails *instance)
    {
        struct RTICdrStream * md5Stream = NULL;
        struct RTICdrStreamState cdrState;
        char * buffer = NULL;

        RTICdrStreamState_init(&cdrState);
        md5Stream = PRESTypePluginDefaultEndpointData_getMD5Stream(endpoint_data);

        if (md5Stream == NULL) {
            return RTI_FALSE;
        }

        RTICdrStream_resetPosition(md5Stream);
        RTICdrStream_setDirtyBit(md5Stream, RTI_TRUE);

        if (!ltm_dds::LTMTopicDetailsPlugin_serialize_key(
            endpoint_data,instance,md5Stream, RTI_FALSE, RTI_CDR_ENCAPSULATION_ID_CDR_BE, RTI_TRUE,NULL)) {

            int size;

            RTICdrStream_pushState(md5Stream, &cdrState, -1);

            size = (int)ltm_dds::LTMTopicDetailsPlugin_get_serialized_sample_size(
                endpoint_data,
                RTI_FALSE,
                RTI_CDR_ENCAPSULATION_ID_CDR_BE,
                0,
                instance);

            if (size <= RTICdrStream_getBufferLength(md5Stream)) {
                RTICdrStream_popState(md5Stream, &cdrState);        
                return RTI_FALSE;
            }   

            RTIOsapiHeap_allocateBuffer(&buffer,size,0);

            if (buffer == NULL) {
                RTICdrStream_popState(md5Stream, &cdrState);
                return RTI_FALSE;
            }

            RTICdrStream_set(md5Stream, buffer, size);
            RTIOsapiMemory_zero(
                RTICdrStream_getBuffer(md5Stream),
                RTICdrStream_getBufferLength(md5Stream));
            RTICdrStream_resetPosition(md5Stream);
            RTICdrStream_setDirtyBit(md5Stream, RTI_TRUE);
            if (!ltm_dds::LTMTopicDetailsPlugin_serialize_key(
                endpoint_data,instance,md5Stream, RTI_FALSE, RTI_CDR_ENCAPSULATION_ID_CDR_BE, RTI_TRUE,NULL)) 
            {
                RTICdrStream_popState(md5Stream, &cdrState);
                RTIOsapiHeap_freeBuffer(buffer);
                return RTI_FALSE;
            }        
        }   

        if (PRESTypePluginDefaultEndpointData_getMaxSizeSerializedKey(endpoint_data) > (unsigned int)(MIG_RTPS_KEY_HASH_MAX_LENGTH)) {
            RTICdrStream_computeMD5(md5Stream, keyhash->value);
        } else {
            RTIOsapiMemory_zero(keyhash->value,MIG_RTPS_KEY_HASH_MAX_LENGTH);
            RTIOsapiMemory_copy(
                keyhash->value, 
                RTICdrStream_getBuffer(md5Stream), 
                RTICdrStream_getCurrentPositionOffset(md5Stream));
        }

        keyhash->length = MIG_RTPS_KEY_HASH_MAX_LENGTH;

        if (buffer != NULL) {
            RTICdrStream_popState(md5Stream, &cdrState);
            RTIOsapiHeap_freeBuffer(buffer);
        }
        return RTI_TRUE;
    }

    RTIBool 
    LTMTopicDetailsPlugin_serialized_sample_to_keyhash(
        PRESTypePluginEndpointData endpoint_data,
        struct RTICdrStream *stream, 
        DDS_KeyHash_t *keyhash,
        RTIBool deserialize_encapsulation,
        void *endpoint_plugin_qos) 
    {   
        char * position = NULL;

        RTIBool done = RTI_FALSE;
        RTIBool error = RTI_FALSE;
        LTMTopicDetails * sample=NULL;

        if (endpoint_plugin_qos) {} /* To avoid warnings */
        if (stream == NULL) {
            error = RTI_TRUE;
            goto fin;
        }

        if(deserialize_encapsulation) {
            if (!RTICdrStream_deserializeAndSetCdrEncapsulation(stream)) {
                return RTI_FALSE;
            }

            position = RTICdrStream_resetAlignment(stream);
        }

        sample = (LTMTopicDetails *)
        PRESTypePluginDefaultEndpointData_getTempSample(endpoint_data);

        if (sample == NULL) {
            return RTI_FALSE;
        }

        if (!ltm_dds::LTMTimestampPlugin_skip(
            endpoint_data,
            stream, 
            RTI_FALSE, RTI_TRUE, 
            endpoint_plugin_qos)) {
            goto fin; 
        }
        if (!RTICdrStream_deserializeUnsignedLong(
            stream, &sample->topicID_)) {
            return RTI_FALSE;
        }
        done = RTI_TRUE;
      fin:
        if(!error) {
            if (done != RTI_TRUE && 
            RTICdrStream_getRemainder(stream) >=
            RTI_CDR_PARAMETER_HEADER_ALIGNMENT) {
                return RTI_FALSE;   
            }
        } else {
            return error;
        } 

        if(deserialize_encapsulation) {
            RTICdrStream_restoreAlignment(stream,position);
        }

        if (!ltm_dds::LTMTopicDetailsPlugin_instance_to_keyhash(
            endpoint_data, keyhash, sample)) {
            return RTI_FALSE;
        }

        return RTI_TRUE;
    }

    /* ------------------------------------------------------------------------
    * Plug-in Installation Methods
    * ------------------------------------------------------------------------ */
    struct PRESTypePlugin *LTMTopicDetailsPlugin_new(void) 
    { 
        struct PRESTypePlugin *plugin = NULL;
        const struct PRESTypePluginVersion PLUGIN_VERSION = 
        PRES_TYPE_PLUGIN_VERSION_2_0;

        RTIOsapiHeap_allocateStructure(
            &plugin, struct PRESTypePlugin);
        if (plugin == NULL) {
            return NULL;
        }

        plugin->version = PLUGIN_VERSION;

        /* set up parent's function pointers */
        plugin->onParticipantAttached =
        (PRESTypePluginOnParticipantAttachedCallback)
        ltm_dds::LTMTopicDetailsPlugin_on_participant_attached;
        plugin->onParticipantDetached =
        (PRESTypePluginOnParticipantDetachedCallback)
        ltm_dds::LTMTopicDetailsPlugin_on_participant_detached;
        plugin->onEndpointAttached =
        (PRESTypePluginOnEndpointAttachedCallback)
        ltm_dds::LTMTopicDetailsPlugin_on_endpoint_attached;
        plugin->onEndpointDetached =
        (PRESTypePluginOnEndpointDetachedCallback)
        ltm_dds::LTMTopicDetailsPlugin_on_endpoint_detached;

        plugin->copySampleFnc =
        (PRESTypePluginCopySampleFunction)
        ltm_dds::LTMTopicDetailsPlugin_copy_sample;
        plugin->createSampleFnc =
        (PRESTypePluginCreateSampleFunction)
        LTMTopicDetailsPlugin_create_sample;
        plugin->destroySampleFnc =
        (PRESTypePluginDestroySampleFunction)
        LTMTopicDetailsPlugin_destroy_sample;

        plugin->serializeFnc =
        (PRESTypePluginSerializeFunction)
        ltm_dds::LTMTopicDetailsPlugin_serialize;
        plugin->deserializeFnc =
        (PRESTypePluginDeserializeFunction)
        ltm_dds::LTMTopicDetailsPlugin_deserialize;
        plugin->getSerializedSampleMaxSizeFnc =
        (PRESTypePluginGetSerializedSampleMaxSizeFunction)
        ltm_dds::LTMTopicDetailsPlugin_get_serialized_sample_max_size;
        plugin->getSerializedSampleMinSizeFnc =
        (PRESTypePluginGetSerializedSampleMinSizeFunction)
        ltm_dds::LTMTopicDetailsPlugin_get_serialized_sample_min_size;

        plugin->getSampleFnc =
        (PRESTypePluginGetSampleFunction)
        LTMTopicDetailsPlugin_get_sample;
        plugin->returnSampleFnc =
        (PRESTypePluginReturnSampleFunction)
        LTMTopicDetailsPlugin_return_sample;

        plugin->getKeyKindFnc =
        (PRESTypePluginGetKeyKindFunction)
        ltm_dds::LTMTopicDetailsPlugin_get_key_kind;

        plugin->getSerializedKeyMaxSizeFnc =   
        (PRESTypePluginGetSerializedKeyMaxSizeFunction)
        ltm_dds::LTMTopicDetailsPlugin_get_serialized_key_max_size;
        plugin->serializeKeyFnc =
        (PRESTypePluginSerializeKeyFunction)
        ltm_dds::LTMTopicDetailsPlugin_serialize_key;
        plugin->deserializeKeyFnc =
        (PRESTypePluginDeserializeKeyFunction)
        ltm_dds::LTMTopicDetailsPlugin_deserialize_key;
        plugin->deserializeKeySampleFnc =
        (PRESTypePluginDeserializeKeySampleFunction)
        ltm_dds::LTMTopicDetailsPlugin_deserialize_key_sample;

        plugin-> instanceToKeyHashFnc = 
        (PRESTypePluginInstanceToKeyHashFunction)
        ltm_dds::LTMTopicDetailsPlugin_instance_to_keyhash;
        plugin->serializedSampleToKeyHashFnc = 
        (PRESTypePluginSerializedSampleToKeyHashFunction)
        ltm_dds::LTMTopicDetailsPlugin_serialized_sample_to_keyhash;

        plugin->getKeyFnc =
        (PRESTypePluginGetKeyFunction)
        LTMTopicDetailsPlugin_get_key;
        plugin->returnKeyFnc =
        (PRESTypePluginReturnKeyFunction)
        LTMTopicDetailsPlugin_return_key;

        plugin->instanceToKeyFnc =
        (PRESTypePluginInstanceToKeyFunction)
        ltm_dds::LTMTopicDetailsPlugin_instance_to_key;
        plugin->keyToInstanceFnc =
        (PRESTypePluginKeyToInstanceFunction)
        ltm_dds::LTMTopicDetailsPlugin_key_to_instance;
        plugin->serializedKeyToKeyHashFnc = NULL; /* Not supported yet */
        plugin->typeCode =  (struct RTICdrTypeCode *)ltm_dds::LTMTopicDetails_get_typecode();

        plugin->languageKind = PRES_TYPEPLUGIN_CPP_LANG;

        /* Serialized buffer */
        plugin->getBuffer = 
        (PRESTypePluginGetBufferFunction)
        LTMTopicDetailsPlugin_get_buffer;
        plugin->returnBuffer = 
        (PRESTypePluginReturnBufferFunction)
        LTMTopicDetailsPlugin_return_buffer;
        plugin->getSerializedSampleSizeFnc =
        (PRESTypePluginGetSerializedSampleSizeFunction)
        ltm_dds::LTMTopicDetailsPlugin_get_serialized_sample_size;

        plugin->endpointTypeName = LTMTopicDetailsTYPENAME;

        return plugin;
    }

    void
    LTMTopicDetailsPlugin_delete(struct PRESTypePlugin *plugin)
    {
        RTIOsapiHeap_freeStructure(plugin);
    } 

    /* ----------------------------------------------------------------------------
    *  Type LTMTopicDetailsList
    * -------------------------------------------------------------------------- */

    /* -----------------------------------------------------------------------------
    Support functions:
    * -------------------------------------------------------------------------- */

    LTMTopicDetailsList*
    LTMTopicDetailsListPluginSupport_create_data_w_params(
        const struct DDS_TypeAllocationParams_t * alloc_params){
        LTMTopicDetailsList *sample = NULL;

        RTIOsapiHeap_allocateStructure(
            &sample, LTMTopicDetailsList);

        if(sample != NULL) {
            if (!ltm_dds::LTMTopicDetailsList_initialize_w_params(sample,alloc_params)) {
                RTIOsapiHeap_freeStructure(sample);
                return NULL;
            }
        }        
        return sample; 
    } 

    LTMTopicDetailsList *
    LTMTopicDetailsListPluginSupport_create_data_ex(RTIBool allocate_pointers){
        LTMTopicDetailsList *sample = NULL;

        RTIOsapiHeap_allocateStructure(
            &sample, LTMTopicDetailsList);

        if(sample != NULL) {
            if (!ltm_dds::LTMTopicDetailsList_initialize_ex(sample,allocate_pointers, RTI_TRUE)) {
                RTIOsapiHeap_freeStructure(sample);
                return NULL;
            }
        }
        return sample; 
    }

    LTMTopicDetailsList *
    LTMTopicDetailsListPluginSupport_create_data(void)
    {
        return ltm_dds::LTMTopicDetailsListPluginSupport_create_data_ex(RTI_TRUE);
    }

    void 
    LTMTopicDetailsListPluginSupport_destroy_data_w_params(
        LTMTopicDetailsList *sample,
        const struct DDS_TypeDeallocationParams_t * dealloc_params) {

        ltm_dds::LTMTopicDetailsList_finalize_w_params(sample,dealloc_params);

        RTIOsapiHeap_freeStructure(sample);
    }

    void 
    LTMTopicDetailsListPluginSupport_destroy_data_ex(
        LTMTopicDetailsList *sample,RTIBool deallocate_pointers) {

        ltm_dds::LTMTopicDetailsList_finalize_ex(sample,deallocate_pointers);

        RTIOsapiHeap_freeStructure(sample);
    }

    void 
    LTMTopicDetailsListPluginSupport_destroy_data(
        LTMTopicDetailsList *sample) {

        ltm_dds::LTMTopicDetailsListPluginSupport_destroy_data_ex(sample,RTI_TRUE);

    }

    RTIBool 
    LTMTopicDetailsListPluginSupport_copy_data(
        LTMTopicDetailsList *dst,
        const LTMTopicDetailsList *src)
    {
        return ltm_dds::LTMTopicDetailsList_copy(dst,src);
    }

    void 
    LTMTopicDetailsListPluginSupport_print_data(
        const LTMTopicDetailsList *sample,
        const char *desc,
        unsigned int indent_level)
    {

        RTICdrType_printIndent(indent_level);

        if (desc != NULL) {
            RTILog_debug("%s:\n", desc);
        } else {
            RTILog_debug("\n");
        }

        if (sample == NULL) {
            RTILog_debug("NULL\n");
            return;
        }

        if (ltm_dds::LTMTopicDetailsSeq_get_contiguous_bufferI(sample) != NULL) {
            RTICdrType_printArray(
                ltm_dds::LTMTopicDetailsSeq_get_contiguous_bufferI(sample), 
                ltm_dds::LTMTopicDetailsSeq_get_length(sample),
                sizeof(ltm_dds::LTMTopicDetails),
                (RTICdrTypePrintFunction)ltm_dds::LTMTopicDetailsPluginSupport_print_data,
                "", indent_level + 1);
        } else {
            RTICdrType_printPointerArray(
                ltm_dds::LTMTopicDetailsSeq_get_discontiguous_bufferI(sample), 
                ltm_dds::LTMTopicDetailsSeq_get_length(sample),
                (RTICdrTypePrintFunction)ltm_dds::LTMTopicDetailsPluginSupport_print_data,
                "", indent_level + 1);
        }

    }

    /* ----------------------------------------------------------------------------
    Callback functions:
    * ---------------------------------------------------------------------------- */

    RTIBool 
    LTMTopicDetailsListPlugin_copy_sample(
        PRESTypePluginEndpointData endpoint_data,
        LTMTopicDetailsList *dst,
        const LTMTopicDetailsList *src)
    {
        if (endpoint_data) {} /* To avoid warnings */
        return ltm_dds::LTMTopicDetailsListPluginSupport_copy_data(dst,src);
    }

    /* ----------------------------------------------------------------------------
    (De)Serialize functions:
    * ------------------------------------------------------------------------- */
    unsigned int 
    LTMTopicDetailsListPlugin_get_serialized_sample_max_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment);

    RTIBool 
    LTMTopicDetailsListPlugin_serialize(
        PRESTypePluginEndpointData endpoint_data,
        const LTMTopicDetailsList *sample, 
        struct RTICdrStream *stream,    
        RTIBool serialize_encapsulation,
        RTIEncapsulationId encapsulation_id,
        RTIBool serialize_sample, 
        void *endpoint_plugin_qos)
    {
        char * position = NULL;
        RTIBool retval = RTI_TRUE;

        if (endpoint_data) {} /* To avoid warnings */
        if (endpoint_plugin_qos) {} /* To avoid warnings */

        if(serialize_encapsulation) {
            if (!RTICdrStream_serializeAndSetCdrEncapsulation(stream , encapsulation_id)) {
                return RTI_FALSE;
            }

            position = RTICdrStream_resetAlignment(stream);
        }

        if(serialize_sample) {

            if (ltm_dds::LTMTopicDetailsSeq_get_contiguous_bufferI(sample) != NULL) {
                if (!RTICdrStream_serializeNonPrimitiveSequence(
                    stream,
                    ltm_dds::LTMTopicDetailsSeq_get_contiguous_bufferI(sample),
                    ltm_dds::LTMTopicDetailsSeq_get_length(sample),
                    (10),
                    sizeof(ltm_dds::LTMTopicDetails),
                    (RTICdrStreamSerializeFunction)ltm_dds::LTMTopicDetailsPlugin_serialize,
                    RTI_FALSE, encapsulation_id,RTI_TRUE,
                    endpoint_data,endpoint_plugin_qos)) {
                    return RTI_FALSE;
                } 
            } else {
                if (!RTICdrStream_serializeNonPrimitivePointerSequence(
                    stream,
                    (const void **) ltm_dds::LTMTopicDetailsSeq_get_discontiguous_bufferI(sample),
                    ltm_dds::LTMTopicDetailsSeq_get_length(sample),
                    (10),
                    (RTICdrStreamSerializeFunction)ltm_dds::LTMTopicDetailsPlugin_serialize,
                    RTI_FALSE, encapsulation_id,RTI_TRUE,
                    endpoint_data,endpoint_plugin_qos)) {
                    return RTI_FALSE;
                } 

            } 

        }

        if(serialize_encapsulation) {
            RTICdrStream_restoreAlignment(stream,position);
        }

        return retval;
    }

    RTIBool 
    LTMTopicDetailsListPlugin_deserialize_sample(
        PRESTypePluginEndpointData endpoint_data,
        LTMTopicDetailsList *sample,
        struct RTICdrStream *stream,   
        RTIBool deserialize_encapsulation,
        RTIBool deserialize_sample, 
        void *endpoint_plugin_qos)
    {

        char * position = NULL;

        if (endpoint_data) {} /* To avoid warnings */
        if (endpoint_plugin_qos) {} /* To avoid warnings */
        if(deserialize_encapsulation) {

            if (!RTICdrStream_deserializeAndSetCdrEncapsulation(stream)) {
                return RTI_FALSE;
            }

            position = RTICdrStream_resetAlignment(stream);
        }
        if(deserialize_sample) {

            ltm_dds::LTMTopicDetailsList_initialize_ex(sample, RTI_FALSE, RTI_FALSE);

            {
                RTICdrUnsignedLong sequence_length;
                if (ltm_dds::LTMTopicDetailsSeq_get_contiguous_bufferI(sample) != NULL) {
                    if (!RTICdrStream_deserializeNonPrimitiveSequence(
                        stream,
                        ltm_dds::LTMTopicDetailsSeq_get_contiguous_bufferI(sample),
                        &sequence_length,
                        ltm_dds::LTMTopicDetailsSeq_get_maximum(sample),
                        sizeof(ltm_dds::LTMTopicDetails),
                        (RTICdrStreamDeserializeFunction)ltm_dds::LTMTopicDetailsPlugin_deserialize_sample,
                        RTI_FALSE,RTI_TRUE,
                        endpoint_data,endpoint_plugin_qos)) {
                        return RTI_FALSE;
                    }
                } else {
                    if (!RTICdrStream_deserializeNonPrimitivePointerSequence(
                        stream,
                        (void **) ltm_dds::LTMTopicDetailsSeq_get_discontiguous_bufferI(sample),
                        &sequence_length,
                        ltm_dds::LTMTopicDetailsSeq_get_maximum(sample),
                        (RTICdrStreamDeserializeFunction)ltm_dds::LTMTopicDetailsPlugin_deserialize_sample,
                        RTI_FALSE,RTI_TRUE,
                        endpoint_data,endpoint_plugin_qos)) {
                        return RTI_FALSE;
                    }
                }
                if (!ltm_dds::LTMTopicDetailsSeq_set_length(
                    sample,sequence_length)) {
                    return RTI_FALSE;
                }        

            }
        }

        if(deserialize_encapsulation) {
            RTICdrStream_restoreAlignment(stream,position);
        }

        return RTI_TRUE;
    }

    RTIBool LTMTopicDetailsListPlugin_skip(
        PRESTypePluginEndpointData endpoint_data,
        struct RTICdrStream *stream,   
        RTIBool skip_encapsulation,
        RTIBool skip_sample, 
        void *endpoint_plugin_qos)
    {
        char * position = NULL;

        if (endpoint_data) {} /* To avoid warnings */
        if (endpoint_plugin_qos) {} /* To avoid warnings */

        if(skip_encapsulation) {
            if (!RTICdrStream_skipEncapsulation(stream)) {
                return RTI_FALSE;
            }

            position = RTICdrStream_resetAlignment(stream);
        }

        if (skip_sample) {

            {
                RTICdrUnsignedLong sequence_length;
                if (!RTICdrStream_skipNonPrimitiveSequence(
                    stream,
                    &sequence_length,
                    sizeof(ltm_dds::LTMTopicDetails),
                    (RTICdrStreamSkipFunction)ltm_dds::LTMTopicDetailsPlugin_skip,
                    RTI_FALSE,RTI_TRUE,
                    endpoint_data,endpoint_plugin_qos)) {
                    return RTI_FALSE;
                }
            }
        }

        if(skip_encapsulation) {
            RTICdrStream_restoreAlignment(stream,position);
        }

        return RTI_TRUE;
    }

    unsigned int 
    LTMTopicDetailsListPlugin_get_serialized_sample_max_size_ex(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool * overflow,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment)
    {

        unsigned int initial_alignment = current_alignment;

        unsigned int encapsulation_size = current_alignment;

        if (endpoint_data) {} /* To avoid warnings */ 
        if (overflow) {} /* To avoid warnings */

        if (include_encapsulation) {

            if (!RTICdrEncapsulation_validEncapsulationId(encapsulation_id)) {
                return 1;
            }
            RTICdrStream_getEncapsulationSize(encapsulation_size);
            encapsulation_size -= current_alignment;
            current_alignment = 0;
            initial_alignment = 0;
        }

        current_alignment +=RTICdrType_getNonPrimitiveSequenceMaxSizeSerializedEx(
            overflow,
            current_alignment, (10),
            ltm_dds::LTMTopicDetailsPlugin_get_serialized_sample_max_size_ex,
            RTI_FALSE,encapsulation_id,endpoint_data);

        if (include_encapsulation) {
            current_alignment += encapsulation_size;
        }
        return  current_alignment - initial_alignment;
    }

    unsigned int 
    LTMTopicDetailsListPlugin_get_serialized_sample_max_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment)
    {
        unsigned int size;
        RTIBool overflow = RTI_FALSE;

        size = LTMTopicDetailsListPlugin_get_serialized_sample_max_size_ex(
            endpoint_data,&overflow,include_encapsulation,encapsulation_id,current_alignment);

        if (overflow) {
            size = RTI_CDR_MAX_SERIALIZED_SIZE;
        }

        return size;
    }

    unsigned int 
    LTMTopicDetailsListPlugin_get_serialized_sample_min_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment)
    {

        unsigned int initial_alignment = current_alignment;

        unsigned int encapsulation_size = current_alignment;

        if (endpoint_data) {} /* To avoid warnings */ 

        if (include_encapsulation) {

            if (!RTICdrEncapsulation_validEncapsulationId(encapsulation_id)) {
                return 1;
            }
            RTICdrStream_getEncapsulationSize(encapsulation_size);
            encapsulation_size -= current_alignment;
            current_alignment = 0;
            initial_alignment = 0;
        }

        current_alignment +=RTICdrType_getNonPrimitiveSequenceMaxSizeSerialized(
            current_alignment, 0,
            ltm_dds::LTMTopicDetailsPlugin_get_serialized_sample_min_size,
            RTI_FALSE,encapsulation_id,endpoint_data);

        if (include_encapsulation) {
            current_alignment += encapsulation_size;
        }
        return  current_alignment - initial_alignment;
    }

    /* Returns the size of the sample in its serialized form (in bytes).
    * It can also be an estimation in excess of the real buffer needed 
    * during a call to the serialize() function.
    * The value reported does not have to include the space for the
    * encapsulation flags.
    */
    unsigned int
    LTMTopicDetailsListPlugin_get_serialized_sample_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment,
        const LTMTopicDetailsList * sample) 
    {

        unsigned int initial_alignment = current_alignment;

        unsigned int encapsulation_size = current_alignment;

        if (endpoint_data) {} /* To avoid warnings */ 
        if (sample==NULL) {
            return 0;
        }

        if (include_encapsulation) {

            if (!RTICdrEncapsulation_validEncapsulationId(encapsulation_id)) {
                return 1;
            }
            RTICdrStream_getEncapsulationSize(encapsulation_size);
            encapsulation_size -= current_alignment;
            current_alignment = 0;
            initial_alignment = 0;
        }

        if (ltm_dds::LTMTopicDetailsSeq_get_contiguous_bufferI(sample) != NULL) {
            current_alignment += RTICdrStream_getNonPrimitiveSequenceSerializedSize(
                current_alignment, ltm_dds::LTMTopicDetailsSeq_get_length(sample),
                sizeof(ltm_dds::LTMTopicDetails),
                (RTICdrTypeGetSerializedSampleSizeFunction)ltm_dds::LTMTopicDetailsPlugin_get_serialized_sample_size,
                RTI_FALSE,encapsulation_id,
                ltm_dds::LTMTopicDetailsSeq_get_contiguous_bufferI(sample),
                endpoint_data);
        } else {
            current_alignment += RTICdrStream_getNonPrimitivePointerSequenceSerializedSize(
                current_alignment,  
                ltm_dds::LTMTopicDetailsSeq_get_length(sample),
                sizeof(ltm_dds::LTMTopicDetails),
                (RTICdrTypeGetSerializedSampleSizeFunction)ltm_dds::LTMTopicDetailsPlugin_get_serialized_sample_size,
                RTI_FALSE,encapsulation_id,
                (const void **)ltm_dds::LTMTopicDetailsSeq_get_discontiguous_bufferI(sample),
                endpoint_data);
        }

        if (include_encapsulation) {
            current_alignment += encapsulation_size;
        }
        return current_alignment - initial_alignment;
    }

    /* --------------------------------------------------------------------------------------
    Key Management functions:
    * -------------------------------------------------------------------------------------- */

    PRESTypePluginKeyKind 
    LTMTopicDetailsListPlugin_get_key_kind(void)
    {
        return PRES_TYPEPLUGIN_NO_KEY;
    }

    RTIBool 
    LTMTopicDetailsListPlugin_serialize_key(
        PRESTypePluginEndpointData endpoint_data,
        const LTMTopicDetailsList *sample, 
        struct RTICdrStream *stream,    
        RTIBool serialize_encapsulation,
        RTIEncapsulationId encapsulation_id,
        RTIBool serialize_key,
        void *endpoint_plugin_qos)
    {
        char * position = NULL;

        if(serialize_encapsulation) {
            if (!RTICdrStream_serializeAndSetCdrEncapsulation(stream , encapsulation_id)) {
                return RTI_FALSE;
            }

            position = RTICdrStream_resetAlignment(stream);
        }

        if(serialize_key) {

            if (ltm_dds::LTMTopicDetailsSeq_get_contiguous_bufferI(sample) != NULL) {
                if (!RTICdrStream_serializeNonPrimitiveSequence(
                    stream,
                    ltm_dds::LTMTopicDetailsSeq_get_contiguous_bufferI(sample),
                    ltm_dds::LTMTopicDetailsSeq_get_length(sample),
                    (10),
                    sizeof(ltm_dds::LTMTopicDetails),
                    (RTICdrStreamSerializeFunction)ltm_dds::LTMTopicDetailsPlugin_serialize_key,
                    RTI_FALSE, encapsulation_id,RTI_TRUE,
                    endpoint_data,endpoint_plugin_qos)) {
                    return RTI_FALSE;
                } 
            } else {
                if (!RTICdrStream_serializeNonPrimitivePointerSequence(
                    stream,
                    (const void **) ltm_dds::LTMTopicDetailsSeq_get_discontiguous_bufferI(sample),
                    ltm_dds::LTMTopicDetailsSeq_get_length(sample),
                    (10),
                    (RTICdrStreamSerializeFunction)ltm_dds::LTMTopicDetailsPlugin_serialize_key,
                    RTI_FALSE, encapsulation_id,RTI_TRUE,
                    endpoint_data,endpoint_plugin_qos)) {
                    return RTI_FALSE;
                } 

            } 

        }

        if(serialize_encapsulation) {
            RTICdrStream_restoreAlignment(stream,position);
        }

        return RTI_TRUE;
    }

    RTIBool LTMTopicDetailsListPlugin_deserialize_key_sample(
        PRESTypePluginEndpointData endpoint_data,
        LTMTopicDetailsList *sample, 
        struct RTICdrStream *stream,
        RTIBool deserialize_encapsulation,
        RTIBool deserialize_key,
        void *endpoint_plugin_qos)
    {
        char * position = NULL;

        if (endpoint_data) {} /* To avoid warnings */
        if (endpoint_plugin_qos) {} /* To avoid warnings */

        if(deserialize_encapsulation) {

            if (!RTICdrStream_deserializeAndSetCdrEncapsulation(stream)) {
                return RTI_FALSE;
            }

            position = RTICdrStream_resetAlignment(stream);
        }
        if (deserialize_key) {

            {
                RTICdrUnsignedLong sequence_length;
                if (ltm_dds::LTMTopicDetailsSeq_get_contiguous_bufferI(sample) != NULL) {
                    if (!RTICdrStream_deserializeNonPrimitiveSequence(
                        stream,
                        ltm_dds::LTMTopicDetailsSeq_get_contiguous_bufferI(sample),
                        &sequence_length,
                        ltm_dds::LTMTopicDetailsSeq_get_maximum(sample),
                        sizeof(ltm_dds::LTMTopicDetails),
                        (RTICdrStreamDeserializeFunction)ltm_dds::LTMTopicDetailsPlugin_deserialize_key_sample,
                        RTI_FALSE,RTI_TRUE,
                        endpoint_data,endpoint_plugin_qos)) {
                        return RTI_FALSE;
                    }
                } else {
                    if (!RTICdrStream_deserializeNonPrimitivePointerSequence(
                        stream,
                        (void **) ltm_dds::LTMTopicDetailsSeq_get_discontiguous_bufferI(sample),
                        &sequence_length,
                        ltm_dds::LTMTopicDetailsSeq_get_maximum(sample),
                        (RTICdrStreamDeserializeFunction)ltm_dds::LTMTopicDetailsPlugin_deserialize_key_sample,
                        RTI_FALSE,RTI_TRUE,
                        endpoint_data,endpoint_plugin_qos)) {
                        return RTI_FALSE;
                    }
                }
                if (!ltm_dds::LTMTopicDetailsSeq_set_length(
                    sample,sequence_length)) {
                    return RTI_FALSE;
                }        

            }
        }

        if(deserialize_encapsulation) {
            RTICdrStream_restoreAlignment(stream,position);
        }

        return RTI_TRUE;
    }

    unsigned int
    LTMTopicDetailsListPlugin_get_serialized_key_max_size_ex(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool * overflow,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment)
    {

        unsigned int initial_alignment = current_alignment;

        unsigned int encapsulation_size = current_alignment;

        if (endpoint_data) {} /* To avoid warnings */
        if (overflow) {} /* To avoid warnings */

        if (include_encapsulation) {

            if (!RTICdrEncapsulation_validEncapsulationId(encapsulation_id)) {
                return 1;
            }
            RTICdrStream_getEncapsulationSize(encapsulation_size);
            encapsulation_size -= current_alignment;
            current_alignment = 0;
            initial_alignment = 0;
        }

        current_alignment +=RTICdrType_getNonPrimitiveSequenceMaxSizeSerializedEx(
            overflow,
            current_alignment, (10),
            ltm_dds::LTMTopicDetailsPlugin_get_serialized_key_max_size_ex,
            RTI_FALSE,encapsulation_id,endpoint_data);

        if (include_encapsulation) {
            current_alignment += encapsulation_size;
        }
        return current_alignment - initial_alignment;
    }

    unsigned int
    LTMTopicDetailsListPlugin_get_serialized_key_max_size(
        PRESTypePluginEndpointData endpoint_data,
        RTIBool include_encapsulation,
        RTIEncapsulationId encapsulation_id,
        unsigned int current_alignment)
    {
        unsigned int size;
        RTIBool overflow = RTI_FALSE;

        size = LTMTopicDetailsListPlugin_get_serialized_key_max_size_ex(
            endpoint_data,&overflow,include_encapsulation,encapsulation_id,current_alignment);

        if (overflow) {
            size = RTI_CDR_MAX_SERIALIZED_SIZE;
        }

        return size;
    }

    RTIBool 
    LTMTopicDetailsListPlugin_serialized_sample_to_key(
        PRESTypePluginEndpointData endpoint_data,
        LTMTopicDetailsList *sample,
        struct RTICdrStream *stream, 
        RTIBool deserialize_encapsulation,  
        RTIBool deserialize_key, 
        void *endpoint_plugin_qos)
    {
        char * position = NULL;

        if (endpoint_data) {} /* To avoid warnings */
        if (endpoint_plugin_qos) {} /* To avoid warnings */

        if(deserialize_encapsulation) {
            if (!RTICdrStream_deserializeAndSetCdrEncapsulation(stream)) {
                return RTI_FALSE;
            }
            position = RTICdrStream_resetAlignment(stream);
        }

        if (deserialize_key) {

            {
                RTICdrUnsignedLong sequence_length;
                if (ltm_dds::LTMTopicDetailsSeq_get_contiguous_bufferI(sample) != NULL) {
                    if (!RTICdrStream_deserializeNonPrimitiveSequence(
                        stream,
                        ltm_dds::LTMTopicDetailsSeq_get_contiguous_bufferI(sample),
                        &sequence_length,
                        ltm_dds::LTMTopicDetailsSeq_get_maximum(sample),
                        sizeof(ltm_dds::LTMTopicDetails),
                        (RTICdrStreamDeserializeFunction)ltm_dds::LTMTopicDetailsPlugin_serialized_sample_to_key,
                        RTI_FALSE,RTI_TRUE,
                        endpoint_data,endpoint_plugin_qos)) {
                        return RTI_FALSE;
                    }
                } else {
                    if (!RTICdrStream_deserializeNonPrimitivePointerSequence(
                        stream,
                        (void **) ltm_dds::LTMTopicDetailsSeq_get_discontiguous_bufferI(sample),
                        &sequence_length,
                        ltm_dds::LTMTopicDetailsSeq_get_maximum(sample),
                        (RTICdrStreamDeserializeFunction)ltm_dds::LTMTopicDetailsPlugin_serialized_sample_to_key,
                        RTI_FALSE,RTI_TRUE,
                        endpoint_data,endpoint_plugin_qos)) {
                        return RTI_FALSE;
                    }
                }
                if (!ltm_dds::LTMTopicDetailsSeq_set_length(
                    sample,sequence_length)) {
                    return RTI_FALSE;
                }        

            }

        }

        if(deserialize_encapsulation) {
            RTICdrStream_restoreAlignment(stream,position);
        }

        return RTI_TRUE;
    }

    /* ------------------------------------------------------------------------
    * Plug-in Installation Methods
    * ------------------------------------------------------------------------ */
} /* namespace ltm_dds  */

