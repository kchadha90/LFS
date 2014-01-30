#ifndef LFS_UTIL_TIME_H_
#define LFS_UTIL_TIME_H_
#include "util/defs.h"
#include <time.h>
namespace lfs {

typedef uint64_t DateTime;
typedef int64_t TimeSpan;
const DateTime DateTime_epoch = 0;
const DateTime DateTime_invalid = UINT64_C(0xFFFFFFFFFFFFFFFF);
const TimeSpan TimeSpan_zero = 0;
const TimeSpan TimeSpan_second = 1000000000;

DateTime DateTime_now(void);
inline DateTime DateTime_make(struct timespec ts) { return ts.tv_sec * TimeSpan_second + ts.tv_nsec; }
inline time_t DateTime_time(DateTime self) { return self / TimeSpan_second; }
inline TimeSpan DateTime_diff(DateTime self, DateTime other) { return (TimeSpan)(other - self); }
inline DateTime DateTime_add(DateTime self, TimeSpan diff) { return (DateTime)(self + diff); }

};//namespace lfs
#endif//LFS_UTIL_TIME_H_
