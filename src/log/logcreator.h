/*
procedure of creating a log
1. lc.set_<metrics>
2. lc.VerifyMetrics()
3. create a disk of length lc.noctets_disk()
4. lc.set_disk()
5. lc.WriteSuperblock()
6. lc.WriteCheckpoints()

*/
#ifndef LFS_LOG_LOGCREATOR_H_
#define LFS_LOG_LOGCREATOR_H_
#include "util/defs.h"
#include "disk/disk.h"
#include "log/superblock.h"
namespace lfs {

class LogCreator {
  public:
    LogCreator(void);
    ~LogCreator(void);
    size_t noctet_sector(void) const { return this->noctet_sector_; }
    void set_noctet_sector(size_t value) { this->noctet_sector_ = value; }
    DiskSectorNum nsector_blk(void) const { return this->nsector_blk_; }
    void set_nsector_blk(DiskSectorNum value) { this->nsector_blk_ = value; }
    BlkNum nblk_seg(void) const { return this->nblk_seg_; }
    void set_nblk_seg(BlkNum value) { this->nblk_seg_ = value; }
    SegNum nseg_disk(void) const { return this->nseg_disk_; }
    void set_nseg_disk(SegNum value) { this->nseg_disk_ = value; }

    size_t noctet_disk(void) const;
    Disk* disk(void) const { return this->disk_; }
    bool set_disk(Disk* disk);//set disk, returns true if disk meets metrics

    bool VerifyMetrics(void) const;//verify metrics meet constraints
    bool WriteSuperblock(void) const;//write superblock
    bool WriteCheckpoints(void) const;//write empty checkpoints (inodes have zero length)

  private:
    class SuperblockCreator : public Superblock {
      public:
        SuperblockCreator(void) {}
        Disk* disk(void) const { return this->disk_; }
        void set_disk(Disk* disk);//set disk, returns true if disk meets metrics
        void SetMetrics(DiskSectorNum nsector_blk, BlkNum nblk_seg, SegNum nseg_disk);
        bool WriteToDisk(void) const;
      private:
        Disk* disk_;
    };
    
    size_t noctet_sector_;//count of octets per sector
    DiskSectorNum nsector_blk_;//count of sectors per block
    BlkNum nblk_seg_;//count of blocks per segment
    SegNum nseg_disk_;//count of segments on the disk
    Disk* disk_;
    SuperblockCreator* sbc_;
    Superblock* superblock(void) const { return this->sbc_; }
    DISALLOW_COPY_AND_ASSIGN(LogCreator);
};

};//namespace lfs
#endif//LFS_LOG_LOGCREATOR_H_
