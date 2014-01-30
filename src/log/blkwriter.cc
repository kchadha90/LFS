#include "log/blkwriter.h"
#include <cstdlib>
#include <cstring>
#include "log/log.h"
#include "util/logging.h"
namespace lfs {

BlkWriter::BlkWriter(SegIO* segio, SegAllocator* allocator, SegNum start_seg) {
  assert(segio != NULL);
  assert(allocator != NULL);
  this->segio_ = segio;
  this->allocator_ = allocator;
  this->cur_seg_ = start_seg;
  this->blkmap_ = (BlkInfo**)calloc(this->superblock()->nblk_seg(), sizeof(BlkInfo*));
  this->buf_ = (uint8_t*)malloc(this->superblock()->noctet_seg());
  this->log_ = NULL;
}

BlkWriter::~BlkWriter(void) {
  while (!this->blklist_.empty()) {
    delete this->blklist_.front();
    this->blklist_.pop();
  }
  free(this->blkmap());
  free(this->buf_);
}

SegBlkNum BlkWriter::Put(BlkInfo* bi, const uint8_t* buf) {
  BlkNum blk = this->AllocBlk();
  if (blk == BlkNum_invalid) {
    if (!this->WriteDisk()) return SegBlkNum_invalid;
    if ((blk = this->AllocBlk()) == BlkNum_invalid) return SegBlkNum_invalid;
  }
  std::memcpy(this->buf() + blk * this->superblock()->noctet_blk(), buf, this->superblock()->noctet_blk());
  SegBlkNum segblk = SegBlkNum_make(this->cur_seg(), blk);
  bi->set_segblk(segblk);
  this->blklist_.push(bi);
  this->blkmap()[blk] = bi;
  if (this->log() != NULL) this->log()->on_WriteBlk(bi);
  return segblk;
}

bool BlkWriter::Die(SegBlkNum segblk) {
  if (this->log() != NULL) this->log()->on_DieBlk(segblk);
  if (SegBlkNum_seg(segblk) != this->cur_seg()) return false;
  BlkNum blk = SegBlkNum_blk(segblk);
  if (this->blkmap()[blk] == NULL) return false;
  this->blkmap()[blk] = NULL;
  return true;
}

bool BlkWriter::WriteDisk(void) {
  SegNum cur_seg = this->cur_seg();
  bool need_write = false;
  for (BlkNum blk = 0; blk < this->superblock()->first_segsummaryblk(); ++blk) {
    if (this->blkmap()[blk] != NULL) {
      need_write = true;
      break;
    }
  }
  if (!need_write) {
    Logging::Log(kLLInfo, kLCLog, "BlkWriter::WriteDisk(cur_seg=%"PRIu32") !need_write", cur_seg);
    return true;
  }
  
  SegNum next_seg = this->allocator()->Next();
  if (next_seg == SegNum_invalid) {
    Logging::Log(kLLWarn, kLCLog, "BlkWriter::WriteDisk(cur_seg=%"PRIu32",next_seg=invalid)", cur_seg);
    return false;
  }
  Logging::Log(kLLInfo, kLCLog, "BlkWriter::WriteDisk(cur_seg=%"PRIu32",next_seg=%"PRIu32")", cur_seg, next_seg);
  
  this->CollectSegSummary(next_seg);
  if (!this->segio()->Write(cur_seg, this->buf())) return false;
  this->set_cur_seg(next_seg);
  if (this->log() != NULL) this->log()->on_NewSeg(cur_seg, next_seg);
  return true;
}

uint8_t* BlkWriter::Read(SegBlkNum segblk) const {
  if (SegBlkNum_seg(segblk) != this->cur_seg()) return NULL;
  BlkNum blk = SegBlkNum_blk(segblk);
  if (this->blkmap()[blk] == NULL) return NULL;
  return this->buf() + blk * this->superblock()->noctet_blk();
}

bool BlkWriter::GetMeta(SegBlkNum segblk, BlkInfo* bi) const {
  if (SegBlkNum_seg(segblk) != this->cur_seg()) return false;
  BlkNum blk = SegBlkNum_blk(segblk);
  BlkInfo* bi1 = this->blkmap()[blk];
  if (bi1 == NULL) return false;
  bi->CopyFrom(bi1);
  return true;
}

void BlkWriter::set_cur_seg(SegNum value) {
  if (this->cur_seg_ != value) {
    ::memset(this->blkmap(), 0, this->superblock()->nblk_seg() * sizeof(BlkInfo*));
  }
  this->cur_seg_ = value;
}

uint8_t* BlkWriter::buf_segsummary(void) const {
  return this->buf() + this->superblock()->first_segsummaryblk() * this->superblock()->noctet_blk();
}

BlkNum BlkWriter::AllocBlk(void) {
  for (BlkNum blk = 0; blk < this->superblock()->first_segsummaryblk(); ++blk) {
    if (this->blkmap()[blk] == NULL) return blk;
  }
  return BlkNum_invalid;
}

void BlkWriter::CollectSegSummary(SegNum next_seg) {
  uint8_t* buf_segsummary = this->buf_segsummary();
  ::memset(buf_segsummary, 0, this->superblock()->nblk_segsummary() * this->superblock()->noctet_blk());

  BlkSeq seq = 0;
  while (!this->blklist_.empty()) {
    BlkInfo* bi = this->blklist_.front();
    BlkNum blk = SegBlkNum_blk(bi->segblk());
    if (this->blkmap()[blk] == bi) {
      uint8_t* buf_rec = buf_segsummary + BlkInfo::kNoctet * blk;
      bi->set_seq(seq++);
      bi->Write(buf_rec);
    }
    delete bi;
    this->blklist_.pop();
  }
  
  SegInfo si;
  si.set_next_seg(next_seg);
  si.set_timestamp(DateTime_now());
  uint8_t* buf_si = buf_segsummary + BlkInfo::kNoctet * this->superblock()->nblk_seg();
  si.Write(buf_si);
}

};//namespace lfs
