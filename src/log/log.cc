#include "log/log.h"
//#include "cleaner/seqsegalloc.h"
#include "cleaner/listsegalloc.h"
#include "file/inode_store.h"
#include "cleaner/cleaner.h"
#include "util/logging.h"
namespace lfs {

Log::Log(void) {
  this->set_segcache_capacity(4);
  this->set_checkpoint_interval(4);
  this->set_cleaning_bounds(4, 8);
  this->suppress_cleaning_ = false;
  this->segalloc_ = NULL;
  this->superblock_ = NULL;
  this->segio_ = NULL;
  this->checkpoints_ = NULL;
  this->br_ = NULL;
  this->bw_ = NULL;
  this->istore_ = NULL;
  this->cleaner_ = NULL;
}

bool Log::Open(Disk* disk) {
  assert(disk != NULL);
  this->set_disk(disk);
  this->set_superblock(this->create_superblock());
  if (this->superblock() == NULL) return false;
  this->set_segio(this->create_segio());
  if (this->segio() == NULL) return false;
  this->set_checkpoints(this->create_checkpoints());
  if (this->checkpoints() == NULL) return false;
  this->set_segalloc(this->create_segalloc());
  if (this->segalloc() == NULL) return false;
  this->set_bw(this->create_bw());
  if (this->bw() == NULL) return false;
  this->set_br(this->create_br());
  if (this->br() == NULL) return false;

  this->in_transaction_ = false;
  this->checkpoint_pending_ = 0;
  return true;
}

bool Log::Close(void) {
  this->set_br(NULL);
  this->set_bw(NULL);
  this->set_segalloc(NULL);
  this->set_checkpoints(NULL);
  this->set_segio(NULL);
  this->set_superblock(NULL);
  return true;
}

bool Log::set_checkpoint_interval(SegNum value) {
  if (value < 1) return false;
  this->checkpoint_interval_ = value;
  return true;
}

bool Log::set_cleaning_bounds(SegNum start, SegNum stop) {
  this->cleaning_start_ = start;
  this->cleaning_stop_ = stop;
  return true;
}

void Log::on_WriteBlk(BlkInfo* bi) {
  if (bi->type() == kBTInd || bi->type() == kBTData) this->segusage()->Live(bi);
}

void Log::on_DieBlk(SegBlkNum segblk) {
  if (segblk == SegBlkNum_unassigned) return;
  this->segusage()->Die(segblk);
}

void Log::on_NewSeg(SegNum last_seg, SegNum cur_seg) {
  ++this->checkpoint_pending_;
  if (this->cleaner() != NULL) this->cleaner()->on_NewSeg(last_seg, cur_seg);
  bool checkpoint_written = this->WriteCheckpointCond(kCheckpointReason_NewSeg);
  if (checkpoint_written) this->RunCleanerCond();
}

void Log::TransactionStart(void) {
  this->in_transaction_ = true;
}

void Log::TransactionFinish(void) {
  this->in_transaction_ = false;
  this->WriteCheckpointCond(kCheckpointReason_TransactionFinish);
}

void Log::ForceCheckpoint(void) {
  this->checkpoint_pending_ = this->checkpoint_interval() + 1;
  this->WriteCheckpointCond(kCheckpointReason_Force);
}

Superblock* Log::create_superblock(void) {
  Superblock* v = new Superblock();
  if (v->Read(this->disk())) return v;
  delete v;
  return NULL;
}

SegIO* Log::create_segio(void) {
  return new SegIO(this->disk(), this->superblock(), this->segcache_capacity());
}

Checkpoints* Log::create_checkpoints(void) {
  Checkpoints* v = new Checkpoints(this->segio());
  if (v->Read()) return v;
  delete v;
  return NULL;
}

SegAllocator* Log::create_segalloc(void) {
  //return new SequentialSegAllocator(this->superblock(), this->checkpoint()->next_seg() + 1);
  ListSegAllocator* v = new ListSegAllocator();
  this->segusage()->set_segalloc(v);
  return v;
}

BlkWriter* Log::create_bw(void) {
  BlkWriter* v = new BlkWriter(this->segio(), this->segalloc(), this->checkpoint()->next_seg());
  v->set_log(this);
  return v;
}

BlkReader* Log::create_br(void) {
  BlkReader* v = new BlkReader(this->segio());
  v->set_bw(this->bw());
  return v;
}

bool Log::WriteCheckpointCond(CheckpointReason reason) {
  if (this->checkpoint_pending_ < this->checkpoint_interval()) return false;
  if (this->in_transaction_) return false;
  if (reason != kCheckpointReason_NewSeg) {
    this->bw()->WriteDisk();//triggers upcall, which in turn calls WriteCheckpointCond(NewSeg)
    return false;
  }
  //kCheckpointReason_NewSeg: new segment is allocated but not yet written
  if (this->istore() != NULL) this->istore()->SaveInoFiles();
  this->checkpoint()->set_next_seg(this->bw()->cur_seg());
  this->checkpoints()->Write();
  this->segusage()->on_Checkpoint();
  this->checkpoint_pending_ = 0;
  return true;
}

bool Log::RunCleanerCond(void) {
  if (this->suppress_cleaning_) {
    this->suppress_cleaning_ = false;
    return false;
  }
  SegNum nseg_clean = this->segusage()->nseg_clean();
  if (nseg_clean > this->cleaning_start()) return false;
  Logging::Log(kLLInfo, kLCLog, "Log::RunCleanerCond start cleaning nseg_clean=%"PRIu32, nseg_clean);
  if (this->cleaner() != NULL) {
    this->TransactionStart();
    Cleaner::FinishReason fr = this->cleaner()->Run();
    if (fr == Cleaner::kCFRLoop) {
      this->suppress_cleaning_ = true;
    } else if (fr == Cleaner::kCFRError) {
      //rollback to checkpoint
    }
    this->ForceCheckpoint();
    this->TransactionFinish();
    this->suppress_cleaning_ = false;
  } else {
    Logging::Log(kLLInfo, kLCLog, "Log::RunCleanerCond no cleaner attached");
  }
  return true;
}

bool Log::last_cleaning_fail(void) const {
  if (this->cleaner() == NULL) return false;
  return this->cleaner()->last_result() != Cleaner::kCFRComplete && this->cleaner()->last_result() != Cleaner::kCFRNone;
}

void Log::set_superblock(Superblock* value) {
  if (this->superblock_ != NULL) {
    delete this->superblock_;
  }
  this->superblock_ = value;
}

void Log::set_segio(SegIO* value) {
  if (this->segio_ != NULL) {
    delete this->segio_;
  }
  this->segio_ = value;
}

void Log::set_checkpoints(Checkpoints* value) {
  if (this->checkpoints_ != NULL) {
    delete this->checkpoints_;
  }
  this->checkpoints_ = value;
}

void Log::set_segalloc(SegAllocator* value) {
  if (this->segalloc_ != NULL) {
    delete this->segalloc_;
  }
  this->segalloc_ = value;
}

void Log::set_bw(BlkWriter* value) {
  if (this->bw_ != NULL) {
    delete this->bw_;
  }
  this->bw_ = value;
}

void Log::set_br(BlkReader* value) {
  if (this->br_ != NULL) {
    delete this->br_;
  }
  this->br_ = value;
}


};//namespace lfs
