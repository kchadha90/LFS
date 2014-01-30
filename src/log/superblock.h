#ifndef LFS_LOG_SUPERBLOCK_H_
#define LFS_LOG_SUPERBLOCK_H_
#include "util/defs.h"
#include "disk/disk.h"
#include "log/segblknum.h"
namespace lfs {

class Superblock {
  public:
    Superblock(void) {}
    bool Read(Disk* disk);
    size_t noctet_sector(void) const { return this->noctet_sector_; }
    DiskSectorNum nsector_blk(void) const { return this->nsector_blk_; }
    size_t noctet_blk(void) const { return this->noctet_sector() * this->nsector_blk(); }//count of octets in block
    BlkNum nblk_seg(void) const { return this->nblk_seg_; }
    DiskSectorNum nsector_seg(void) const { return this->nsector_blk() * this->nblk_seg(); }//count of sectors in segment
    size_t noctet_seg(void) const { return this->noctet_blk() * this->nblk_seg(); }//count of octets in segment
    SegNum nseg_disk(void) const { return this->nseg_disk_; }
    SegNum first_logseg(void) const { return this->first_logseg_; }
    SegNum checkpoint_seg(int index) const { return (index == 0 || index == 1) ? this->checkpoint_seg_[index] : SegNum_invalid; }
    SegNum nseg_checkpoint(void) const { return this->nseg_checkpoint_; }
    BlkNum nblk_segsummary(void) const { return this->nblk_segsummary_; }
    bool IsLogSeg(SegNum seg) const { return seg >= this->first_logseg(); }
    BlkNum first_segsummaryblk(void) const { return this->nblk_seg() - this->nblk_segsummary(); }//first segment summary block
    bool IsSegSummary(BlkNum blk) const { return blk >= this->first_segsummaryblk(); }

  protected:
    static const uint8_t kMagic[256];
    static const uint32_t kLfsVersion = 1;
    void set_noctet_sector(size_t value);
    void set_nsector_blk(DiskSectorNum value);
    void set_nblk_seg(BlkNum value);
    void set_first_logseg(SegNum value);
    void set_nseg_disk(SegNum value);
    void set_checkpoint_seg(int index, SegNum value);
    void set_nseg_checkpoint(SegNum value);
    void set_nblk_segsummary(BlkNum value);
    DiskSectorNum FindBlockSize(Disk* disk);//search sectors for kMagic and determine nsector_blk, return 0 if kMagic is not found in first 1025 sectors
    bool Read(uint8_t* buf, size_t noctet_blk);//read metrics from seg 0 blk 2
    bool Write(uint8_t* buf) const;//read metrics from seg 0 blk 2

  private:
    size_t noctet_sector_;//count of octets per sector
    DiskSectorNum nsector_blk_;//count of sectors per block
    BlkNum nblk_seg_;//count of blocks per segment
    SegNum nseg_disk_;//count of segments on the disk
    SegNum first_logseg_;//first log segment number
    SegNum checkpoint_seg_[2];//start segment number of checkpoints
    SegNum nseg_checkpoint_;//count of segments per checkpoint
    BlkNum nblk_segsummary_;//count of segment summary blocks per segment
    DISALLOW_COPY_AND_ASSIGN(Superblock);
};

};//namespace lfs
#endif//LFS_LOG_SUPERBLOCK_H_
