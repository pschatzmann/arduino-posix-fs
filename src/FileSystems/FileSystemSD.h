#pragma once

#include "ConfigFS.h"
#include "SPI.h"
#include "SD.h"
#include "LoggerFS.h"

#ifdef USE_DUMMY_SD_IMPL
// Some processors already provides this functionality: So we just add a dummy class
namespace file_systems {
class FileSystemSD {
public:
  FileSystemSD(const char *name, fs::SDFS &sd) {
    FS_LOGI("FileSystemSD ignored on ESP32");
  }
};
}
#else

#include "FileSystems/Registry.h"
#include <fcntl.h>

#define MAGIC_DIR_SD 12345679

typedef SDClass ES_SD;

namespace file_systems {

/**
 * @brief Custom extension of DIR
 */
struct DIR_SD : public DIR_BASE {
  DIR_SD() { magic_id = MAGIC_DIR_SD; }
  /// dirent related to this DIR
  dirent actual_dirent;
  /// all unprocessed files: only used by FileSystemMemory!
  File dir;
};

/**
 * @brief Content with File
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
struct RegContentFile : public RegContent {
  RegContentFile(File f) {
    id = ContentFile;
    file = f;
  }
  File file;
};

/**
 * @brief We provide the posix file operations with the help of the SD library.
 * For the ESP32 we set up a virtual file system.
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
class FileSystemSD : public FileSystem {
public:
  FileSystemSD(const char *name, ES_SD &sd) : FileSystem(name) {
    setSD(sd);
    setup(name);
  }

  FileSystemSD(const char *name) : FileSystem(name) { setup(name); }

  void setSD(ES_SD &sd) { p_fs = &sd; }

  /// Selects the actual File System to be used for directory searches
  void setFileSystemForSearch() {
    FS_TRACED();
    DefaultRegistry.setFileSystemForSearch(this);
  }

  int open(const char *path, int flags, int mode) {
    FS_TRACED();
    File file = getFS().open(path, getMode(flags));
    if (!file) {
      FS_LOGE("File does not exist: %s", path);
      return -1;
    }
    return addOpenFile(path, file);
  }

  ssize_t write(int fd, const void *data, size_t size) {
    FS_TRACED();
    return getFile(fd).write((uint8_t *)data, size);
  }

  ssize_t read(int fd, void *data, size_t size) {
    FS_TRACED();
    return getFile(fd).read((uint8_t *)data, size);
  }

  int close(int fd) {
    FS_TRACED();
    getFile(fd).close();
    free(files[fd]);
    files[fd] = nullptr;
    return 0;
  }

  int fstat(int fd, struct stat *st) {
    FS_TRACED();
    File *p_f = &getFile(fd);
    st->st_size = p_f->size();
    st->st_mode = p_f->isDirectory() ? S_IFDIR : S_IFREG;
    st->st_ino = fd;
    return 0;
  }

  int stat(const char *path, struct stat *st){
    File file = getFS().open(path, getMode(flags));
    st->st_size = p_f->size();
    st->st_mode = p_f->isDirectory() ? S_IFDIR : S_IFREG;
    file.close();
  }

  off_t lseek(int fd, off_t offset, int mode) {
    FS_TRACED();
    // SeekMode { SeekSet = 0, SeekCur = 1, SeekEnd = 2};
    #ifdef SEEK_MODE_SUPPORTED
      return getFile(fd).seek(offset, (fs::SeekMode)mode);
    #else
      if(mode!=0) return 0;
      return getFile(fd).seek(offset);
    #endif
  }

  DIR *opendir(const char *path) {
    FS_LOGD("opendir: %s", path);
    // remove path prefix if necessary
    const char *file_name = path + filenameOffset();
    FS_LOGD("SD.open %s", file_name);
    // File file = getFS().open(file_name, FILE_READ);
    File file = SD.open(file_name, FILE_READ);
    if (!file) {
      FS_LOGW("dir not found %s", file_name);
      return nullptr;
    }
    if (!file.isDirectory()) {
      FS_LOGW("file not a directory %s", file_name);
      return nullptr;
    }
    // save directory to scan
    DIR_SD *result = new DIR_SD();
    result->p_file_system = this;
    result->dir = file;
    FS_LOGD("=> %s", file.name());
    return result;
  }

  dirent *readdir(DIR *dir) {
    FS_TRACED();
    DIR_SD *pdir = (DIR_SD *)dir;
    File next = pdir->dir.openNextFile();
    if (!next) {
      FS_LOGD("openNextFile() did not provide new file");
      return nullptr;
    }

    // fill dirent with filename and file type
    dirent &info = static_cast<DIR_SD *>(pdir)->actual_dirent;
    strncpy(info.d_name, next.name(), FILENAME_MAX);
    info.d_type = next.isDirectory() ? DT_DIR : DT_REG;

    return &info;
  }

  int closedir(DIR *pdir) {
    FS_TRACED();
    if (pdir != nullptr) {
      free(pdir);
    }
    return 0;
  }

  virtual const char *name() override { return FS_NAME_SD; }

protected:
  ES_SD *p_fs = nullptr;
  Vector<File *> files;
  const char* FS_NAME_SD = "FileSystemSD";

#ifdef ESP32
  esp_vfs_t myfs;
#endif

  void setup(const char *path) {
    setFileSystemForSearch();
    DefaultRegistry.add(*this);
    filename_offset = strlen(path);
  }

  // FS file system
  ES_SD &getFS() { return *p_fs; }

  // Returns the File by fd
  File &getFile(int fd) {
    RegEntry &entry = DefaultRegistry.getEntry(fd);
    RegContentFile *cf = (RegContentFile *)entry.content;
    return cf->file;
  }

  // Opens the file and adds the File object as content
  int addOpenFile(const char *path, File &file) {
    RegEntry &entry = DefaultRegistry.openFile(path);
    if (entry) {
      entry.content = new RegContentFile(file);
      return entry.fileID;
    } else {
      return -1;
    }
  }

#ifdef FILE_MODE_STR
  const char* getMode(int flags){
    const char *mstr = "r";
    if (flags & O_WRONLY || flags & O_RDWR) {
      mstr = "w";
    } else if (flags & O_APPEND) {
      mstr = "a";
    }
    return mstr;
  }
#endif

#ifdef FILE_MODE_CHR
   char getMode(int flags){
    char mstr = FILE_READ;
    if (flags & O_WRONLY || flags & O_RDWR) {
      mstr = FILE_WRITE;
    } else if (flags & O_APPEND) {
      mstr = FILE_WRITE;
    }
  }
#endif

};

} // namespace file_systems

#endif
