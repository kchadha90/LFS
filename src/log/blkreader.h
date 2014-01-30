#ifndef LFS_LOG_BLKREADER_H_
#define LFS_LOG_BLKREADER_H_
#include "util/defs.h"
#include "log/segio.h"
#include "log/blkwriter.h"
namespace lfs {

class BlkReader {
  public:
    BlkReader(SegIO* segio);
    Superblock* superblock(void) const { return this->segio()->superblock(); }
    void set_bw(BlkWriter* value) { this->bw_ = value; }//attach a BlkWriter so that recently written blocks are read correctly
    bool Read(SegBlkNum segblk, uint8_t* buf);//read a block
    bool Read(SegBlkNum segblk, size_t start, size_t count, uint8_t* buf);//read part of a block
    bool Read(SegBlkNum segblk, size_t start, size_t count, charbuf* cb);//read part of a block into charbuf
    bool GetMeta(SegBlkNum segblk, BlkInfo* bi);//read metadataz

  private:
    SegIO* segio_;
    BlkWriter* bw_;
    BlkWriter* bw(void) const { return this->bw_; }
    SegIO* segio(void) const { return this->segio_; }
    uint8_t* BlkWriterRead(SegBlkNum segblk);
    DISALLOW_COPY_AND_ASSIGN(BlkReader);
};


};//namespace lfs
#endif//LFS_LOG_BLKREADER_H_
