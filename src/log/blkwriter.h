#ifndef LFS_LOG_BLKWRITER_H_
#define LFS_LOG_BLKWRITER_H_
#include "util/defs.h"
#include <queue>
#include "log/segio.h"
#include "log/seg_summary.h"
#include "cleaner/segalloc.h"
namespace lfs {

class Log;

class BlkWriter {
  public:
    BlkWriter(SegIO* segio, SegAllocator* allocator, SegNum start_seg);
    ~BlkWriter(void);
    Superblock* superblock(void) const { return this->segio()->superblock(); }
    SegNum cur_seg(void) const { return this->cur_seg_; }
    SegBlkNum Put(BlkInfo* bi, const uint8_t* buf);//put a block (bi is now owned by BlkWriter)
    bool Die(SegBlkNum segblk);//mark a block as died and can be reused by a later Put
    bool WriteDisk(void);//write current segment to disk, and switch to new segment
    uint8_t* Read(SegBlkNum segblk) const;//return the block if it's in buf and live, otherwise NULL; return value owned by BlkWriter, valid until next non-const call, should not be modified
    bool GetMeta(SegBlkNum segblk, BlkInfo* bi) const;//clone metadata to bi if block is in buf and live, otherwise false
    void set_log(Log* value) { this->log_ = value; }

  private:
    SegIO* segio_;
    SegAllocator* allocator_;
    SegNum cur_seg_;//current segment number
    uint8_t* buf_;//segment buffer [noctet_seg]
    std::queue<BlkInfo*> blklist_;//list of BlkInfo, in log order; died blocks are not removed
    BlkInfo** blkmap_;//map from BlkNum to BlkInfo
    Log* log_;
    
    SegIO* segio(void) const { return this->segio_; }
    SegAllocator* allocator(void) const { return this->allocator_; }
    void set_cur_seg(SegNum value);
    uint8_t* buf(void) const { return this->buf_; }
    uint8_t* buf_segsummary(void) const;
    BlkInfo** blkmap(void) const { return this->blkmap_; }
    Log* log(void) const { return this->log_; }
    
    BlkNum AllocBlk(void);//allocate a block in current segment, returns BlkNum_invalid if full
    void CollectSegSummary(SegNum next_seg);//put BlkInfo as seg_summary in buf, clears blklist
    DISALLOW_COPY_AND_ASSIGN(BlkWriter);
};

};//namespace lfs
#endif//LFS_LOG_BLKWRITER_H_
