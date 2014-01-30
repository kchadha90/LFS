#ifndef LFS_CLEANER_SEGALLOC_H_
#define LFS_CLEANER_SEGALLOC_H_
#include "util/defs.h"
#include "log/segblknum.h"
namespace lfs {

class SegAllocator {
  public:
    SegAllocator(void) {}
    virtual SegNum Next(void) = 0;//allocate a clean segment for writing, returns SegNum_invalid if disk is full
  private:
    DISALLOW_COPY_AND_ASSIGN(SegAllocator);
};

};//namespace lfs
#endif//LFS_CLEANER_SEGALLOC_H_
