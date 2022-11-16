#pragma once

// ********** Common Logging *****
#define FS_LOG_PORT Serial
#define FS_DEFAULT_LOGLEVEL FSWarning

#ifndef LOG_METHOD
#  define LOG_METHOD __PRETTY_FUNCTION__
#endif


// ********** OSX/Linux **************
#ifdef IS_DESKTOP
#  include <sys/stat.h>
#  include <dirent.h>
#  define POSIX_C_METHOD_IMPLEMENTATION 1
#  define FS_USE_F_INTERNAL
#  include "ConfigFS/fs_stdio.h"
#endif

// ********** ESP33 **************
#ifdef ESP32
#  define POSIX_C_METHOD_IMPLEMENTATION 0
#  define FILE_MODE_STR
#  define SEEK_MODE_SUPPORTED
#  define USE_DUMMY_SD_IMPL
#  define SUPPORTS_SD
#  include "FS.h"
#  include "esp_vfs.h"
#endif

// ********** RP2040 **************
#if defined(ARDUINO_ARCH_MBED)
# define FS_IS_MBED
# define MAXNAMLEN 160
# define _DIR DIR_impl
#include "platform/mbed_toolchain.h"
#include "mbed_retarget.h"

struct DIR_impl {
    void *handle;
    struct dirent entry;
};

#elif defined(TARGET_RP2040)
#  define POSIX_C_METHOD_IMPLEMENTATION 1
#  define FILE_MODE_STR
#  define SEEK_MODE_SUPPORTED
#  define FS_USE_F_INTERNAL
#  include "ConfigFS/fs_stdio.h"
#  include "ConfigFS/fs_dirent.h"
#  include "ConfigFS/fs_fcntl.h"
#  include "sys/stat.h"
#endif

// ********** STM32 **************
#ifdef ARDUINO_ARCH_STM32
#  define POSIX_C_METHOD_IMPLEMENTATION 1
#  define FILE_MODE_CHR
#  include "sys/stat.h"
#  include "ConfigFS/fs_dirent.h"
#  include "ConfigFS/fs_stdio.h"
#endif

#ifdef ARDUINO_ARCH_AVR
#  define POSIX_C_METHOD_IMPLEMENTATION 1
#  define FILE_MODE_STR
#  define ssize_t long
#  define off_t long
#  define FILENAME_MAX 80
#  define FS_LOGGING_ACTIVE 0
#  include "ConfigFS/fs_dirent.h"
#  include "ConfigFS/fs_stat.h"
#  include "ConfigFS/fs_stdio.h"
#endif

#ifndef FS_LOGGING_ACTIVE
#  define FS_LOGGING_ACTIVE 1
#endif

// Common Functionaliry
#include "ConfigFS/fs_common.h"

#ifndef min
#  define min(A,B) A<B?A:B
#endif
