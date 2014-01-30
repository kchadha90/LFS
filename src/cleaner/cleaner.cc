#include "cleaner/cleaner.h"
#include <algorithm>
#include "util/logging.h"
namespace lfs {

Cleaner::Cleaner(Filesystem* fs) {
  assert(fs != NULL);
  this->fs_ = fs;
  this->log()->set_cleaner(this);
  this->last_result_ = kCFRNone;
}

Cleaner::FinishReason Cleaner::Run(void) {
  SegNum nseg_clean_initial = this->segusage()->nseg_clean();
  SegNum count_cleaned = 0;//count of segments cleaned in this run
  this->count_newseg_ = 1;
  
  std::priority_queue<SegUsage::CleanScore> csq = this->segusage()->CalcCleanScores();
  SegNum batch_size = (SegNum)std::max(1, std::min((int)this->log()->segcache_capacity()-2, (int)nseg_clean_initial-1));//number of segments to clean at once
  
  Logging::Log(kLLInfo, kLCCleaner, "Cleaner::Run nseg_clean=%"PRIu32" csq.size()=%"PRIuMAX" batch_size=%"PRIu32, nseg_clean_initial, (uintmax_t)csq.size(), batch_size);
  while (nseg_clean_initial + count_cleaned - this->count_newseg_ < this->log()->cleaning_stop()) {
    if (csq.empty()) return this->set_last_result(kCFRLoop);
    if (count_cleaned >= 2*this->log()->cleaning_stop()) return this->set_last_result(kCFRTooMany);
    if (this->count_newseg_ >= nseg_clean_initial-1) return this->set_last_result(kCFRFull);
    std::vector<SegNum> segs;
    size_t this_batch_size = std::min((size_t)csq.size(), (size_t)batch_size);
    Logging::Log(kLLDebug, kLCCleaner, "Cleaner::Run start batch of size %"PRIu32"", this_batch_size);
    while (segs.size() < this_batch_size) {
      const SegUsage::CleanScore& cs = csq.top();
      segs.push_back(cs.seg());
      Logging::Log(kLLDebug, kLCCleaner, "Cleaner::Run prepare seg=%"PRIu32" score=%f live=%"PRIu32, cs.seg(), cs.score(), cs.live());
      csq.pop();
    }
    if (!this->CleanSegments(segs)) return this->set_last_result(kCFRError);
    count_cleaned += segs.size();
    Logging::Log(kLLDebug, kLCCleaner, "Cleaner::Run count_cleaned=%"PRIu32" count_newseg=%"PRIu32" csq.size()=%"PRIuMAX, count_cleaned, this->count_newseg_, (uintmax_t)csq.size());
  }
  return this->set_last_result(kCFRComplete);
}

bool Cleaner::CleanSegments(const std::vector<SegNum>& segs) {
  std::priority_queue<BlkInfo,std::vector<BlkInfo>,Cleaner::AgeSort> bq;
  for (std::vector<SegNum>::const_iterator it = segs.begin(); it != segs.end(); ++it) {
    SegNum seg = *it;
    for (BlkNum blk = 0; blk < this->log()->superblock()->first_segsummaryblk(); ++blk) {
      SegBlkNum segblk = SegBlkNum_make(seg, blk);
      BlkInfo bi;
      if (!this->log()->br()->GetMeta(segblk, &bi)) {
        return false;
      }
      bq.push(bi);
    }
  }
  
  this->count_moveblk_ = 0;
  while (!bq.empty()) {
    const BlkInfo* bi = &bq.top();
    if (!this->MoveBlk(bi)) {
      Logging::Log(kLLWarn, kLCCleaner, "Cleaner::MoveBlk(%016"PRIx64") fail", bi->segblk());
      return false;
    }
    bq.pop();
  }
  Logging::Log(kLLDebug, kLCCleaner, "Cleaner::CleanSegments move %"PRIu32" live blocks", this->count_moveblk_);
  return true;
}

bool Cleaner::MoveBlk(const BlkInfo* bi) {
  if (bi->type() != kBTInd && bi->type() != kBTData) return true;
  ino_t ino = bi->ino();
  if (ino >= Inode::kIno_Good) {
    if (!this->istore()->IsInUse(ino)) return true;
    InodeVer iver = this->istore()->ReadVersion(ino);
    if (iver != bi->ver()) return true;
  }
  
  bool is_ino_file = ino == Inode::kIno_InoMap || ino == Inode::kIno_InoVec || ino == Inode::kIno_InoTbl;
  File* file;
  if (is_ino_file) file = this->fs()->istore()->GetInoFile(ino);
  else file = this->fs()->GetFile(ino);
  
  bool res = true;
  if (bi->type() == kBTData) {
    if (bi->segblk() == file->GetAddress(bi->off())) {
      ++this->count_moveblk_;
      res = file->MoveDataBlk(bi->off());
    }
  } else if (bi->type() == kBTInd) {
    if (bi->segblk() == file->GetIndAddress(bi->off())) {
      ++this->count_moveblk_;
      res = file->MoveIndBlk(bi->off());
    }
  }
  
  if (!is_ino_file) this->fs()->ReleaseFile(file);
  return res;
}


Cleaner::FinishReason Cleaner::set_last_result(Cleaner::FinishReason value) {
  this->last_result_ = value;
  std::string str = "unknown";
  switch (value) {
    case kCFRComplete: str = "kCFRComplete"; break;
    case kCFRTooMany: str = "kCFRTooMany"; break;
    case kCFRLoop: str = "kCFRLoop"; break;
    case kCFRFull: str = "kCFRFull"; break;
    case kCFRError: str = "kCFRError"; break;
    default: break;
  }
  Logging::Log(kLLWarn, kLCCleaner, "Cleaner::Run return %s", str.c_str());
  return value;
}

};//namespace lfs
