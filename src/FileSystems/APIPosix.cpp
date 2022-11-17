#include "ConfigFS.h"

#if POSIX_C_METHOD_IMPLEMENTATION
#include "FileSystems/Registry.h"
#include <stdio.h>

// To prevent linker errors in STM32
extern "C" int _unlink(const char *);
extern "C" int _stat(const char *pathname, struct stat *statbuf);
extern "C" int _open(const char *name, int flags, int mode);

void *mem_map(const char *path, size_t *p_size) {
  return file_systems::Registry::DefaultRegistry().fileSystemByName("FileSystemMemory")
      .mem_map(path, p_size);
}

int open(const char *name, int flags, ...) {
  return file_systems::Registry::DefaultRegistry().fileSystem(name).open(name, flags, 0);
}

int close(int file) {
  if (file<0) return file;
  return file_systems::Registry::DefaultRegistry().fileSystem(file).close(file);
}

int fstat(int file, struct stat *statbuf) {
  if (file<0) return file;
  return file_systems::Registry::DefaultRegistry().fileSystem(file).fstat(file, statbuf);
}

int stat(const char *pathname, struct stat *statbuf) {
  return file_systems::Registry::DefaultRegistry().fileSystem(pathname).stat(pathname,
                                                                 statbuf);
}

int read(int file, void *ptr, size_t len) {
  if (file<0) return file;
  return file_systems::Registry::DefaultRegistry().fileSystem(file).read(file, ptr, len);
}

int write(int file, const void *ptr, size_t len) {
  if (file<0) return file;
  return file_systems::Registry::DefaultRegistry().fileSystem(file).write(file, ptr, len);
}

off_t lseek(int file, off_t offset, int mode) {
  if (file<0) return file;
  return file_systems::Registry::DefaultRegistry().fileSystem(file).lseek(file, offset, mode);
}

DIR *opendir(const char *name) {
  return file_systems::Registry::DefaultRegistry().fileSystem(name).opendir(name);
}

int closedir(DIR *dirp) {
  file_systems::FileSystemBase *pfs =
      static_cast<file_systems::DIR_BASE *>(dirp)->p_file_system;
  return pfs->closedir(dirp);
}

struct dirent *readdir(DIR *dirp) {
  file_systems::FileSystemBase *pfs =
      static_cast<file_systems::DIR_BASE *>(dirp)->p_file_system;
  return pfs->readdir(dirp);
}

int unlink(const char *pathname) {
  return file_systems::Registry::DefaultRegistry().fileSystem(pathname).unlink(pathname);
}


#endif // POSIX_C_METHOD_IMPLEMENTATION
