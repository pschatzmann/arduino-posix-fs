#pragma once
#include "ConfigFS.h"
#include "FileSystems/Registry.h"
#include "LoggerFS.h"
#include "stdint.h"

#define MAGIC_DIR_EXT 12345679

namespace file_systems {

inline const char* FS_NAME_MEM = "FileSystemMemory";

/**
 * @brief Custom extension of DIR
 */
struct DIR_EXT : public DIR_BASE {
  DIR_EXT() { magic_id = MAGIC_DIR_EXT; }
  /// dirent related to this DIR
  dirent actual_dirent;
  /// all unprocessed files: only used by FileSystemMemory!
  Vector<RegEntry *> files;
  int pos = 0;
};

/**
 * @brief  In Memory File Content to refer to data stored e.g. in PROGMEM
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
struct RegContentMemory : public RegContent {
  RegContentMemory() { id = ContentMemory; }
  operator bool() { return data != nullptr; }
  const uint8_t *data = nullptr;
  size_t size = 0;
  size_t current_pos = 0;
};

/**
 * @brief Dedicated File System for PROGMEM memory files
 * @author Phil Schatzmann
 * @copyright GPLv3
 **/
class FileSystemMemory : public FileSystem {
public:
  FileSystemMemory(const char *path) : FileSystem(path) {
    setFileSystemForSearch();
    DefaultRegistry.add(*this);
#ifdef ESP32
    filename_offset = strlen(path);
    // myfs.flags = ESP_VFS_FLAG_CONTEXT_PTR;
    myfs.write = [](int fd, const void *data, size_t size) {
      FileSystem &fs = DefaultRegistry.fileSystemByName(FS_NAME_MEM);
      return fs.write(fd, data, size);
    };
    myfs.open = [](const char *path, int flags, int mode) {
      FileSystem &fs = DefaultRegistry.fileSystemByName(FS_NAME_MEM);
      return fs.open(path, flags, mode);
    };
    myfs.fstat = [](int fd, struct stat *st) {
      FileSystem &fs = DefaultRegistry.fileSystemByName(FS_NAME_MEM);
      return fs.fstat(fd, st);
    };
    myfs.stat = [](const char*fn, struct stat *st) {
      FileSystem &fs = DefaultRegistry.fileSystemByName(FS_NAME_MEM);
      return fs.stat(fn, st);
    };
    myfs.close = [](int fd) {
      FileSystem &fs = DefaultRegistry.fileSystemByName(FS_NAME_MEM);
      return fs.close(fd);
    };
    myfs.read = [](int fd, void *data, size_t size) {
      FileSystem &fs = DefaultRegistry.fileSystemByName(FS_NAME_MEM);
      return fs.read(fd, data, size);
    };
    myfs.lseek = [](int fd, off_t offset, int mode) {
      FileSystem &fs = DefaultRegistry.fileSystemByName(FS_NAME_MEM);
      return fs.lseek(fd, offset, mode);
    };
    myfs.opendir = [](const char *name) {
      FileSystem &fs = DefaultRegistry.fileSystemByName(FS_NAME_MEM);
      return fs.opendir(name);
    };
    myfs.readdir = [](DIR *pdir) {
      DIR_EXT *ext = (DIR_EXT *)pdir;
      if (ext->magic_id != MAGIC_DIR_EXT) {
        FS_LOGE("invalid FileID %d", ext->magic_id);
        return (dirent *)nullptr;
      }
      return ext->p_file_system->readdir(pdir);
    };
    myfs.closedir = [](DIR *pdir) {
      DIR_EXT *ext = (DIR_EXT *)pdir;
      if (ext->magic_id != MAGIC_DIR_EXT) {
        FS_LOGE("invalid FileID %d", ext->magic_id);
        return -1;
      }
      return ext->p_file_system->closedir(pdir);
    };
    myfs.unlink = [](const char* fn) {
      FileSystem &fs = DefaultRegistry.fileSystemByName(FS_NAME_MEM);
      return fs.unlink(fn);
   };

    esp_vfs_register(path, &myfs, this);
#endif
  };

