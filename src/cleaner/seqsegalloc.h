#ifndef LFS_CLEANER_SEQSEGALLOC_H_
#define LFS_CLEANER_SEQSEGALLOC_H_
#include "util/defs.h"
#include "log/superblock.h"
#include "cleaner/segalloc.h"
namespace lfs {

//SegAllocator that allocates segments sequentially without cleaning
class SequentialSegAllocator : public SegAllocator {
  public:
    SequentialSegAllocator(Superblock* superblock, SegNum start_seg);
    SegNum Next(void);
    Superblock* superblock(void) const { return this->superblock_; }
    SegNum cur_seg(void) const { return this->cur_seg_; }
  private:
    Superblock* superblock_;
    SegNum cur_seg_;
    void set_superblock(Superblock* value) { this->superblock_ = value; }
    bool set_cur_seg(SegNum value);
    DISALLOW_COPY_AND_ASSIGN(SequentialSegAllocator);
};

};//namespace lfs
#endif//LFS_CLEANER_SEQSEGALLOC_H_
