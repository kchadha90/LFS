#ifndef LFS_DISK_VDISK_H_
#define LFS_DISK_VDISK_H_
#include "util/defs.h"
#include "exttool/disk.h"
#include "disk/disk.h"
namespace lfs {

//a disk backed by CSC552 disk.h
class VDisk : public Disk {
  public:
    VDisk();
    bool Open(const char* filename);
    bool Close(void);
    virtual bool Seek(DiskSectorNum sector);
    virtual bool Read(DiskSectorNum sector_count, uint8_t* buf);
    virtual bool Write(DiskSectorNum sector_count, const uint8_t* buf);
    
  private:
    bool open_;//true after successful Init(), false after Close()
    ::Disk inner_disk_;
    DiskSectorNum cur_sector_;
    DISALLOW_COPY_AND_ASSIGN(VDisk);
};

};//namespace lfs
#endif//LFS_DISK_VDISK_H_
