#pragma once
#include "Collections/Str.h"

namespace file_systems {

/**
 * @brief Abstract file system which can be identified by a prefix: E.g.
 * /Memory. Implements all privided file operations
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
class FileSystemBase {

public:
  FileSystemBase() = default;
  /// @brief Constructor for a new file system
  /// @param path path prefix for all files
  FileSystemBase(const char *path) { path_prefix = path; }
  ~FileSystemBase() {}
  /// Provides the path for all relevant files managed by this file system
  virtual const char *pathPrefix() { return path_prefix; }
  /// Checks if the file is managed by this file system
  virtual bool isValidFile(const char *path) {
    return (Str(path).startsWith(pathPrefix()));
  }

  operator bool() { return path_prefix != nullptr; }

  virtual const char *name() { return "FileSystem"; }
  // file operations
  virtual int open(const char *path, int flags, int mode) {
    FS_LOGE("FileSystemBase::open %s", path);
    return -1;
  }
  virtual ssize_t write(int fd, const void *data, size_t size) { return 0; }
  virtual ssize_t read(int fd, void *data, size_t size) { return 0; }
  virtual int close(int fd) { return -1; };
  virtual int fstat(int fd, struct stat *st) { return -1; };
  virtual int stat(const char *pathname, struct stat *statbuf) { return -1; };
  virtual off_t lseek(int fd, off_t offset, int mode) { return -1; };
  virtual off_t tell(int fd) { return -1; }

  // directory operations
  virtual DIR *opendir(const char *name) { return nullptr; }
  virtual dirent *readdir(DIR *pdir) { return nullptr; }
  virtual int closedir(DIR *pdir) { return -1; }
  virtual int unlink(const char *path) { return -1; }
  // method for memory file to get the data content
  virtual void *mem_map(const char *path, size_t *p_size) { return NULL; }
  virtual bool is_readonly() { return false; }

  /// file name w/o leading /
  static const char *standardName(const char *name) {
    return name[0] == '/' ? name + 1 : name;
  }

protected:
  const char *path_prefix = "@";
  int filename_offset = 0;
#ifdef ESP32
  esp_vfs_t myfs;
#endif

  /// The ESP32 is removing the path prefix for all file processing
  virtual int filenameOffset() { return filename_offset; }
  // Converts the name to the internal name (removing the path prefix)
  const char *internalFileName(const char *name, bool withPrefix) {
    return withPrefix && FileSystemBase::isValidFile(name) ? standardName(name + filenameOffset()) : standardName(name);
  }
};

} // namespace file_systems
