#include "disk/vdisk.h"
#include <sys/types.h>
#include "util/logging.h"
namespace lfs {

VDisk::VDisk(void) {
  this->open_ = false;
  this->inner_disk_ = NULL;
  this->cur_sector_ = 0;
}

bool VDisk::Open(const char* filename) {
  assert(filename != NULL);
  this->Close();
  u_int sector_count;
  this->inner_disk_ = ::Disk_Open((char*)filename, 0, &sector_count);
  if (this->inner_disk_ == NULL) return false;
  this->set_nsector(sector_count);
  this->set_noctet_sector(DISK_SECTOR_SIZE);
  this->open_ = true;
  return true;
}

bool VDisk::Close(void) {
  if (!this->open_) return false;
  this->open_ = false;
  int res = ::Disk_Close(this->inner_disk_);
  return res == 0;
}

bool VDisk::Seek(DiskSectorNum sector) {
  assert(this->open_);
  if (sector >= this->nsector()) {
    Logging::Log(kLLWarn, kLCDisk, "VDisk::Seek(%"PRIu32") seek beyond end", sector);
    return false;
  }
  this->cur_sector_ = sector;
  return true;
}

bool VDisk::Read(DiskSectorNum sector_count, uint8_t* buf) {
  assert(this->open_);
  int res = ::Disk_Read(this->inner_disk_, (u_int)this->cur_sector_, (u_int)sector_count, buf);
  if (res != 0) {
    Logging::Log(kLLError, kLCDisk, "VDisk::Read(sector=%"PRIu32",count=%"PRIu32") Disk_Read error=%d", this->cur_sector_, sector_count, res);
  }
  return res == 0;
}

bool VDisk::Write(DiskSectorNum sector_count, const uint8_t* buf) {
  assert(this->open_);
  int res = ::Disk_Write(this->inner_disk_, (u_int)this->cur_sector_, (u_int)sector_count, (void*)buf);
  if (res != 0) {
    Logging::Log(kLLError, kLCDisk, "VDisk::Write(sector=%"PRIu32",count=%"PRIu32") Disk_Write error=%d", this->cur_sector_, sector_count, res);
  }
  return res == 0;
}

};//namespace lfs
