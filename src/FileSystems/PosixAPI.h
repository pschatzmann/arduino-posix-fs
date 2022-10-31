//#pragma once
#include "ConfigFS.h"

#if POSIX_C_METHOD_IMPLEMENTATION
#include <stdio.h>
#include "FileSystems/Registry.h"

 int open(const char *name, int flags, int mode=0) {
  return file_systems::DefaultRegistry.fileSystem(name).open(name, flags, mode);
}
 int close(int file) { 
  return file_systems::DefaultRegistry.fileSystem(file).close(file); 
}

 int fstat(int file, struct stat *st) {
  return file_systems::DefaultRegistry.fileSystem(file).fstat(file, st);
}

 int read(int file, char *ptr, int len) {
  return file_systems::DefaultRegistry.fileSystem(file).read(file, ptr, len);
}

 int write(int file, char *ptr, int len) {
  return file_systems::DefaultRegistry.fileSystem(file).write(file, ptr, len);
}

 DIR *opendir(const char *name){
  return file_systems::DefaultRegistry.fileSystem(name).opendir(name);
}

 int closedir(DIR *dirp){
  file_systems::FileSystem *pfs = static_cast<file_systems::DIR_BASE*>(dirp)->p_file_system;
  return pfs->closedir(dirp);
}

 struct dirent *readdir(DIR *dirp){
  file_systems::FileSystem *pfs = static_cast<file_systems::DIR_BASE*>(dirp)->p_file_system;
  return pfs->readdir(dirp);
}

#endif
