#ifndef ACE_CONFIG_H
#define ACE_CONFIG_H

// This configures ACE infrastructure and is placed in $(ACE_ROOT)/ace/config.h

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define __ACE_INLINE__ 1                /* Always have inline on - also fixes a Solaris problem with linking templates */

#define ACE_HAS_STRICT 1
#define ACE_COMPILE_TIMEPROBES 1        /* Compile the timeprobe module */
#define ACE_ENABLE_SWAP_ON_WRITE 1
#define ACE_USES_CLASSIC_SVC_CONF 1
#define ACE_LACKS_DEPRECATED_MACROS 1
#define ACE_HAS_STANDARD_CPP_LIBRARY 1
//#define ACE_USE_THREAD_MANAGER_ADAPTER 1

# if defined(_MSC_VER)

#  define ACE_USE_SELECT_REACTOR_FOR_REACTOR_IMPL 1
#  define ACE_DISABLE_WIN32_ERROR_WINDOWS 1

#  if defined(_DEBUG)

#   define ACE_CRTDBG_MAP_ALLOC 1

//#   define _CRTDBG_MAP_ALLOC
//#   define _CRTDBG_MAP_ALLOC_NEW

#   define ACE_MALLOC_FUNC(s)       _malloc_dbg((s), _NORMAL_BLOCK, __FILE__, __LINE__)
#   define ACE_CALLOC_FUNC(c,s)     _calloc_dbg((c), (s), _NORMAL_BLOCK, __FILE__, __LINE__)
#   define ACE_FREE_FUNC(p)         _free_dbg((p), _NORMAL_BLOCK)
#   define ACE_REALLOC_FUNC(p,s)    _realloc_dbg((p), (s), _NORMAL_BLOCK, __FILE__, __LINE__)
    /* For completeness' sake, you should probably put- */
#   define ACE_HAS_STRDUP_EMULATION
#   define ACE_HAS_WCSDUP_EMULATION

#  else

#   pragma intrinsic (abs,memcmp,memcpy,memset,strcat,strcmp,strcpy,strlen)

#  endif

#  pragma warning (disable:4996) /* Declared deprecated */
#  pragma warning (disable:4503) /* Decorated name length exceeded, name was truncated */
//#  pragma warning (disable:4244) /* 'initializing' : conversion from 'X' to 'Y', possible loss of data */

#  include "ace/config-win32.h"

# elif defined(__SUNPRO_CC)

#  include "ace/config-sunos5.10.h"

# elif defined(__ghs__)

#  include "ace/config-integrity.h"

# elif defined(__ANDROID_API__)

#  include "ace/config-android.h"

# elif defined(__APPLE__)

#  include "ace/config-macosx.h"

# elif defined(__GNUG__) || defined(__GNUC__)

#  include "ace/config-linux.h"

# else

# error UNKNOWN Compiler!

# endif

/* Fixes for ACE Builds and defined types */

#if !defined(ACE_RANDR_TYPE)
# define ACE_RANDR_TYPE size_t
#endif

#endif
