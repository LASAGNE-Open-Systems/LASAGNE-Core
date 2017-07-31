#ifndef __DAF_STREAM_MACRO_H
#define __DAF_STREAM_MACRO_H


#define DAF_STREAM_DECLARE(CLS,SERVICE_CLASS) \
extern "C" CLS##_Export ACE_Stream<ACE_SYNCH> * \
ACE_MAKE_SVC_CONFIG_FACTORY_NAME(ACE_VERSIONED_NAMESPACE_NAME,SERVICE_CLASS) (ACE_Service_Object_Exterminator *);


#define DAF_STREAM_DEFINE(CLS,SERVICE_CLASS) \
void ACE_MAKE_SVC_CONFIG_GOBBLER_NAME(ACE_VERSIONED_NAMESPACE_NAME,SERVICE_CLASS) (void *p) { \
  ACE_Stream<ACE_SYNCH> * _p = \
    static_cast< ACE_Stream<ACE_SYNCH> *> (p); \
  ACE_ASSERT (_p != 0); \
  delete _p; } \
extern "C" CLS##_Export ACE_Stream<ACE_SYNCH> *\
ACE_MAKE_SVC_CONFIG_FACTORY_NAME(ACE_VERSIONED_NAMESPACE_NAME,SERVICE_CLASS) (ACE_Service_Object_Exterminator *gobbler) \
{ \
  ACE_TRACE (#SERVICE_CLASS); \
  if (gobbler != 0) \
    *gobbler = (ACE_Service_Object_Exterminator) ACE_MAKE_SVC_CONFIG_GOBBLER_NAME(ACE_VERSIONED_NAMESPACE_NAME,SERVICE_CLASS); \
  return new SERVICE_CLASS; \
}


#define DAF_MODULE_DECLARE(CLS,SERVICE_CLASS) \
extern "C" CLS##_Export ACE_Module<ACE_SYNCH> * \
ACE_MAKE_SVC_CONFIG_FACTORY_NAME(ACE_VERSIONED_NAMESPACE_NAME,SERVICE_CLASS) (ACE_Service_Object_Exterminator *);

#if 0
#define DAF_MODULE_DEFINE(CLS,SERVICE_CLASS) \
void ACE_MAKE_SVC_CONFIG_GOBBLER_NAME(ACE_VERSIONED_NAMESPACE_NAME,SERVICE_CLASS) (void *p) { \
  ACE_Module<ACE_SYNCH> * _p = \
    static_cast< ACE_Module<ACE_SYNCH> *> (p); \
  ACE_ASSERT (_p != 0); \
  delete _p; } \
extern "C" CLS##_Export ACE_Module<ACE_SYNCH> *\
ACE_MAKE_SVC_CONFIG_FACTORY_NAME(ACE_VERSIONED_NAMESPACE_NAME,SERVICE_CLASS) (ACE_Service_Object_Exterminator *gobbler) \
{ \
  ACE_TRACE (#SERVICE_CLASS); \
  if (gobbler != 0) \
    *gobbler = (ACE_Service_Object_Exterminator) ACE_MAKE_SVC_CONFIG_GOBBLER_NAME(ACE_VERSIONED_NAMESPACE_NAME,SERVICE_CLASS); \
  return new SERVICE_CLASS; \
}
#else
#define DAF_MODULE_DEFINE(CLS,SERVICE_CLASS) \
void ACE_MAKE_SVC_CONFIG_GOBBLER_NAME(ACE_VERSIONED_NAMESPACE_NAME,SERVICE_CLASS) (void *p) { \
  ACE_Module<ACE_SYNCH> * _p = \
    static_cast< ACE_Module<ACE_SYNCH> *> (p); \
  ACE_ASSERT (_p != 0); \
  delete _p; } \
extern "C" CLS##_Export ACE_Module<ACE_SYNCH> *\
ACE_MAKE_SVC_CONFIG_FACTORY_NAME(ACE_VERSIONED_NAMESPACE_NAME,SERVICE_CLASS) (ACE_Service_Object_Exterminator *gobbler) \
{ \
  ACE_UNUSED_ARG(gobbler); \
  ACE_TRACE (#SERVICE_CLASS); \
  return new SERVICE_CLASS; \
}
#endif // #if0

//

#endif //__DAF_STREAM_MACRO_H
