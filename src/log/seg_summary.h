#ifndef LFS_LOG_SEG_SUMMARY_H_
#define LFS_LOG_SEG_SUMMARY_H_
#include "util/defs.h"
#include <sys/stat.h>
#include "util/time.h"
#include "file/inode.h"
namespace lfs {

enum BlkType {
  kBTSegSum = 0x0,
  kBTBoot = 0x1,
  kBTSuper = 0x2,
  kBTChkPt = 0x3,
  kBTDirCh = 0x4,
  kBTInd = 0x5,
  kBTData = 0xF
};

typedef uint16_t BlkSeq;
const BlkSeq BlkSeq_max = 0x0FFF;
const BlkSeq BlkSeq_free = 0x0FFF;//indicates a block is free when writing the segment

typedef uint32_t BlkOffset;
const BlkOffset BlkOffset_invalid = 0xFFFFFFFF;

//per block info in segment summary
//unimplemented: indirect block level field
class BlkInfo {
  public:
    static const size_t kNoctet = 24;//storage length
    BlkInfo(void) {}
    BlkInfo(const BlkInfo& other) { this->CopyFrom(&other); }
    SegBlkNum segblk(void) const { return this->segblk_; }
    void set_segblk(SegBlkNum value) { this->segblk_ = value; }
    BlkSeq seq(void) const { return this->seq_; }
    bool set_seq(BlkSeq value);
    BlkType type(void) const { return this->type_; }
    void set_type(BlkType value) { this->type_ = value; }
    ino_t ino(void) const { return this->ino_; }
    void set_ino(ino_t value) { this->ino_ = value; }
    InodeVer ver(void) const { return this->ver_; }
    void set_ver(InodeVer value) { this->ver_ = value; }
    BlkOffset off(void) const { return this->off_; }
    void set_off(BlkOffset value) { this->off_ = value; }
    DateTime mtime(void) const { return this->mtime_; }
    void set_mtime(DateTime value) { this->mtime_ = value; }
    bool Read(uint8_t* buf);//read from buf
    bool Write(uint8_t* buf) const;//write to buf
    void CopyFrom(const BlkInfo* other);
    BlkInfo& operator=(const BlkInfo& other) { this->CopyFrom(&other); return *this; }
    
  private:
    SegBlkNum segblk_;//SegBlkNum, not written to disk
    BlkSeq seq_;//sequence within a segment
    BlkType type_;//type of block
    ino_t ino_;//type=Ind,Data: inode number
    InodeVer ver_;//type=Ind,Data: inode version
    BlkOffset off_;//type=Data: file offset (in blocks); type=Ind: first pointer's file offset (in blocks)
    DateTime mtime_;//type=Data: time this data was created (does not change after moving by cleaner)
};

//per segment info in segment summary
class SegInfo {
  public:
    static const size_t kNoctet = 24;
    SegInfo(void) {}
    SegNum next_seg(void) const { return this->next_seg_; }
    void set_next_seg(SegNum value) { this->next_seg_ = value; }
    DateTime timestamp(void) const { return this->timestamp_; }
    void set_timestamp(DateTime value) { this->timestamp_ = value; }
    bool Read(uint8_t* buf);//read from buf
    bool Write(uint8_t* buf) const;//write to buf

  private:
    SegNum next_seg_;//next segment number
    DateTime timestamp_;//timestamp this segment is written
    DISALLOW_COPY_AND_ASSIGN(SegInfo);
};

inline bool BlkInfo::set_seq(BlkSeq value) {
  if (value > BlkSeq_max) return false;
  this->seq_ = value;
  return true;
}

};//namespace lfs
#endif//LFS_LOG_SEG_SUMMARY_H_
