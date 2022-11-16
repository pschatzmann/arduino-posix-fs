/**
 * @file fs_stdio.h
 * @author Phil Schatzmann
 * @brief Simple minimal replacment for stdio.h
 * @version 0.1
 * @date 2022-11-12
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#pragma once
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

void* espeak_mem_map(const char* path, int* len); 

struct stat;
// original functioins
int open(const char *name, int flags, ...);
int close(int file);
int fstat(int file, struct stat *st);
int read(int file, void *ptr, size_t len);
int write(int file, const void *ptr, size_t len);
off_t lseek(int fd, off_t offset, int mode);


#ifdef FS_USE_F_INTERNAL
// potentially replaced functions
FILE *fopen_i(const char *path, const char *mode);
size_t fread_i(void *buffer, size_t size, size_t count, FILE *stream);
char *fgets_i(char *buffer, int size, FILE *stream);
int fclose_i(FILE *fp);
int fseek_i(FILE *stream, long int offset, int whence);
int fgetc_i(FILE *stream);
#endif

#ifdef __cplusplus
}
#endif

#ifdef FS_USE_F_INTERNAL
#  define fopen fopen_i
#  define fread fread_i
#  define fgets fgets_i
#  define fclose fclose_i
#  define fgetc fgetc_i
#  define fseek fseek_i
#endif
