#include "disk/logging_disk.h"
#include "util/logging.h"
namespace lfs {

LoggingDisk::LoggingDisk(Disk* inner_disk) {
  assert(inner_disk != NULL);
  this->inner_disk_ = inner_disk;
  this->set_nsector(inner_disk->nsector());
  this->set_noctet_sector(inner_disk->noctet_sector());
}

bool LoggingDisk::Seek(DiskSectorNum sector) {
  bool res = this->inner_disk()->Seek(sector);
  Logging::Log(kLLDebug, kLCDisk, "Seek(%"PRIu32") %d", sector, res);
  return res;
}

bool LoggingDisk::Read(DiskSectorNum sector_count, uint8_t* buf) {
  bool res = this->inner_disk()->Read(sector_count, buf);
  Logging::Log(kLLDebug, kLCDisk, "Read(%"PRIu32") %d", sector_count, res);
  return res;
}

bool LoggingDisk::Write(DiskSectorNum sector_count, const uint8_t* buf) {
  bool res = this->inner_disk()->Write(sector_count, buf);
  Logging::Log(kLLDebug, kLCDisk, "Read(%"PRIu32") %d", sector_count, res);
  return res;
}

};//namespace lfs
