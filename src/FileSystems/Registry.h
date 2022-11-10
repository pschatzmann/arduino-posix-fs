#pragma once
#include "Collections/Queue.h"
#include "Collections/Str.h"
#include "Collections/Vector.h"
#include "ConfigFS.h"
#include "LoggerFS.h"

namespace file_systems {

/// Enum Used to identfy the content type
enum RegContentType { ContentUndefined, ContentFile, ContentMemory };

/**
 * @brief Abstract file system which can be identified by a prefix: E.g.
 * /Memory. Implements all privided file operations
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
class FileSystem {

public:
  FileSystem() = default;
  /// @brief Constructor for a new file system
  /// @param path path prefix for all files
  FileSystem(const char *path) { path_prefix = path; }
  /// Provides the path for all relevant files managed by this file system
  virtual const char *pathPrefix() { return path_prefix; }
  /// Checks if the file is managed by this file system
  virtual bool isValidFile(const char *path) {
    return (Str(path).startsWith(pathPrefix()));
  }
  /// The ESP32 is removing the path prefix for all file processing 
  virtual int filenameOffset() {
    return filename_offset;
  }

  operator bool() { return path_prefix != nullptr; }

  virtual const char *name() { return "FileSystem"; }
  // file operations
  virtual int open(const char *path, int flags, int mode) { return -1; }
  virtual ssize_t write(int fd, const void *data, size_t size) { return 0; }
  virtual ssize_t read(int fd, void *data, size_t size) { return 0; }
  virtual int close(int fd) { return -1; };
  virtual int fstat(int fd, struct stat *st) { return -1; };
  virtual int stat(const char *pathname, struct stat *statbuf){ return -1; };
  virtual off_t lseek(int fd, off_t offset, int mode) { return -1; };
  // directory operations
  virtual DIR *opendir(const char *name) { return nullptr; }
  virtual dirent *readdir(DIR *pdir) { return nullptr; }
  virtual int closedir(DIR *pdir) { return -1; }
  virtual int unlink(const char* path) { return -1; }
  // method for memory file to get the data content
  virtual void* mem_map(const char* path,size_t *p_size) { return NULL; }

protected:
  const char *path_prefix = "@";
  int filename_offset = 0;
#ifdef ESP32
  esp_vfs_t myfs;
#endif
};

/**
 * @brief Common data for custom DIR
 * @author Phil Schatzmann
 * @copyright GPLv3
 * 
 */
struct DIR_BASE : public DIR {
  int magic_id = 0;
  FileSystem *p_file_system = nullptr;
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
    assert(memory_guard==12345);
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
  FileSystem *p_file_system = nullptr;
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
inline RegEntry NoRegEntry;
// Invalid FileSystem
inline FileSystem NoFileSystem("/null");
// Shared vector for all open files
inline Vector<RegEntry *> open_files;
// Shared vector for all file systems
inline Vector<FileSystem *> file_systems;

/**
 * @brief Registry which manages open files
 * @author Phil Schatzmann
 * @copyright GPLv3
 */
class Registry {
public:
  Registry() = default;

  /// Registers the file system
  void add(FileSystem &fileSystem) {
    FS_TRACED();
    file_systems.push_back(&fileSystem);
  }

  /// opens a new file and provides the corresponding file descriptor
  RegEntry &openFile(const char *path) {
    FS_TRACED();
    FileSystem &fs = fileSystem(path);
    if (!fs) {
      FS_LOGE("openFile: No filesystem for %s", path);
      return NoRegEntry;
    }
    return openFile(path, fs);
  }

  /// opens a new file and provides the corresponding file descriptor
  RegEntry &openFile(const char *path, FileSystem &fs) {
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
  FileSystem &fileSystem(const char *path) {
    FS_TRACED();
    for (auto p_fs : file_systems) {
      if (p_fs->FileSystem::isValidFile(path)) {
        return *p_fs;
      }
    }
    FS_LOGE("No filesystem for %s", path);
    return NoFileSystem;
  }

  /// Determines the file system for the fileID
  FileSystem &fileSystem(int id) {
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
    if (fileID < size()) {
      return *open_files[fileID];
    }
    FS_LOGE("fileSystem: No Regentry for %d", fileID);
    return NoRegEntry;
  }

  /// Reurns the number of file entries
  size_t size() { return open_files.size(); }

  /// Defines the actual file system that is used for directory searches
  void setFileSystemForSearch(FileSystem *fs) { search_file_system = fs; }

  /// Provides the actual file system that is used for directory searches
  FileSystem &fileSystemForSearch() { return *search_file_system; }

  /// Determines the file system by name
  FileSystem &fileSystemByName(const char *path) {
    FS_TRACED();
    for (auto p_fs : file_systems) {
      if (Str(p_fs->name()).equals(path)) {
        return *p_fs;
      }
    }
    if (!file_systems.empty()){
      FS_LOGE("No filesystem for %s", path);
    } else {
      FS_LOGD("No filesystems available");
    }
    return NoFileSystem;
  }

protected:
  FileSystem *search_file_system;

  // Finds an empty stop in the open files list
  int findOpenEmpty() {
    FS_TRACED();
    int result = -1;
    for (int j = 0; j < size(); j++) {
      RegEntry *entry = open_files[j];
      if (entry == nullptr) {
        result = j;
        break;
      }
    }
    return result;
  }
};

inline Registry DefaultRegistry;

} // namespace file_systems