  /// Selects the actual File System to be used for directory searches
  void setFileSystemForSearch() {
    DefaultRegistry.setFileSystemForSearch(this);
  }

  /// file is valid if it has been added
  bool isValidFile(const char *path) override {
    FS_TRACED();
    for (RegEntry *entry : files) {
      if (entry != nullptr && *entry) {
        if (Str(entry->file_name).equals(path)) {
          return true;
        }
      }
    }
    return false;
  }

  /// adds a in memory file
  bool add(const char *name, const void *data, size_t len) {
    FS_LOGI("add: name='%s' len=%d", name, len);
    if (&DefaultRegistry.fileSystem(name) != this) {
      FS_LOGE("File %s not vaid for  %s in %s", name, this->pathPrefix(),
              DefaultRegistry.fileSystem(name).name());
      return false;
    }
    RegEntry *entry = new RegEntry();
    entry->p_file_system = this;
    // setup content
    RegContentMemory *content = new RegContentMemory();
    content->data = (uint8_t *)data;
    content->size = len;
    // setup entry
    entry->file_name = name;
    entry->content = content;
    files.push_back(entry);
    FS_LOGD("files: %d", files.size());
    return true;
  }

  /// number of file entries
  size_t size() { return files.size(); }

  /// Returns true if there are no files
  bool isEmpty() { return size() == 0; }

  RegEntry &get(const char *path) { return getEntry(path); }

  /// file operations
  int open(const char *path, int flags, int mode) override {
    FS_TRACED();
    RegEntry &mem_entry = get(path);
    if (!mem_entry) {
      FS_LOGW("mem_entry invalid");
      return -1;
    }
    RegEntry &entry = DefaultRegistry.openFile(path, *this);
    // make content available in open files
    if (&entry == &NoRegEntry) {
      FS_LOGW("entry invalid");
      return -1;
    }
    entry.content = mem_entry.content;
    return entry.fileID;
  }

  /// write: not suported
  ssize_t write(int fd, const void *data, size_t size) override {
    FS_LOGW("Write not supported by file system");
    return 0;
  };

  ssize_t read(int fd, void *data, size_t size) override {
    FS_TRACED();
    if (size == 0) {
      return 0;
    }
    RegEntry &entry = DefaultRegistry.getEntry(fd);
    RegContentMemory *p_memory = getContent(entry);
    if (p_memory == nullptr) {
      FS_LOGW("No content for %s", entry.file_name);
      return 0;
    }
    int pos = p_memory->current_pos;
    int len = min(size, p_memory->size - pos);
    p_memory->current_pos += len;
    memmove(data, p_memory->data + pos, len);
    FS_LOGD("=> read: pos=%d size=%d fd=%d -> %d", pos, (int)size, fd, len);
    return len;
  }

  int close(int fd) override {
    FS_TRACED();
    DefaultRegistry.closeFile(fd);
    return 0;
  }

  int fstat(int fd, struct stat *st) override {
    FS_TRACED();
    RegEntry &entry = DefaultRegistry.getEntry(fd);
    RegContentMemory *p_memory = getContent(entry);
    if (p_memory == nullptr || !*p_memory) {
      return -1;
    }
    // st->st_ino = entry.fileID;
    st->st_size = p_memory->size;
    // st->st_mode = entry->isDir() ? S_IFDIR : S_IFREG;
    st->st_mode = S_IFREG;
    FS_LOGD("=> fstat fd=%d -> size=%d ", fd, st->st_size);
    return 0;
  }

  int stat(const char *path, struct stat *st){
    RegEntry &mem_entry = get(path);
    if (!mem_entry) {
      FS_LOGW("mem_entry invalid");
      return -1;
    }
    RegContentMemory *p_memory = getContent(mem_entry);
    // st->st_ino = entry.fileID;
    st->st_size = p_memory->size;
    // st->st_mode = entry->isDir() ? S_IFDIR : S_IFREG;
    st->st_mode = S_IFREG;
    FS_LOGD("=> stat path=%s -> size=%d ", path, st->st_size);
    return 0;
  }

