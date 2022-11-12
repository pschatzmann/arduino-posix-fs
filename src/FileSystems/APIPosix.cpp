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

off_t lseek(int file, off_t offset, int mode) {
  return file_systems::DefaultRegistry.fileSystem(file).lseek(file, offset, mode);
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


#endif // POSIX_C_METHOD_IMPLEMENTATION
