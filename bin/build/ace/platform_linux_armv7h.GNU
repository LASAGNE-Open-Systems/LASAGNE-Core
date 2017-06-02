# -*- Makefile -*-
# $Id: platform_cubox.GNU 97320 2013-09-05 07:53:58Z derekd $

no_hidden_visibility ?= 1
inline ?= 1

include $(ACE_ROOT)/include/makeinclude/platform_linux_common.GNU

SYSARCH ?= $(shell uname -m)

ifeq ($(insure),0)
  CC  ?= gcc
  CXX ?= g++
  CXX_FOR_VERSION_TEST ?= $(CXX)
else
  CXX_FOR_VERSION_TEST ?= g++
endif

CXX_HOSTMACHINE := $(shell $(CXX_FOR_VERSION_TEST) -dumpmachine)

# set archtecture specific build flags
ifeq (armv7,$(findstring armv7,$(SYSARCH)))
  FLAGS_C_CC += -march=armv7-a
  LDFLAGS    += -march=armv7-a
endif

ifeq (abihf,$(findstring abihf,$(CXX_HOSTMACHINE)))
  FLAGS_C_CC += -mfloat-abi=hard
endif

ifeq ($(buildbits),32)
  FLAGS_C_CC += -m32
  LDFLAGS    += -m32
endif
ifeq ($(buildbits),64)
  FLAGS_C_CC += -m64
  LDFLAGS    += -m64
endif

# DT_RUNPATH is preferred over DT_RPATH, but the linker will only use it when
# this extra flag option (enable-new-dtags) is present
LD_RPATH_FLAGS += -Wl,--enable-new-dtags

# Rely on _GNU_SOURCE to set these defaults defined in /usr/include/features.h
# instead of setting them directly here (older versions of gcc don't set it
# for you): _SVID_SOURCE _BSD_SOURCE _POSIX_SOURCE _POSIX_C_SOURCE=199506L, ...
CPPFLAGS += -D_GNU_SOURCE

DCFLAGS  += -ggdb
DCCFLAGS += -ggdb
DLD      = $(CXX)
LD       = $(CXX)
LIBS     += -ldl

ifeq ($(threads),1)
  FLAGS_C_CC += -pthread
  SOFLAGS += -pthread
  LIBS += -lrt
endif

ifeq ($(optimize),1)
  SOFLAGS += -Wl,-O3
endif

SOFLAGS += -shared
SOBUILD = $(COMPILE.cc) $(PIC) -o $(VSHDIR)$*.so $<; \
          $(SOLINK.cc) -o $@ $(LDFLAGS) $(VSHDIR)$*.o
PRELIB  = @true

# Test for template instantiation, add to SOFLAGS if SONAME set,
# add -E to LDFLAGS if using GNU ld
#
include $(ACE_ROOT)/include/makeinclude/platform_g++_common.GNU

OCFLAGS ?= -O3
OCCFLAGS ?= -O3

# Added line below to support "Executable Shared Object" files (as
# needed by the service configurator).
# Marius Kjeldahl <mariusk@sn.no, marius@funcom.com>
ifeq ($(threads),1)
    ESOBUILD = $(COMPILEESO.cc) $(PIC) -shared -o $(VSHDIR)$*.so $<
    ifndef PRELIB
       PRELIB = @true
    endif # ! PRELIB
endif
