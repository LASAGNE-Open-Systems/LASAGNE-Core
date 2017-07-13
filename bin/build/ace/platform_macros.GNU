include $(DAF_ROOT)/MPC/config/TAF.features
include $(ACE_ROOT)/include/makeinclude/platform_linux.GNU

ifeq ($(findstring $(CXX_MAJOR_VERSION),6 7),$(CXX_MAJOR_VERSION))
 ifneq ($(no_deprecated_declarations),0)
  FLAGS_C_CC += -Wno-deprecated-declarations
 endif
endif

ifeq ($(findstring $(CXX_MAJOR_VERSION),7 8),$(CXX_MAJOR_VERSION))
 ifneq ($(no_implicit_fallthrough),0)
  FLAGS_C_CC += -Wno-implicit-fallthrough
 endif
 ifneq ($(no_unused_result),0)
  CCFLAGS += -Wno-unused-result
 endif
endif
