#ifndef LFS_LOG_LOG_H_
#define LFS_LOG_LOG_H_
#include "util/defs.h"
#include "disk/disk.h"
#include "log/superblock.h"
#include "log/segio.h"
#include "log/checkpoint.h"
#include "cleaner/segusage.h"
#include "log/blkreader.h"
#include "log/blkwriter.h"
namespace lfs {

class InodeStore;
class Cleaner;

class Log {
  public:
    Log(void);
    virtual bool Open(Disk* disk);
    virtual bool Close(void);

    SegNum segcache_capacity(void) const { return this->segcache_capacity_; }
    void set_segcache_capacity(SegNum value) { this->segcache_capacity_ = value; }
    SegNum checkpoint_interval(void) const { return this->checkpoint_interval_; }
    bool set_checkpoint_interval(SegNum value);
    SegNum cleaning_start(void) const { return this->cleaning_start_; }
    SegNum cleaning_stop(void) const { return this->cleaning_stop_; }
    bool set_cleaning_bounds(SegNum start, SegNum stop);
    void set_istore(InodeStore* value) { this->istore_ = value; }
    void set_cleaner(Cleaner* value) { this->cleaner_ = value; }
    bool last_cleaning_fail(void) const;//whether last cleaning has a failure; true=disk is almost full

    Disk* disk(void) const { return this->disk_; }
    Superblock* superblock(void) const { return this->superblock_; }
    SegIO* segio(void) const { return this->segio_; }
    Checkpoints* checkpoints(void) const { return this->checkpoints_; }
    Checkpoint* checkpoint(void) const { return this->checkpoints()->c(); }
    SegUsage* segusage(void) const { return this->checkpoint()->segusage(); }
    SegAllocator* segalloc(void) { return this->segalloc_; }
    BlkWriter* bw(void) const { return this->bw_; }
    BlkReader* br(void) const { return this->br_; }
    
    //upcalls from BlkWriter
    void on_WriteBlk(BlkInfo* bi);//after writing a block
    void on_DieBlk(SegBlkNum segblk);//before marking a block as died; also called when seg!=bw->cur_seg
    void on_NewSeg(SegNum last_seg, SegNum cur_seg);//after switching to new segment; not called when starting to write first segment
    //calls from FUSE or tools
    void TransactionStart(void);//before starting transaction
    void TransactionFinish(void);//after finishing transaction; filesystem should be consistent at this time
    void ForceCheckpoint(void);//force writing checkpoint
    
  protected:
    enum CheckpointReason {
      kCheckpointReason_NewSeg = 1,
      kCheckpointReason_TransactionFinish = 2,
      kCheckpointReason_Force = 3
    };
  
    bool in_transaction_;
    SegNum checkpoint_pending_;//if >=checkpoint_interval, should write a checkpoint as soon as filesystem becomes consistent
    bool suppress_cleaning_;//skip next cleaning

    virtual Superblock* create_superblock(void);
    virtual SegIO* create_segio(void);
    virtual Checkpoints* create_checkpoints(void);
    virtual SegAllocator* create_segalloc(void);
    virtual BlkWriter* create_bw(void);
    virtual BlkReader* create_br(void);

    InodeStore* istore(void) const { return this->istore_; }
    Cleaner* cleaner(void) const { return this->cleaner_; }
    bool WriteCheckpointCond(CheckpointReason reason);//write checkpoint if not in transaction and reaching interval
    bool RunCleanerCond(void);//run cleaner if necessary; call after writing checkpoint

  private:
    SegNum segcache_capacity_;//segio cache capacity
    SegNum checkpoint_interval_;//count of segments written before writing a checkpoint; must >0
    SegNum cleaning_start_;
    SegNum cleaning_stop_;

    Disk* disk_;
    Superblock* superblock_;
    SegIO* segio_;
    Checkpoints* checkpoints_;
    SegAllocator* segalloc_;
    BlkWriter* bw_;
    BlkReader* br_;
    InodeStore* istore_;
    Cleaner* cleaner_;

    void set_disk(Disk* value) { this->disk_ = value; }
    void set_superblock(Superblock* value);
    void set_segio(SegIO* value);
    void set_checkpoints(Checkpoints* value);
    void set_segalloc(SegAllocator* value);
    void set_bw(BlkWriter* value);
    void set_br(BlkReader* value);
    
    DISALLOW_COPY_AND_ASSIGN(Log);
};

};//namespace lfs
#endif//LFS_LOG_LOG_H_
