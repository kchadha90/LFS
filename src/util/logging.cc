#include "util/logging.h"
#include <cstdio>
#include <cstdarg>
namespace lfs {

LoggingLevel Logging::min_level_ = kLLDebug;
LoggingComponent Logging::components_ = 0xFFFFFFFF;

LoggingLevel Logging::min_level(void) {
  return Logging::min_level_;
}

void Logging::set_min_level(LoggingLevel value) {
  Logging::min_level_ = value;
}

LoggingComponent Logging::components(void) {
  return Logging::components_;
}

void Logging::set_components(LoggingComponent value) {
  Logging::components_ = value;
}

void Logging::Log(LoggingLevel level, LoggingComponent component, const char* format, ...) {
  if (level < Logging::min_level()) return;
  if ((component & Logging::components()) == 0) return;
  
  va_list argptr;
  va_start(argptr, format);
  vfprintf(stderr, format, argptr);
  va_end(argptr);
  fprintf(stderr, "\n");
}


};//namespace lfs
