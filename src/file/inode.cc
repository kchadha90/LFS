#include "file/inode.h"
#include <cstring>
namespace lfs {

Inode::Inode(void) {
  this->ino_ = 0;
  this->Clear();
}

void Inode::Clear(void) {
  this->mode_ = 0;
  this->nlink_ = 0;
  this->uid_ = 0;
  this->gid_ = 0;
  this->size_ = 0;
  this->blocks_ = 0;
  this->atime_ = 0;
  this->mtime_ = 0;
  this->ctime_ = 0;
  for (int i = 0; i < Inode::kNBlks; ++i) this->blkptr_[i] = SegBlkNum_unassigned;
  this->version_ = InodeVer_invalid;
}

void Inode::CopyFrom(Inode* other) {
  this->ino_ = other->ino_;
  this->mode_ = other->mode_;
  this->nlink_ = other->nlink_;
  this->uid_ = other->uid_;
  this->gid_ = other->gid_;
  this->size_ = other->size_;
  this->blocks_ = other->blocks_;
  this->atime_ = other->atime_;
  this->mtime_ = other->mtime_;
  this->ctime_ = other->ctime_;
  memcpy(this->blkptr_, other->blkptr_, kNBlks * sizeof(SegBlkNum));
  this->version_ = other->version_;
}

bool Inode::Equals(Inode* other) {
  return this->ino_ == other->ino_
      && this->mode_ == other->mode_
      && this->nlink_ == other->nlink_
      && this->uid_ == other->uid_
      && this->gid_ == other->gid_
      && this->size_ == other->size_
      && this->blocks_ == other->blocks_
      && this->atime_ == other->atime_
      && this->mtime_ == other->mtime_
      && this->ctime_ == other->ctime_
      && 0 == memcmp(this->blkptr_, other->blkptr_, kNBlks * sizeof(SegBlkNum))
      && this->version_ == other->version_;
}

void Inode::SetTimes(DateTime value) {
  this->set_atime(value);
  this->set_mtime(value);
  this->set_ctime(value);
}

};//namespace lfs