  off_t lseek(int fd, off_t offset, int whence) override {
    FS_TRACED();
    RegEntry &entry = DefaultRegistry.getEntry(fd);
    RegContentMemory *p_memory = getContent(entry);
    if (p_memory == nullptr) {
      return -1;
    }
    switch (whence) {
    case SEEK_SET:
      p_memory->current_pos = offset;
      if (p_memory->current_pos > p_memory->size) {
        int diff = p_memory->size - p_memory->current_pos;
        offset -= diff;
        p_memory->current_pos = p_memory->size - 1;
      }
      break;
    case SEEK_CUR:
      p_memory->current_pos += offset;
      if (p_memory->current_pos > p_memory->size) {
        int diff = p_memory->size - p_memory->current_pos;
        offset -= diff;
        p_memory->current_pos = p_memory->size - 1;
      }
      break;
    case SEEK_END:
      p_memory->current_pos = p_memory->size - offset;
      if (p_memory->current_pos < 0) {
        p_memory->current_pos = 0;
        offset = offset + p_memory->current_pos;
      }
      break;
    }
    return offset;
  }

  // directory operations
  DIR *opendir(const char *name) override {
    FS_LOGD("FileSystemMemory::opendir(%s)", name);
    DIR_EXT *result = new DIR_EXT();
    result->p_file_system = this;

    // find matching files
    for (RegEntry *entry : files) {
      // cut off prefix from file names
      const char *file_name = entry->file_name + filenameOffset();
      FS_LOGD("-> %s %s", file_name, name);
      if (Str(file_name).startsWith(name)) {
        FS_LOGD("--> %s %s", file_name, name);
        result->files.push_back(entry);
      }
    }
    result->actual_dirent.d_type = DT_REG;
    result->pos = 0;
    FS_LOGD("=> opendir: %d files", result->files.size());
    return (DIR *)result;
  }

  dirent *readdir(DIR *dir) override {
    FS_TRACED();
    DIR_EXT *p_dir = (DIR_EXT *)dir;
    if (p_dir->pos >= p_dir->files.size()) {
      FS_LOGD("==> readdir: pos=%d size=%d END", p_dir->pos,
              p_dir->files.size());
      return nullptr;
    }
    RegEntry *entry = p_dir->files[p_dir->pos];
    p_dir->pos++;
    // copy filname to dirent
    strncpy(p_dir->actual_dirent.d_name, entry->file_name + filenameOffset(),
            MAXNAMLEN);
    FS_LOGD("==> readdir: pos=%d size=%d %s", p_dir->pos, p_dir->files.size(),
            entry->file_name + filenameOffset());
    return &(p_dir->actual_dirent);
  }

  int closedir(DIR *dir) override {
    FS_TRACED();
    DIR_EXT *p_dir = (DIR_EXT *)dir;
    if (p_dir != nullptr) {
      delete p_dir;
    }
    return 0;
  }

  int unlink(const char* path){
    FS_LOGE("unlink not supported");
    return -1;
  }

  virtual const char *name() { return FS_NAME_MEM; }

protected:
  // Files in Directory
  Vector<RegEntry *> files;

  // gets a file entry by index
  RegEntry &getEntry(int fd) {
    RegEntry *e = files[fd];
    return *e;
  }

  // gets a file entry by name
  RegEntry &getEntry(const char *fileName) {
    for (auto e : files) {
      const char *entry_file_name = e->file_name + filenameOffset();
      if (Str(fileName).equals(entry_file_name)) {
        return *e;
      }
    }
    return NoRegEntry;
  }

  RegContentMemory *getContent(RegEntry &entry) {
    FS_TRACED();
    RegContentMemory *result = nullptr;
    RegContent *p_content = entry.content;
    if (p_content != nullptr && p_content->id == ContentMemory) {
      result = (RegContentMemory *)p_content;
    }
    return result;
  }
};

} // namespace file_systems
