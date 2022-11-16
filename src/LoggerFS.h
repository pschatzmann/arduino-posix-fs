#pragma once
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "ConfigFS.h"
#include "Arduino.h"

namespace file_systems {

enum FSLogLevel_t { FSDebug, FSInfo, FSWarning, FSError };

/**
 * @brief Logging class which supports multiple log levels
 * 
 */
class FSLoggerClass {
public:
  // global actual loggin level for application
  FSLogLevel_t logLevel = FS_DEFAULT_LOGLEVEL;

  /// start the logger
  bool begin(FSLogLevel_t l, Print &out) {
    p_out = &out;
    logLevel = l;
    return true;
  }

  /// Print log message
  void log(FSLogLevel_t level, const char *fmt...) {
    if (logLevel <= level) { // AUDIOKIT_LOG_LEVEL = Debug
      char log_buffer[200];
      strcpy(log_buffer, FS_log_msg[level]);
      strcat(log_buffer, ":     ");
      va_list arg;
      va_start(arg, fmt);
      vsprintf(log_buffer + 9, fmt, arg);
      va_end(arg);
      p_out->println(log_buffer);
    }
  }

protected:
  // Error level as string
  const char *FS_log_msg[4] = {"Debug", "Info", "Warning", "Error"};
  Print *p_out = &FS_LOG_PORT;
};

INLINE_VAR FSLoggerClass FSLogger;

#if FS_LOGGING_ACTIVE
#  define FS_LOGD(fmt, ...) file_systems::FSLogger.log(FSDebug, fmt, ##__VA_ARGS__)
#  define FS_LOGI(fmt, ...) file_systems::FSLogger.log(FSInfo, fmt, ##__VA_ARGS__)
#  define FS_LOGW(fmt, ...) file_systems::FSLogger.log(FSWarning, fmt, ##__VA_ARGS__)
#  define FS_LOGE(fmt, ...) file_systems::FSLogger.log(FSError, fmt, ##__VA_ARGS__)
#  define FS_TRACED() file_systems::FSLogger.log(FSDebug, LOG_METHOD)
#  define FS_TRACEI() file_systems::FSLogger.log(FSInfo, LOG_METHOD)
#  define FS_TRACEE() file_systems::FSLogger.log(FSError, LOG_METHOD)
#else
#  define FS_LOGD(fmt, ...) 
#  define FS_LOGI(fmt, ...) 
#  define FS_LOGW(fmt, ...) 
#  define FS_LOGE(fmt, ...) file_systems::FSLogger.log(FSError, fmt, ##__VA_ARGS__)
#  define FS_TRACED() 
#  define FS_TRACEI() 
#  define FS_TRACEE() 
#endif

} // namespace arduino_FS