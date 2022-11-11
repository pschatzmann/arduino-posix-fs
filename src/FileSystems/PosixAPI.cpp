// #pragma once
#include "ConfigFS.h"

#if POSIX_C_METHOD_IMPLEMENTATION
#include "FileSystems/Registry.h"
#include <stdio.h>

// To prevent linker errors in STM32
extern "C" int _unlink(const char *);
extern "C" int _stat(const char *pathname, struct stat *statbuf);
extern "C" int _open(const char *name, int flags, int mode);

void *mem_map(const char *path, size_t *p_size) {
  return file_systems::DefaultRegistry.fileSystemByName("FileSystemMemory")
      .mem_map(path, p_size);
}

int open(const char *name, int flags, ...) {
  return file_systems::DefaultRegistry.fileSystem(name).open(name, flags, 0);
}

int close(int file) {
  return file_systems::DefaultRegistry.fileSystem(file).close(file);
}

int fstat(int file, struct stat *statbuf) {
  return file_systems::DefaultRegistry.fileSystem(file).fstat(file, statbuf);
}

int stat(const char *pathname, struct stat *statbuf) {
  return file_systems::DefaultRegistry.fileSystem(pathname).stat(pathname,
                                                                 statbuf);
}

int read(int file, void *ptr, size_t len) {
  return file_systems::DefaultRegistry.fileSystem(file).read(file, ptr, len);
}

int write(int file, const void *ptr, size_t len) {
  return file_systems::DefaultRegistry.fileSystem(file).write(file, ptr, len);
}

DIR *opendir(const char *name) {
  return file_systems::DefaultRegistry.fileSystem(name).opendir(name);
}

int closedir(DIR *dirp) {
  file_systems::FileSystem *pfs =
      static_cast<file_systems::DIR_BASE *>(dirp)->p_file_system;
  return pfs->closedir(dirp);
}

struct dirent *readdir(DIR *dirp) {
  file_systems::FileSystem *pfs =
      static_cast<file_systems::DIR_BASE *>(dirp)->p_file_system;
  return pfs->readdir(dirp);
}

int unlink(const char *pathname) {
  return file_systems::DefaultRegistry.fileSystem(pathname).unlink(pathname);
}

#ifdef MBED_HACK

int _open(const char *name, int flags, int mode = 0) {
  return open(name, flags, mode);
}
int _stat(const char *pathname, struct stat *statbuf) {
  return stat(pathname, statbuf);
}
int _unlink(const char *pathname) { return unlink(pathname); }

#endif

// C++ file operations are mapped to _i methods with the help of defines
#ifdef ADD_CPP_IMPL

FILE *fopen_i(const char *path, const char *mode) {
  int file = open(path, 0);
  if (file < 0)
    return nullptr;
  FILE *fp = (FILE *)malloc(sizeof(FILE));
  fp->_file = file;
  return fp;
}

size_t fread_i(void *buffer, size_t size, size_t count, FILE *stream) {
  return read(stream->_file, buffer, size * count);
}

// Reads a single charac
int fgetc_i(FILE *stream) {
  char c;
  size_t len = fread_i(&c, 1, 1, stream);
  return len == 1 ? c : -1;
}

char *fgets_i(char * s, int n, FILE *f) {
  int count=0;
  int c=-1;
  for (int j=0;j<n-1;j++){
    c = fgetc_i(f);
    if (c<0){
      break;
    }
    count = j;
    s[count]=c;
    if (c=='\n'){
      break;
    }
  }
  s[count+1]=0;
  return c!=-1 ? s : nullptr;
}

int fclose_i(FILE *fp) { return close(fp->_file); }

#endif

#endif // POSIX_C_METHOD_IMPLEMENTATION
