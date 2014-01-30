#ifndef LFS_CLEANER_SEGUSAGE_H_
#define LFS_CLEANER_SEGUSAGE_H_
#include "util/defs.h"
#include <queue>
#include "log/checkpoint.h"
#include "log/seg_summary.h"
#include "cleaner/listsegalloc.h"
namespace lfs {

class SegUsage {
  public:
    class CleanScore {
      public:
        CleanScore(SegNum seg, float score, BlkNum live);
        CleanScore(const CleanScore& other);
        SegNum seg(void) const { return this->seg_; }
        float score(void) const { return this->score_; }
        BlkNum live(void) const { return this->live_; }
        bool operator<(CleanScore other) const;
        CleanScore& operator=(const CleanScore& other);
      private:
        SegNum seg_;
        float score_;
        BlkNum live_;
    };
  
    SegUsage(Checkpoint* checkpoint, lfs::physical::Checkpoint* p);
    ListSegAllocator* segalloc(void) const { return this->segalloc_; }
    void set_segalloc(ListSegAllocator* value);
    lfs::physical::Checkpoint* p(void) const { return this->p_; }//used by FsVerifier
    
    //calls from Checkpoint
    void Read(void);//read from protobuf
    void Write(void);//write to protobuf
    void on_Checkpoint(void);//after writing a checkpoint

    void Live(BlkInfo* bi);
    void Die(SegBlkNum segblk);
    
    SegNum nseg_clean(void);//count number of clean segments
    std::priority_queue<CleanScore> CalcCleanScores(void);//calculate clean scores

  private:
    Checkpoint* checkpoint_;
    lfs::physical::Checkpoint* p_;
    ListSegAllocator* segalloc_;
    Checkpoint* checkpoint(void) const { return this->checkpoint_; }
    Superblock* superblock(void) const { return this->checkpoint()->superblock(); }
    void UpdateAllocList(void);
    DISALLOW_COPY_AND_ASSIGN(SegUsage);
};

};//namespace lfs
#endif//LFS_CLEANER_SEGUSAGE_H_
