#pragma once
#include "Collections/Queue.h"
#include "Collections/Str.h"
#include "Collections/Vector.h"
#include "ConfigFS.h"
#include "FileSystems/FileSystemBase.h"
#include "LoggerFS.h"

namespace file_systems {

/// Enum Used to identfy the content type
enum RegContentType { ContentUndefined, ContentFile, ContentMemory };

/**
 * @brief Common data for custom DIR
 * @author Phil Schatzmann
 * @copyright GPLv3
 *
 */
struct DIR_BASE : public DIR {
  DIR_BASE()=default;
  virtual ~DIR_BASE() {}
  int magic_id = 0;
  FileSystemBase *p_file_system = nullptr;
  virtual bool seek(off_t offset) { return false; }
  virtual off_t tell() { return -1; }
  virtual ssize_t size() { return -1; };
};
/**
 * @brief Base class for all Contents
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
struct RegContent {
  RegContentType id = ContentUndefined;
};

/**
 * @brief An individual file entry (e.g. to manage open files)
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
struct RegEntry {
  RegEntry() = default;
  virtual ~RegEntry() {
    assert(memory_guard == 12345);
#ifdef ESP32
    assert(heap_caps_check_integrity_all(true));
#endif
    if (content != nullptr) {
      FS_LOGD("~RegEntry: %s", file_name);
      delete content;
      content = nullptr;
    }
  }
  int memory_guard = 12345;
  /// reference to the file system
  FileSystemBase *p_file_system = nullptr;
  /// the name of the file
  const char *file_name = nullptr;
  /// pointer to specific content object
  RegContent *content = nullptr;
  /// index in the vector of open files
  int fileID = 0; // index pos in open_files vector
  /// returns true when the content is defined
  virtual operator bool() { return content != nullptr; }
};

// Invalid RegEntry
static RegEntry NoRegEntry;
// Invalid FileSystem
static FileSystemBase NoFileSystem("/null");

/**
 * @brief Registry which manages open files
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
class Registry {
public:
  Registry() = default;

  /// Registers the file system
  void add(FileSystemBase &fileSystem) {
    FS_TRACED();
    file_systems.push_back(&fileSystem);
  }

  /// opens a new file and provides the corresponding file descriptor
  RegEntry &openFile(const char *path) {
    FS_TRACED();
    FileSystemBase &fs = fileSystem(path);
    if (!fs) {
      FS_LOGE("openFile: No filesystem for %s", path);
      return NoRegEntry;
    }
    return openFile(path, fs);
  }

  /// opens a new file and provides the corresponding file descriptor
  RegEntry &openFile(const char *path, FileSystemBase &fs) {
    FS_TRACED();
    RegEntry *new_entry = new RegEntry();
    new_entry->p_file_system = &fs;
    new_entry->file_name = path;

    int idx = findOpenEmpty();

    if (idx >= 0) {
      // replace nullptr with entry
      new_entry->fileID = idx;
      open_files[idx] = new_entry;
    } else {
      // add new entry at end
      new_entry->fileID = size();
      open_files.push_back(new_entry);
    }
    FS_LOGD("=> total open files %d", size());
    return *new_entry;
  }

  /// Determines the file system for a file path
  FileSystemBase &fileSystem(const char *path) {
    FS_LOGD("fileSystem(%s)", path);
    for (auto p_fs : file_systems) {
      if (p_fs->FileSystemBase::isValidFile(path)) {
        FS_LOGD("-> %s", p_fs->name());
        return *p_fs;
      }
    }
    FS_LOGE("No filesystem for %s", path);
    return NoFileSystem;
  }

  /// Determines the file system for the fileID
  FileSystemBase &fileSystem(int id) {
    RegEntry &entry = getEntry(id);
    if (entry && entry.p_file_system != nullptr) {
      return *(entry.p_file_system);
    }
    FS_LOGE("No filesystem for %d", id);
    return NoFileSystem;
  }

  /// closes the file at the indicated idx
  void closeFile(RegEntry &entry) { closeFile(entry.fileID); }

  /// closes the file at the indicated idx
  void closeFile(int fileID) {
    FS_TRACED();
    RegEntry *p_entry = open_files[fileID];
    if (p_entry != nullptr) {
      delete p_entry;
      open_files[fileID] = nullptr;
    }
  }

  /// Returns the File by fd
  RegEntry &getEntry(int fileID) {
    if ((size_t)fileID < size()) {
      return *open_files[fileID];
    }
    FS_LOGE("fileSystem: No Regentry for %d", fileID);
    return NoRegEntry;
  }

  /// Reurns the number of file entries
  size_t size() { return open_files.size(); }

  /// Defines the actual file system that is used for directory searches
  void setFileSystemForSearch(FileSystemBase *fs) { search_file_system = fs; }

  /// Provides the actual file system that is used for directory searches
  FileSystemBase &fileSystemForSearch() { return *search_file_system; }

  /// Determines the file system by name
  FileSystemBase &fileSystemByName(const char *path) {
    FS_TRACED();
    for (auto p_fs : file_systems) {
      if (Str(p_fs->name()).equals(path)) {
        return *p_fs;
      }
    }
    if (!file_systems.empty()) {
      FS_LOGE("No filesystem for %s", path);
    } else {
      FS_LOGD("No filesystems available");
    }
    return NoFileSystem;
  }

protected:
  FileSystemBase *search_file_system;
  // Shared vector for all open files
  Vector<RegEntry *> open_files;
  // Shared vector for all file systems
  Vector<FileSystemBase *> file_systems;

  // Finds an empty stop in the open files list
  int findOpenEmpty() {
    FS_TRACED();
    int result = -1;
    for (size_t j = 0; j < size(); j++) {
      RegEntry *entry = open_files[j];
      if (entry == nullptr) {
        result = j;
        break;
      }
    }
    return result;
  }
};

extern Registry DefaultRegistry;

} // namespace file_systems
