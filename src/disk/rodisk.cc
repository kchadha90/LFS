#include "disk/rodisk.h"
namespace lfs {

ReadOnlyDisk::ReadOnlyDisk(Disk* inner_disk) {
  assert(inner_disk != NULL);
  this->inner_disk_ = inner_disk;
  this->set_nsector(inner_disk->nsector());
  this->set_noctet_sector(inner_disk->noctet_sector());
}

bool ReadOnlyDisk::Seek(DiskSectorNum sector) {
  bool res = this->inner_disk()->Seek(sector);
  return res;
}

bool ReadOnlyDisk::Read(DiskSectorNum sector_count, uint8_t* buf) {
  bool res = this->inner_disk()->Read(sector_count, buf);
  return res;
}

bool ReadOnlyDisk::Write(DiskSectorNum sector_count, const uint8_t* buf) {
  return false;
}

};//namespace lfs
