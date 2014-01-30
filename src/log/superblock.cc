#include "log/superblock.h"
#include <cstdlib>
#include <cstring>
#include "util/protobuf.h"
#include "log/physical.pb.h"
namespace lfs {

const uint8_t Superblock::kMagic[256] = { 0xf6, 0x7f, 0xf2, 0x20, 0x7a, 0x49, 0x9a, 0x8a, 0x0a, 0x6a, 0xd1, 0xe3, 0x27, 0xc6, 0xb2, 0x1d, 0x42, 0xe1, 0xe7, 0x90, 0xb1, 0x32, 0x05, 0xfc, 0x25, 0x75, 0x89, 0xbc, 0x1f, 0x7a, 0xb3, 0x05, 0x4f, 0x6c, 0x8d, 0xd5, 0xe7, 0xc2, 0x3b, 0xe9, 0xdb, 0x76, 0x15, 0x0f, 0x94, 0xa7, 0xfa, 0xc2, 0x7d, 0x01, 0xdd, 0xfc, 0xf2, 0x73, 0x71, 0x5d, 0xad, 0x7f, 0x88, 0xfd, 0x5c, 0x20, 0x92, 0x32, 0x93, 0x80, 0x65, 0x76, 0x21, 0x18, 0xd1, 0xab, 0xf6, 0x54, 0x1c, 0xf5, 0x54, 0x8d, 0xe4, 0xb6, 0xcf, 0x1b, 0x35, 0x44, 0xa1, 0x2d, 0x90, 0x8f, 0x6e, 0xc0, 0x74, 0xc6, 0xa3, 0xe0, 0x51, 0xf3, 0xc1, 0x03, 0xa4, 0x4e, 0x0c, 0xa9, 0x9d, 0x3d, 0x8d, 0x7d, 0x8f, 0x3f, 0xea, 0x4d, 0x6a, 0x53, 0x99, 0x1a, 0x94, 0xda, 0x98, 0x2a, 0x91, 0x03, 0x8a, 0x3e, 0x7d, 0x3f, 0xf1, 0xbc, 0x75, 0x13, 0x66, 0x43, 0xd8, 0x34, 0x19, 0x34, 0xb6, 0x4a, 0x17, 0x14, 0x54, 0x3c, 0x9a, 0x18, 0x6c, 0xf9, 0x64, 0xd5, 0xeb, 0x6a, 0xfe, 0x2c, 0x72, 0xd5, 0xe0, 0x11, 0x39, 0x59, 0x7b, 0x03, 0x95, 0xa2, 0xe6, 0xa0, 0x3d, 0xee, 0x6f, 0x03, 0x26, 0x15, 0xd2, 0xdb, 0xe1, 0xee, 0xcc, 0x54, 0xf7, 0x75, 0xb4, 0xe1, 0x94, 0xb0, 0x92, 0x41, 0x5d, 0x13, 0xf6, 0xc1, 0x32, 0x67, 0x72, 0x9d, 0xe3, 0xc3, 0x53, 0x97, 0x6f, 0x79, 0x34, 0x4f, 0xb1, 0x20, 0x4f, 0x40, 0xc2, 0xba, 0x18, 0x9f, 0xec, 0x1c, 0x92, 0x53, 0xf6, 0xa6, 0x84, 0x9d, 0x5b, 0xc9, 0x4b, 0xbc, 0x08, 0xe5, 0x79, 0x13, 0xdd, 0x46 };

void Superblock::set_noctet_sector(size_t value) { this->noctet_sector_ = value; }
void Superblock::set_nsector_blk(DiskSectorNum value) { this->nsector_blk_ = value; }
void Superblock::set_nblk_seg(BlkNum value) { this->nblk_seg_ = value; }
void Superblock::set_first_logseg(SegNum value) { this->first_logseg_ = value; }
void Superblock::set_nseg_disk(SegNum value) { this->nseg_disk_ = value; }
void Superblock::set_checkpoint_seg(int index, SegNum value) { if (index == 0 || index == 1) this->checkpoint_seg_[index] = value; }
void Superblock::set_nseg_checkpoint(SegNum value) { this->nseg_checkpoint_ = value; }
void Superblock::set_nblk_segsummary(BlkNum value) { this->nblk_segsummary_ = value; }

bool Superblock::Read(Disk* disk) {
  DiskSectorNum nsector_blk = this->FindBlockSize(disk);
  if (nsector_blk == 0) return false;
  DiskSectorNum seg0blk2 = 2 * nsector_blk;
  size_t noctet_blk = nsector_blk * disk->noctet_sector();
  uint8_t* buf = (uint8_t*)::malloc(noctet_blk);
  bool success = disk->Seek(seg0blk2) && disk->Read(nsector_blk, buf)
                 && this->Read(buf, noctet_blk);
  ::free(buf);
  return success;
}

DiskSectorNum Superblock::FindBlockSize(Disk* disk) {
  uint8_t* buf = (uint8_t*)::malloc(disk->noctet_sector());
  DiskSectorNum i = 1; DiskSectorNum nsector_blk = 0;
  for (; i <= 1025; ++i) {
    disk->Seek(i);
    disk->Read(1, buf);
    if (0 == ::memcmp(buf, this->kMagic, sizeof(this->kMagic))) {
      nsector_blk = i;
      break;
    }
  }
  return nsector_blk;
}

bool Superblock::Read(uint8_t* buf, size_t noctet_blk) {
  lfs::physical::Superblock p;
  if (!(
    pb_load(&p, buf, noctet_blk) &&
    p.has_version() && p.version() == this->kLfsVersion &&
    p.has_noctet_sector() && p.has_nsector_blk() &&
    p.has_nblk_seg() && p.has_nseg_disk() &&
    p.has_first_logseg() && p.checkpoint_seg_size() == 2 &&
    p.has_nblk_segsummary()
    )) return false;
  this->set_noctet_sector(p.noctet_sector());
  this->set_nsector_blk(p.nsector_blk());
  this->set_nblk_seg(p.nblk_seg());
  this->set_nseg_disk(p.nseg_disk());
  this->set_first_logseg(p.first_logseg());
  this->set_checkpoint_seg(0, p.checkpoint_seg(0));
  this->set_checkpoint_seg(1, p.checkpoint_seg(1));
  this->set_nseg_checkpoint(p.nseg_checkpoint());
  this->set_nblk_segsummary(p.nblk_segsummary());
  return (noctet_blk == this->noctet_blk());
}

bool Superblock::Write(uint8_t* buf) const {
  lfs::physical::Superblock p;
  p.set_version(this->kLfsVersion);
  p.set_noctet_sector(this->noctet_sector());
  p.set_nsector_blk(this->nsector_blk());
  p.set_nblk_seg(this->nblk_seg());
  p.set_nseg_disk(this->nseg_disk());
  p.set_first_logseg(this->first_logseg());
  p.clear_checkpoint_seg();
  p.add_checkpoint_seg(this->checkpoint_seg(0));
  p.add_checkpoint_seg(this->checkpoint_seg(1));
  p.set_nseg_checkpoint(this->nseg_checkpoint());
  p.set_nblk_segsummary(this->nblk_segsummary());
  return pb_save(&p, buf, this->noctet_blk());
}


};//namespace lfs
