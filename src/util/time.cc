#include "util/time.h"
namespace lfs {

DateTime DateTime_now(void) {
  struct timespec ts;
  if (0 == ::clock_gettime(CLOCK_REALTIME, &ts)) {
    return DateTime_make(ts);
  } else return 0;
}

};//namespace lfs
