#ifndef LFS_CLEANER_CLEANER_H_
#define LFS_CLEANER_CLEANER_H_
#include "util/defs.h"
#include <vector>
#include "log/log.h"
#include "fs/fs.h"
namespace lfs {

class Cleaner {
  public:
    enum FinishReason {
      kCFRNone,//no cleaning invoked
      kCFRComplete,//there is at least log->cleaning_stop clean segments
      kCFRTooMany,//already cleaned 2*log->cleaning_stop segments, but cannot make log->cleaning_stop clean segments
      kCFRLoop,//all dirty segments are cleaned once
      kCFRFull,//no clean segment left
      kCFRError//an error occured when cleaning one segment
    };
    Cleaner(Filesystem* fs);
    FinishReason Run(void);//clean until either FinishReason; should be called immediately after writing checkpoint and switching to new segment
    void on_NewSeg(SegNum last_seg, SegNum cur_seg) { ++this->count_newseg_; }//called by Log to notify a new segment is used by BlkWriter
    FinishReason last_result(void) const { return this->last_result_; }

  private:
    class AgeSort {
      public:
        bool operator()(const BlkInfo& x, const BlkInfo& y) { return x.mtime() < y.mtime(); }
    };
    Filesystem* fs_;
    SegNum count_newseg_;
    BlkNum count_moveblk_;
    FinishReason last_result_;
    Filesystem* fs(void) const { return this->fs_; }
    Log* log(void) const { return this->fs()->log(); }
    SegUsage* segusage(void) const { return this->log()->segusage(); }
    InodeStore* istore(void) const { return this->fs()->istore(); }
    bool CleanSegments(const std::vector<SegNum>& segs);//clean a few segments, write live blocks to log tail
    bool MoveBlk(const BlkInfo* bi);//write block to log tail if live
    FinishReason set_last_result(FinishReason value);
    DISALLOW_COPY_AND_ASSIGN(Cleaner);
};

};//namespace lfs
#endif//LFS_CLEANER_CLEANER_H_
