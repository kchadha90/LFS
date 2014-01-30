#include "cleaner/listsegalloc.h"
namespace lfs {

SegNum ListSegAllocator::Next(void) {
  if (this->q_.empty()) return SegNum_invalid;
  SegNum seg = this->q_.front();
  this->q_.pop_front();
  return seg;
}

};//namespace lfs
