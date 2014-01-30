#ifndef LFS_FILE_INODE_H_
#define LFS_FILE_INODE_H_
#include "util/defs.h"
#include <sys/stat.h>
#include "util/time.h"
#include "log/segblknum.h"
namespace lfs {

//inode version
typedef uint32_t InodeVer;
const InodeVer InodeVer_invalid = 0xFFFFFFFF;

//inode
//unimplemented: store file body in inode
class Inode {
  public:
    static const ino_t kIno_Root = 2;//root directory
    static const ino_t kIno_UnlinkedDir = 20;//unlinked directory
    static const ino_t kIno_InoMap = 16;//inode bitmap
    static const ino_t kIno_InoVec = 17;//inode vector
    static const ino_t kIno_InoTbl = 18;//inode table
    static const ino_t kIno_Good = 64;//first non-reserved ino
    static const nlink_t kLinksMax = 65000;
    static const int kNDirBlks = 12;
    static const int kIndBlk = kNDirBlks;
    static const int kDIndBlk = kIndBlk + 1;
    static const int kTIndBlk = kDIndBlk + 1;
    static const int kNBlks = kTIndBlk + 1;

    Inode(void);
    void Clear(void);//clear all fields except ino to zero
    void CopyFrom(Inode* other);
    bool Equals(Inode* other);

    ino_t ino(void) const { return this->ino_; }
    void set_ino(ino_t value) { this->ino_ = value; }
    mode_t mode(void) const { return this->mode_; }
    void set_mode(mode_t value) { this->mode_ = value; }
    nlink_t nlink(void) const { return this->nlink_; }
    bool set_nlink(nlink_t value);
    bool inc_nlink(void) { return this->set_nlink(this->nlink() + 1); }
    bool dec_nlink(void);
    uid_t uid(void) const { return this->uid_; }
    void set_uid(uid_t value) { this->uid_ = value; }
    gid_t gid(void) const { return this->gid_; }
    void set_gid(gid_t value) { this->gid_ = value; }
    size_t size(void) const { return this->size_; }
    void set_size(size_t value) { this->size_ = value; }
    blkcnt_t blocks(void) const { return this->blocks_; }
    void set_blocks(blkcnt_t value) { this->blocks_ = value; }
    DateTime atime(void) const { return this->atime_; }
    void set_atime(DateTime value) { this->atime_ = value; }
    DateTime mtime(void) const { return this->mtime_; }
    void set_mtime(DateTime value) { this->mtime_ = value; }
    DateTime ctime(void) const { return this->ctime_; }
    void set_ctime(DateTime value) { this->ctime_ = value; }
    void SetTimes(DateTime value);
    SegBlkNum blkptr(int i) const { return (i>=0 && i<kNBlks) ? this->blkptr_[i] : 0; }
    bool set_blkptr(int i, SegBlkNum value);
    InodeVer version(void) const { return this->version_; }
    void set_version(InodeVer value) { this->version_ = value; }
    
  private:
    ino_t ino_;
    mode_t mode_;
    nlink_t nlink_;
    uid_t uid_;
    gid_t gid_;
    size_t size_;
    blkcnt_t blocks_;
    DateTime atime_;
    DateTime mtime_;
    DateTime ctime_;
    SegBlkNum blkptr_[kNBlks];
    InodeVer version_;
    DISALLOW_COPY_AND_ASSIGN(Inode);
};

inline bool Inode::set_nlink(nlink_t value) {
  if (value > kLinksMax) return false;
  this->nlink_ = value;
  return true;
}

inline bool Inode::dec_nlink(void) {
  if (this->nlink() == 0) return false;
  return this->set_nlink(this->nlink() - 1);
}

inline bool Inode::set_blkptr(int i, SegBlkNum value) {
  if (i < 0 || i > kNBlks) return false;
  this->blkptr_[i] = value;
  return true;
}

};//namespace lfs
#endif//LFS_FILE_INODE_H_
