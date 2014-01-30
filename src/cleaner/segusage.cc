#include "cleaner/segusage.h"
#include "util/logging.h"
namespace lfs {

SegUsage::SegUsage(Checkpoint* checkpoint, lfs::physical::Checkpoint* p) {
  assert(checkpoint != NULL);
  assert(p != NULL);
  this->checkpoint_ = checkpoint;
  this->p_ = p;
  this->segalloc_ = NULL;
}

void SegUsage::Read(void) {
  //pad the protobuf to number of segments on first use
  while ((SegNum)this->p()->segusage_size() < this->superblock()->nseg_disk()) {
    lfs::physical::SegUsageRec* rec = this->p()->add_segusage();
    rec->set_nblk_live(0);
    rec->set_mtime(0);
  }
}

void SegUsage::Write(void) {
  //we don't have additional structure now
}

void SegUsage::on_Checkpoint(void) {
  this->UpdateAllocList();
}

void SegUsage::Live(BlkInfo* bi) {
  assert(bi != NULL);
  assert(bi->type() == kBTInd || bi->type() == kBTData);
  SegNum seg = SegBlkNum_seg(bi->segblk());
  if (seg < this->superblock()->first_logseg() || seg >= (SegNum)this->p()->segusage_size()) {
    Logging::Log(kLLWarn, kLCCleaner, "SegUsage::Live(seg=%"PRIu32") not a log segment", seg);
    return;
  }
  
  lfs::physical::SegUsageRec* rec = this->p()->mutable_segusage(seg);
  rec->set_nblk_live(rec->nblk_live() + 1);
  if (bi->mtime() > rec->mtime()) rec->set_mtime(bi->mtime());
}

void SegUsage::Die(SegBlkNum segblk) {
  SegNum seg = SegBlkNum_seg(segblk);
  if (seg < this->superblock()->first_logseg() || seg >= (SegNum)this->p()->segusage_size()) {
    Logging::Log(kLLWarn, kLCCleaner, "SegUsage::Die(seg=%"PRIu32") not a log segment", seg);
    return;
  }
  
  lfs::physical::SegUsageRec* rec = this->p()->mutable_segusage(seg);
  if (rec->nblk_live() == 0) {
    Logging::Log(kLLWarn, kLCCleaner, "SegUsage::Die(seg=%"PRIu32") nblk_live is zero", seg);
    return;
  }
  rec->set_nblk_live(rec->nblk_live() - 1);
}

SegNum SegUsage::nseg_clean(void) {
  SegNum count = 0;
  for (SegNum seg = this->superblock()->first_logseg(); seg < (SegNum)this->p()->segusage_size(); ++seg) {
    if (this->p()->segusage(seg).nblk_live() == 0) ++count;
  }
  return count;
}

std::priority_queue<SegUsage::CleanScore> SegUsage::CalcCleanScores(void) {
  DateTime now = DateTime_now();
  std::priority_queue<CleanScore> pq;
  for (SegNum seg = this->superblock()->first_logseg(); seg < (SegNum)this->p()->segusage_size(); ++seg) {
    lfs::physical::SegUsageRec rec = this->p()->segusage(seg);
    if (rec.nblk_live() == 0) continue;
    float u = (float)rec.nblk_live() / this->superblock()->nblk_seg();
    TimeSpan age = DateTime_diff(rec.mtime(), now);
    float score = (1-u) / (1+u) * age;
    CleanScore cs(seg, score, rec.nblk_live());
    pq.push(cs);
  }
  return pq;
}

void SegUsage::set_segalloc(ListSegAllocator* value) {
  this->segalloc_ = value;
  this->UpdateAllocList();
}

void SegUsage::UpdateAllocList(void) {
  if (this->segalloc() == NULL) return;
  this->segalloc()->Clear();
  for (SegNum seg = this->superblock()->first_logseg(); seg < (SegNum)this->p()->segusage_size(); ++seg) {
    if (this->p()->segusage(seg).nblk_live() == 0 && seg != this->checkpoint()->next_seg()) this->segalloc()->Append(seg);
  }
}

SegUsage::CleanScore::CleanScore(SegNum seg, float score, BlkNum live) {
  this->seg_ = seg;
  this->score_ = score;
  this->live_ = live;
}

SegUsage::CleanScore::CleanScore(const CleanScore& other) {
  this->operator=(other);
}

bool SegUsage::CleanScore::operator<(CleanScore other) const {
  return this->score() < other.score();
}

SegUsage::CleanScore& SegUsage::CleanScore::operator=(const CleanScore& other) {
  this->seg_ = other.seg();
  this->score_ = other.score();
  this->live_ = other.live();
  return *this;
}


};//namespace lfs
