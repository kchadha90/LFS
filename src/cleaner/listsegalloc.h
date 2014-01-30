#ifndef LFS_CLEANER_LISTSEGALLOC_H_
#define LFS_CLEANER_LISTSEGALLOC_H_
#include "util/defs.h"
#include <deque>
#include "cleaner/segalloc.h"
namespace lfs {

class ListSegAllocator : public SegAllocator {
  public:
    ListSegAllocator(void) {}
    SegNum Next(void);
    void Clear(void) { this->q_.clear(); }
    void Append(SegNum seg) { this->q_.push_back(seg); }
  private:
    std::deque<SegNum> q_;
    DISALLOW_COPY_AND_ASSIGN(ListSegAllocator);
};

};//namespace lfs
#endif//LFS_CLEANER_LISTSEGALLOC_H_
