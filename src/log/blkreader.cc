#include "log/blkreader.h"
#include <cstring>
namespace lfs {

BlkReader::BlkReader(SegIO* segio) {
  assert(segio != NULL);
  this->segio_ = segio;
  this->bw_ = NULL;
}

bool BlkReader::Read(SegBlkNum segblk, uint8_t* buf) {
  assert(buf != NULL);
  if (segblk == SegBlkNum_unassigned) { memset(buf, 0, this->superblock()->noctet_blk()); return true; }
  if (SegBlkNum_blk(segblk) >= this->superblock()->nblk_seg()) return false;

  uint8_t* bwr = this->BlkWriterRead(segblk);
  if (bwr != NULL) {
    ::memcpy(buf, bwr, this->superblock()->noctet_blk());
    return true;
  }
  return this->segio()->Read(SegBlkNum_seg(segblk), SegBlkNum_blk(segblk) * this->superblock()->noctet_blk(), this->superblock()->noctet_blk(), buf);
}

bool BlkReader::Read(SegBlkNum segblk, size_t start, size_t count, uint8_t* buf) {
  assert(buf != NULL);
  if (segblk == SegBlkNum_unassigned) { memset(buf, 0, count); return true; }
  if (SegBlkNum_blk(segblk) >= this->superblock()->nblk_seg() || start + count > this->superblock()->noctet_blk()) return false;

  uint8_t* bwr = this->BlkWriterRead(segblk);
  if (bwr != NULL) {
    ::memcpy(buf, bwr + start, count);
    return true;
  }
  return this->segio()->Read(SegBlkNum_seg(segblk), SegBlkNum_blk(segblk) * this->superblock()->noctet_blk() + start, count, buf);
}

bool BlkReader::Read(SegBlkNum segblk, size_t start, size_t count, charbuf* cb) {
  assert(cb != NULL);
  if (segblk == SegBlkNum_unassigned) { cb->append(count, 0); return true; }
  if (SegBlkNum_blk(segblk) >= this->superblock()->nblk_seg() || start + count > this->superblock()->noctet_blk()) return false;

  uint8_t* bwr = this->BlkWriterRead(segblk);
  if (bwr != NULL) {
    cb->append(bwr + start, count);
    return true;
  }
  return this->segio()->Read(SegBlkNum_seg(segblk), SegBlkNum_blk(segblk) * this->superblock()->noctet_blk() + start, count, cb);
}

bool BlkReader::GetMeta(SegBlkNum segblk, BlkInfo* bi) {
  assert(bi != NULL);
  if (this->bw() != NULL && this->bw()->GetMeta(segblk, bi)) return true;
  SegNum seg = SegBlkNum_seg(segblk);
  BlkNum blk = SegBlkNum_blk(segblk);
  size_t rec_off = this->superblock()->first_segsummaryblk() * this->superblock()->noctet_blk() + BlkInfo::kNoctet * blk;
  uint8_t buf[BlkInfo::kNoctet];
  if (!this->segio()->Read(seg, rec_off, BlkInfo::kNoctet, buf)) return false;
  if (!bi->Read(buf)) return false;
  bi->set_segblk(segblk);
  return true;
}

uint8_t* BlkReader::BlkWriterRead(SegBlkNum segblk) {
  if (this->bw() == NULL) return NULL;
  return this->bw()->Read(segblk);
}

};//namespace lfs
