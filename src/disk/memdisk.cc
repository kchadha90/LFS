#include "disk/memdisk.h"
#include <cstdlib>
#include <cstring>
namespace lfs {

MemDisk::MemDisk(void) {
  this->buf_ = NULL;
  this->cur_sector_ = 0;
}

MemDisk::~MemDisk(void) {
  if (this->buf_ != NULL) free(this->buf_);
}

bool MemDisk::Init(uint32_t noctet_sector, DiskSectorNum nsector) {
  assert(this->buf_ == NULL);
  assert(noctet_sector > 0);
  this->set_noctet_sector(noctet_sector);
  this->set_nsector(nsector);
  this->buf_ = (uint8_t*)std::calloc(nsector, noctet_sector);
  if (this->buf_ == NULL) return false;
  return true;
}

bool MemDisk::Seek(DiskSectorNum sector) {
  if (sector < 0 || sector >= this->nsector()) return false;
  this->cur_sector_ = sector;
  return true;
}

void MemDisk::CalcOffset(DiskSectorNum sector_count, uint8_t** offset, size_t* count) const {
  *offset = this->buf_ + this->cur_sector_ * this->noctet_sector();
  *count = sector_count * this->noctet_sector();
}

bool MemDisk::Read(DiskSectorNum sector_count, uint8_t* buf) {
  assert(this->buf_ != NULL);
  uint8_t* offset; size_t count;
  this->CalcOffset(sector_count, &offset, &count);
  std::memcpy(buf, offset, count);
  return true;
}

bool MemDisk::Write(DiskSectorNum sector_count, const uint8_t* buf) {
  assert(this->buf_ != NULL);
  uint8_t* offset; size_t count;
  this->CalcOffset(sector_count, &offset, &count);
  std::memcpy(offset, buf, count);
  return true;
}

};//namespace lfs
