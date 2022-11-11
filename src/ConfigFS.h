#pragma once

// ********** Common Logging *****
#define FS_LOG_PORT Serial
#define FS_DEFAULT_LOGLEVEL FSWarning

#ifndef LOG_METHOD
#define LOG_METHOD __PRETTY_FUNCTION__
#endif

// ********** OSX/Linux **************
#ifdef IS_DESKTOP
#include "sys/stat.h"
#include <dirent.h>
#endif

// ********** ESP33 **************
#ifdef ESP32
#define POSIX_C_METHOD_IMPLEMENTATION 0
#define FILE_MODE_STR
#define SEEK_MODE_SUPPORTED
#define USE_DUMMY_SD_IMPL
#define SUPPORTS_SD
#include "FS.h"
#include "esp_vfs.h"
#endif

// ********** RP2040 **************
#if defined(ARDUINO_ARCH_MBED)
// hack to prevent dir.h from loading!
#define POSIX_C_METHOD_IMPLEMENTATION 0
#define USE_DUMMY_SD_IMPL
#define MAXNAMLEN 160
#define MBED_HACK
#include "mbed_retarget.h"
#include "platform/mbed_toolchain.h"
#define _DIR DIR_impl
struct DIR_impl {
  void *handle;
  struct dirent entry;
};

#elif defined(TARGET_RP2040)
#define POSIX_C_METHOD_IMPLEMENTATION 1
#define ADD_CPP_IMPL
#define ADD_FS_DIRENT
#define ADD_FS_DIR
#define FILE_MODE_STR
#define SEEK_MODE_SUPPORTED
#define ADD_FNCTL
#define ADD_FS_STDIO_DEFINES
// #  include "FS.h"
#include "sys/stat.h"
#endif

// ********** STM32 **************
#ifdef ARDUINO_ARCH_STM32
#define POSIX_C_METHOD_IMPLEMENTATION 1
#define ADD_FS_DIRENT
#define ADD_FS_DIR
#define ADD_FS_STDIO_DEFINES
#define FILE_MODE_CHR
#include "sys/stat.h"
#endif

#ifdef ARDUINO_ARCH_AVR
#define POSIX_C_METHOD_IMPLEMENTATION 1
#define ADD_FS_DIRENT
#define ADD_FS_DIR
#define ADD_FS_STDIO_DEFINES
#define FILE_MODE_STR
#define ssize_t long
#define off_t long
#define ADD_FS_STAT
#define FILENAME_MAX 80
#define FS_LOGGING_ACTIVE 0
#endif

#ifndef FS_LOGGING_ACTIVE
#define FS_LOGGING_ACTIVE 1
#endif

// ********** Common **************
#ifdef ADD_FS_STDIO_DEFINES
#ifdef __cplusplus
extern "C" {
#endif
int open(const char *name, int flags, ...);
int close(int file);
int fstat(int file, struct stat *st);
int read(int file, void *ptr, size_t len);
int write(int file, const void *ptr, size_t len);
#ifdef __cplusplus
}
#endif
#endif

// minimum implementation for DIR from dirent.h>
#ifdef ADD_FS_DIR
struct DIR_ {};
typedef struct DIR_ DIR;

#ifdef __cplusplus
extern "C" {
#endif
DIR *opendir(const char *name);
int closedir(DIR *dirp);
struct dirent *readdir(DIR *dirp);
int unlink(const char *pathname);

#ifdef __cplusplus
}
#endif

#endif

// minimum implementation for dirent.h
#ifdef ADD_FS_DIRENT
#define MAXNAMLEN 1024
#define DT_REG 8
#define DT_DIR 4
struct dirent {
  char d_name[MAXNAMLEN];
  int d_type;
};
#endif

// minimum implementation for stat.h
#ifdef ADD_FS_STAT
#ifndef S_IFREG
#define S_IFREG 0100000
#define S_IFDIR 0040000
#define S_IFMT 00170000
#endif

struct stat {
  size_t st_size = 0;
  int st_mode = 0;
  int st_ino = 0;
};

#define S_ISDIR(m) (((m)&S_IFMT) == S_IFDIR)
#define S_ISREG(m) (((m)&S_IFMT) == S_IFREG)

#endif

// some entries from fcntl.h
#ifdef ADD_FNCTL

#define O_ACCMODE 0003
#define O_RDONLY 00
#define O_WRONLY 01
#define O_RDWR 02
#define O_CREAT 0100  /* not fcntl */
#define O_EXCL 0200   /* not fcntl */
#define O_NOCTTY 0400 /* not fcntl */
#define O_TRUNC 01000 /* not fcntl */
#define O_APPEND 02000
#define O_NONBLOCK 04000
#define O_NDELAY O_NONBLOCK
#define O_SYNC 010000
#define O_FSYNC O_SYNC
#define O_ASYNC 020000

#endif

#ifdef ADD_CPP_IMPL
#include <stdio.h>

#define fopen fopen_i
#define fread fread_i
#define fgets fgets_i
#define fclose fclose_i
#define fgetc fgetc_i

#ifdef __cplusplus
extern "C" {
#endif
FILE *fopen_i(const char *path, const char *mode);
size_t fread_i(void *buffer, size_t size, size_t count, FILE *stream);
char *fgets_i(char *buffer, int size, FILE *stream);
int fclose_i(FILE *fp);
int getc_i(FILE *stream);
#ifdef __cplusplus
}
#endif

#endif

// ********** Common Methods *****
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

void *mem_map(const char *path, size_t *p_size);

#ifdef __cplusplus
}
#endif
