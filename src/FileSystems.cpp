#include "FileSystems/Registry.h"
#include "LoggerFS.h"
#include "Collections/Vector.h"

namespace file_systems {

// defining global variables
#if FS_LOGGING_ACTIVE
FSLoggerClass FSLogger;
#endif
Registry DefaultRegistry;

// Shared vector for all open files
Vector<RegEntry *> Registry::open_files;
// Shared vector for all file systems
Vector<FileSystemBase *> Registry::file_systems;


}