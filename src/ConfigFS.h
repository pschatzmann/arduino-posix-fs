#pragma once

// ********** Common Logging *****
#define FS_LOG_PORT Serial
#define FS_DEFAULT_LOGLEVEL FSWarning

#ifndef LOG_METHOD
#  define LOG_METHOD __PRETTY_FUNCTION__
#endif

// ********** ESP33 **************
#ifdef ESP32
#  define POSIX_C_METHOD_IMPLEMENTATION 0
#  define FILE_MODE_STR
#  define SEEK_MODE_SUPPORTED
#  define USE_DUMMY_SD_IMPL
#  define SUPPORTS_SD
#  include "FS.h"
#endif

// ********** RP2040 **************
#if defined(ARDUINO_ARCH_MBED)
// hack to prevent dir.h from loading!
#  define POSIX_C_METHOD_IMPLEMENTATION 0
#  define USE_DUMMY_SD_IMPL
#elif defined(TARGET_RP2040)
#  define POSIX_C_METHOD_IMPLEMENTATION 1
#  define NO_DIRENT
#  define NO_DIR
#  define FILE_MODE_STR
#  define SEEK_MODE_SUPPORTED
//#  include "FS.h"
#  include "sys/stat.h"
#endif

// ********** STM32 **************
#ifdef ARDUINO_ARCH_STM32 
#  define POSIX_C_METHOD_IMPLEMENTATION 1
#  define NO_DIRENT
#  define NO_DIR
#  define FILE_MODE_CHR
#  include "sys/stat.h"
#endif

#ifdef ARDUINO_ARCH_AVR
#  define POSIX_C_METHOD_IMPLEMENTATION 1
#  define NO_DIRENT
#  define NO_DIR
#  define FILE_MODE_STR
#  define ssize_t long
#  define off_t long
#  define NO_STAT
#  define FILENAME_MAX 80
#  define FS_LOGGING_ACTIVE 0
#endif

#ifndef FS_LOGGING_ACTIVE
#define FS_LOGGING_ACTIVE 1
#endif

// ********** Common **************

// minimum implementation for DIR from sys/types.h>
#ifdef NO_DIR
struct DIR {};
struct DIR *opendir(const char *name);
int closedir(struct DIR *dirp);
struct dirent *readdir(struct DIR *dirp);
#endif

// minimum implementation for dirent.h
#ifdef NO_DIRENT
#define MAXNAMLEN 1024
#define DT_REG 8
#define DT_DIR 4
struct dirent {
  char d_name[MAXNAMLEN];
  int d_type;
};
#endif

// minimum implementation for stat.h
#ifdef NO_STAT
#define S_IFREG 0100000
#define S_IFDIR 0040000
#define S_IFMT  00170000

struct stat {
  size_t st_size=0;
  int st_mode=0;
  int st_ino=0;
};

#define S_ISDIR(m) (((m) & S_IFMT) == S_IFDIR)
#define S_ISREG(m) (((m) & S_IFMT) == S_IFREG)
#endif