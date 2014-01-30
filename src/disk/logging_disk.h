#ifndef LFS_DISK_LOGGING_DISK_H_
#define LFS_DISK_LOGGING_DISK_H_
#include "util/defs.h"
#include "disk/disk.h"
namespace lfs {

//a Disk that writes every operation to the Logging component
class LoggingDisk : public Disk {
  public:
    explicit LoggingDisk(Disk* inner_disk);
    Disk* inner_disk(void) const { return this->inner_disk_; }
    virtual bool Seek(DiskSectorNum sector);
    virtual bool Read(DiskSectorNum sector_count, uint8_t* buf);
    virtual bool Write(DiskSectorNum sector_count, const uint8_t* buf);
    
  private:
    Disk* inner_disk_;
    DISALLOW_COPY_AND_ASSIGN(LoggingDisk);
};

};//namespace lfs
#endif//LFS_DISK_LOGGING_DISK_H_
