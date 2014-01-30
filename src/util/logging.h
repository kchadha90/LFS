#ifndef LFS_UTIL_LOGGING_H_
#define LFS_UTIL_LOGGING_H_
#include "defs.h"
namespace lfs {


enum LoggingLevel {
  kLLDebug = 10,
  kLLInfo = 20,
  kLLWarn = 30,
  kLLError = 40
};
typedef uint32_t LoggingComponent;
const LoggingComponent kLCDisk = 0x01;
const LoggingComponent kLCLog = 0x02;
const LoggingComponent kLCFile = 0x04;
const LoggingComponent kLCDir = 0x08;
const LoggingComponent kLCFs = 0x10;
const LoggingComponent kLCFuse = 0x20;
const LoggingComponent kLCCleaner = 0x40;

class Logging {
  public:
    static LoggingLevel min_level(void);
    static void set_min_level(LoggingLevel value);
    static LoggingComponent components(void);
    static void set_components(LoggingComponent value);
    
    static void Log(LoggingLevel level, LoggingComponent component, const char* format, ...);
    
  private:
    Logging(void) {};
    static LoggingLevel min_level_;
    static LoggingComponent components_;
};


};//namespace lfs
#endif//LFS_UTIL_LOGGIN_H_
