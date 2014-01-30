#include "cleaner/seqsegalloc.h"
namespace lfs {

SequentialSegAllocator::SequentialSegAllocator(Superblock* superblock, SegNum start_seg) {
  assert(superblock != NULL);
  assert(start_seg >= superblock->first_logseg());
  this->set_superblock(superblock);
  this->cur_seg_ = start_seg - 1;
}

SegNum SequentialSegAllocator::Next(void) {
  SegNum next_seg = this->cur_seg() + 1;
  if (!this->set_cur_seg(next_seg)) return SegNum_invalid;
  return next_seg;
}

bool SequentialSegAllocator::set_cur_seg(SegNum value) {
  if (value < this->superblock()->first_logseg() || value >= this->superblock()->nseg_disk()) return false;
  this->cur_seg_ = value;
  return true;
}


};//namespace lfs
