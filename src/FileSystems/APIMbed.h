
#pragma once
#include "ConfigFS.h"

#ifdef FS_IS_MBED
#include "BlockDevice.h"
#include "DirHandle.h"
#include "FileHandle.h"
#include "FileSystem.h"
#include "FileSystems/FileSystemBase.h"
#include "FileSystems/Registry.h"
#include "LoggerFS.h"

namespace file_systems {

/**
 * @brief An mbed implementation for an individual File
 *
 */
class MBEDFileHandle : public mbed::FileHandle {
public:
  MBEDFileHandle(file_systems::FileSystemBase *p_fs, const char *fileName,
                 int fd) {
    FS_LOGI("MBEDFileHandle %s", fileName);
    file_name = fileName;
    this->fd = fd;
    this->p_fs = p_fs;
  }
  ssize_t read(void *buffer, size_t size) {
    return p_fs->read(fd, buffer, size);
  }

  ssize_t write(const void *buffer, size_t size) {
    return p_fs->write(fd, buffer, size);
  }

  off_t seek(off_t offset, int whence = SEEK_SET) {
    return p_fs->lseek(fd, offset, whence);
  }

  int close() { return p_fs->close(fd); }

  int sync() { return 0; }

  int isatty() { return 0; }

  off_t tell() { return p_fs->tell(fd); }

  void rewind() { p_fs->lseek(fd, 0, SEEK_SET); }

  off_t size() {
    struct stat statbuf;
    int rc = p_fs->fstat(fd, &statbuf);
    return rc >= 0 ? statbuf.st_size : -1;
  }

  int truncate(off_t length) { return -1; }

  int set_blocking(bool blocking) { return blocking ? 0 : -1; }

  bool is_blocking() const { return true; }

  int enable_input(bool enabled) { return 0; }

  int enable_output(bool enabled) { return enabled ? -1 : 0; }

protected:
  const char *file_name;
  file_systems::FileSystemBase *p_fs;
  int fd;
};

/**
 * @brief An mbed directory implementation
 *
 */
class MBEDDirHandle : public mbed::DirHandle {
public:
  MBEDDirHandle(file_systems::FileSystemBase *p_fs, const char *fileName, DIR_BASE *p_dir) {
    FS_LOGI("MBEDFileHandle %s", fileName);
    this->file_name = fileName;
    this->fd = fd;
    this->p_fs = p_fs;
    this->p_dir = p_dir;
  }
  ssize_t read(struct dirent *ent) {
    FS_TRACEI();
    dirent *result = p_fs->readdir(p_dir);
    if (result == nullptr) {
      return -1;
    }
    *ent = *result;
    // if (tell()==size()){
    //   return 0;
    // }
    return 1;
  }

  int close() {
    FS_TRACEI();
    p_fs->closedir(p_dir);
    return -1;
  }

  void seek(off_t offset) {
    FS_TRACEI();
    p_dir->seek(offset);
  }

  off_t tell() {
    FS_TRACEI();
    return p_dir->tell();
  }

  void rewind() {
    FS_TRACEI();
    p_dir->seek(0);
  }

  size_t size() {
    FS_TRACEI();
    return p_dir->size();
  }

protected:
  const char *file_name;
  file_systems::FileSystemBase *p_fs;
  DIR_BASE *p_dir = nullptr;
  int fd;
};

/**
 * @brief MBED implementation for a file system
 *
 */
class MBEDFileSystem : public mbed::FileSystemLike {
public:
  MBEDFileSystem(const char *name = NULL)
      : mbed::FileSystemLike(FileSystemBase::standardName(name)) {
    FS_LOGI("MBEDFileSystem %s", name);
    this->name = name;
    statvfs_v.f_namemax = MAXNAMLEN;
    statvfs_v.f_fsid = 1;

    mount(nullptr);
  }

  int mount(mbed::BlockDevice *bd = nullptr) {
    FS_TRACEI();
    FileSystemBase &fs = file_systems::Registry::DefaultRegistry().fileSystem(name);
    if (!fs) {
      FS_LOGE("mount failed");

      return -1;
    }
    p_fs = &fs;
    FS_LOGI("using fs -> %s", p_fs->name());
    return 0;
  }

  int unmount() {
    FS_TRACEI();
    p_fs = nullptr;
    return 0;
  }

  int reformat(mbed::BlockDevice *bd = NULL) {
    FS_TRACEI();
    return 0;
  }

  int remove(const char *path) {
    FS_TRACEI();
    return -1;
  }

  int rename(const char *path, const char *newpath) {
    FS_TRACEI();
    return -1;
  }

  int stat(const char *path, struct stat *st) {
    FS_TRACEI();
    return p_fs->stat(path, st);
  }

  int mkdir(const char *path, mode_t mode) {
    FS_TRACEI();
    return -1;
  }

  int statvfs(const char *path, struct statvfs *buf) {
    FS_TRACEI();
    if (Str(path).startsWith(name) && buf != nullptr) {
      *buf = statvfs_v;
      return 0;
    } else {
      return -1;
    }
  }

  static FileSystemLike *get_default_instance() {
    static file_systems::MBEDFileSystem default_fs("/fs");
    return &default_fs;
  }

protected:
  file_systems::FileSystemBase *p_fs = nullptr;
  const char *name = nullptr;
  struct statvfs statvfs_v;

  virtual int open(mbed::FileHandle **file, const char *path, int flags) {
    FS_LOGI("MBEDFileSystem::open: %s", path);
    if (p_fs == nullptr) {
      FS_LOGE("p_fs is null");
      return -1;
    }
    FS_LOGI("using fs -> %s", p_fs->name());
    int fd = p_fs->open(path, flags, 0);
    if (fd < 0) {
      FS_LOGE("open");
      return -1;
    }
    *file = new MBEDFileHandle(p_fs, path, fd);
    return 0;
  }

  int open(mbed::DirHandle **dir, const char *path) {
    FS_LOGI("MBEDFileSystem::open(dir): %s", path);
    if (p_fs == nullptr) {
      FS_LOGI("p_fs is null");
      return -1;
    }
    DIR_BASE *p_dir = (DIR_BASE *)p_fs->opendir(path);
    if (p_dir==nullptr){
      FS_LOGE("opendir");
      return -1;
    }
    *dir = new MBEDDirHandle(p_fs, path, p_dir);
    return 0;
  }
};

} // namespace file_systems

#endif // FS_IS_MBED
