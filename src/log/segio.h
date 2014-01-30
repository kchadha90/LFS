#ifndef LFS_LOG_SEGIO_H_
#define LFS_LOG_SEGIO_H_
#include "util/defs.h"
#include "disk/disk.h"
#include "log/superblock.h"
#include "util/cache.h"
namespace lfs {

//segment I/O
//reading is cached, writing is through
class SegIO {
  public:
    SegIO(Disk* disk, Superblock* superblock, SegNum cache_capacity);
    ~SegIO(void);
    Superblock* superblock(void) const { return this->superblock_; }
    bool protect_seg0(void) const { return this->protect_seg0_; }
    void set_protect_seg0(bool value) { this->protect_seg0_ = value; }
    bool Read(SegNum seg, uint8_t* cb);//read a segment
    bool ReadMulti(SegNum seg_start, SegNum seg_count, charbuf* cb);//read multiple segments into charbuf
    bool Read(SegNum seg, size_t start, size_t count, uint8_t* buf);//read part of a segment into start of buf
    bool Read(SegNum seg, size_t start, size_t count, charbuf* cb);//read part of a segment into charbuf
    bool Write(SegNum seg, const uint8_t* buf);//write a segment (invalidates cache)
    bool WriteMulti(SegNum seg_start, SegNum seg_count, const uint8_t* buf);//write multiple segments
  
  private:
    class SegCache : public Cache<SegNum,charbuf*> {
      public:
        SegCache(size_t capacity) : Cache<SegNum,charbuf*>(capacity) {}
      protected:
        void DestroyValue(charbuf* x) { delete x; }
      private:
        DISALLOW_COPY_AND_ASSIGN(SegCache);
    };

    Disk* disk_;
    Superblock* superblock_;
    bool protect_seg0_;//whether writing to segment 0 is disallowed
    DiskSectorNum nsector_seg_;
    SegCache* cache_;
    Disk* disk(void) const { return this->disk_; }
    SegCache* cache(void) const { return this->cache_; }
    bool SeekTo(SegNum seg);
    charbuf* ReadSeg(SegNum seg);//read a segment from cache or disk
    DISALLOW_COPY_AND_ASSIGN(SegIO);
};

};//namespace lfs
#endif//LFS_LOG_SEGIO_H_
