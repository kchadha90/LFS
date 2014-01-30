#ifndef LFS_DISK_DISK_H_
#define LFS_DISK_DISK_H_
#include "util/defs.h"
namespace lfs {

typedef uint32_t DiskSectorNum;

//abstract Disk class
class Disk {
  public:
    DiskSectorNum nsector(void) const { return this->nsector_; }
    uint32_t noctet_sector(void) const { return this->noctet_sector_; }
    virtual bool Seek(DiskSectorNum sector) =0;
    virtual bool Read(DiskSectorNum sector_count, uint8_t* buf) =0;
    virtual bool Write(DiskSectorNum sector_count, const uint8_t* buf) =0;
    
  protected:
    Disk(void);
    void set_nsector(DiskSectorNum value) { this->nsector_ = value; }
    void set_noctet_sector(uint32_t value) { this->noctet_sector_ = value; }
    
  private:
    DiskSectorNum nsector_;
    uint16_t noctet_sector_;
    DISALLOW_COPY_AND_ASSIGN(Disk);
};

inline Disk::Disk(void) {
  this->nsector_ = 0;
  this->noctet_sector_ = 0;
}

};//namespace lfs
#endif//LFS_DISK_DISK_H_
