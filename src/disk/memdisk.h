#ifndef LFS_DISK_MEMDISK_H_
#define LFS_DISK_MEMDISK_H_
#include "util/defs.h"
#include "disk/disk.h"
namespace lfs {

//a Disk backed by memory
class MemDisk : public Disk {
  public:
    MemDisk(void);
    ~MemDisk(void);
    bool Init(uint32_t noctet_sector, DiskSectorNum nsector);
    virtual bool Seek(DiskSectorNum sector);
    virtual bool Read(DiskSectorNum sector_count, uint8_t* buf);
    virtual bool Write(DiskSectorNum sector_count, const uint8_t* buf);
    
  private:
    void CalcOffset(DiskSectorNum sector_count, uint8_t** offset, size_t* count) const;
    uint8_t* buf_;
    DiskSectorNum cur_sector_;
    DISALLOW_COPY_AND_ASSIGN(MemDisk);
};

};//namespace lfs
#endif//LFS_DISK_VDISK_H_
