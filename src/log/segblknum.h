#ifndef LFS_LOG_BLOCK_H_
#define LFS_LOG_BLOCK_H_
#include "util/defs.h"
namespace lfs {

typedef uint32_t SegNum;
const SegNum SegNum_nseg_min = 0x10;//minimum count of segments per disk
const SegNum SegNum_nseg_max = 0xFFFFF;//maximum count of segments per disk
const SegNum SegNum_invalid = 0xFFFFFFFF;

typedef uint32_t BlkNum;
const BlkNum BlkNum_nblk_min = 0x8;//minimum count of blocks per segment
const BlkNum BlkNum_nblk_max = 0x1000;//maximum count of blocks per segment
const BlkNum BlkNum_invalid = 0xFFFFFFFF;

typedef uint64_t SegBlkNum;
const SegBlkNum SegBlkNum_invalid = UINT64_C(0xFFFFFFFFFFFFFFFF);
const SegBlkNum SegBlkNum_unassigned = UINT64_C(0xFFFFFFFFFFFFFFFE);//indicates a block is not assigned (hole in file)

inline SegNum SegBlkNum_seg(SegBlkNum v) { return v >> 32; };
inline BlkNum SegBlkNum_blk(SegBlkNum v) { return v & 0xFFFFFFFF; }
inline SegBlkNum SegBlkNum_make(SegNum seg, BlkNum blk) { return ((uint64_t)seg << 32) | blk; }

};//namespace lfs
#endif//LFS_LOG_BLOCK_H_
