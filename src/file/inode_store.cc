#include "file/inode_store.h"
#include "file/physical.pb.h"
#include "util/protobuf.h"
#include "dir/linear_dir.h"
#include "file/sysfile_creator.h"
namespace lfs {

InodeStore::InodeStore(void) {
  this->log_ = NULL;
  this->file_map_ = NULL;
  this->file_vec_ = NULL;
  this->file_tbl_ = NULL;
}

bool InodeStore::Open(Log* log) {
  assert(log != NULL);
  this->Close();
  this->log_ = log;
  log->set_istore(this);
  this->checkpoint()->get_inode_inomap(&this->inode_map_);
  this->checkpoint()->get_inode_inovec(&this->inode_vec_);
  this->checkpoint()->get_inode_inotbl(&this->inode_tbl_);
  this->file_map_ = new File(&this->inode_map_, log);
  this->file_vec_ = new File(&this->inode_vec_, log);
  this->file_tbl_ = new File(&this->inode_tbl_, log);
  this->linear_rootdir_ = NULL;
  return true;
}

void InodeStore::Close(void) {
  if (this->log_ != NULL) {
    this->log_->set_istore(NULL);
    this->log_ = NULL;
  }
  if (this->file_map_ != NULL) delete this->file_map_;
  if (this->file_vec_ != NULL) delete this->file_vec_;
  if (this->file_tbl_ != NULL) delete this->file_tbl_;
  this->file_map_ = NULL;
  this->file_vec_ = NULL;
  this->file_tbl_ = NULL;
}

void InodeStore::set_linear_rootdir(LinearRootDirectory* value) {
  this->linear_rootdir_ = value;
  if (value != NULL) value->set_file_inomap(this->file_map());
}

bool InodeStore::InitializeInoFiles(SystemFileCreator* sysfile_creator) {
  assert(this->log_ != NULL);
  this->inode_vec_.set_size(16 * Inode::kIno_Good);
  this->inode_tbl_.set_size(256 * Inode::kIno_Good);
  return sysfile_creator->WriteInoMap(this->file_map());
}

File* InodeStore::GetInoFile(ino_t ino) {
  switch (ino) {
    case Inode::kIno_InoMap: return this->file_map();
    case Inode::kIno_InoVec: return this->file_vec();
    case Inode::kIno_InoTbl: return this->file_tbl();
  }
  return NULL;
}

void InodeStore::SaveInoFiles(void) {
  this->checkpoint()->set_inode_inomap(&this->inode_map_);
  this->checkpoint()->set_inode_inovec(&this->inode_vec_);
  this->checkpoint()->set_inode_inotbl(&this->inode_tbl_);
}

ino_t InodeStore::TransformIno(ino_t ino) const {
  if (this->linear_rootdir()) {
    return this->linear_rootdir()->TransformIno(ino);
  }
  return ino;
}

ino_t InodeStore::AllocateIno(mode_t mode, const std::string path) const {
  assert(this->log_ != NULL);
  if (!S_ISREG(mode) && !S_ISDIR(mode) && !S_ISLNK(mode)) return 0;
  if (this->linear_rootdir()) {
    return this->linear_rootdir()->AllocateIno(mode, path);
  }
  
  size_t map_size = this->inode_map_.size();
  size_t chunk_size = this->file_map()->noctet_blk();//any size works, noctet_blk is more efficient
  charbuf chunk;
  for (size_t chunk_base = 0; chunk_base < map_size; chunk_base += chunk_size) {
    chunk.clear();
    if (chunk_base + chunk_size > map_size) chunk_size = map_size - chunk_base;
    if (!this->file_map()->Read(chunk_base, chunk_size, &chunk)) return 0;
    size_t pos_free = chunk.find_first_not_of((uint8_t)0xFF, chunk_base == 0 ? Inode::kIno_Good / 8 : 0);
    if (pos_free == charbuf::npos) continue;
    uint8_t octet_free = chunk[pos_free];
    for (int bit = 0; bit < 8; ++bit) {
      if ((octet_free & (0x01 << bit)) == 0) {
        return (chunk_base + pos_free) * 8 + bit;
      }
    }
  }
  return map_size * 8 > Inode::kIno_Good ? map_size * 8 : Inode::kIno_Good;
}

bool InodeStore::AllocateIno(mode_t mode, const std::string path, Inode* inode) const {
  ino_t ino = this->AllocateIno(mode, path);
  if (ino == 0) return false;

  inode->Clear();
  inode->set_ino(ino);
  inode->set_mode(mode);
  inode->set_version(this->ReadVersion(ino) + 1);
  return true;
}

bool InodeStore::IsInUse(ino_t ino) const {
  assert(this->log_ != NULL);
  if (ino < Inode::kIno_Good) return true;
  size_t off; uint8_t mask; this->CalcMapOffsetMask(ino, &off, &mask);
  if (off >= this->inode_map_.size()) return false;
  uint8_t buf[1];
  if (!this->file_map()->Read(off, 1, buf)) return true;
  return (buf[0] & mask) != 0;
}

bool InodeStore::Read(Inode* inode) const {
  assert(this->log_ != NULL);
  assert(inode != NULL);
  ino_t ino = inode->ino();
  if (!this->IsInUse(ino)) return false;

  uint8_t vecbuf[16];
  size_t vecoff = 16 * ino;
  if (!this->file_vec()->Read(vecoff, 16, vecbuf)) return false;
  charbuf tblcb;
  size_t tbloff = 256 * ino;
  if (!this->file_tbl()->Read(tbloff, 256, &tblcb)) return false;
  lfs::physical::InodeTblEntry p;
  if (!pb_load(&p, &tblcb)) return false;

  inode->Clear();
  inode->set_ino(ino);
  inode->set_atime(be64toh(*(uint64_t*)(vecbuf + 8)));
  inode->set_version(be32toh(*(uint32_t*)(vecbuf + 0)));
  inode->set_mode(p.mode());
  inode->set_nlink(p.nlink());
  inode->set_uid(p.uid());
  inode->set_gid(p.gid());
  inode->set_size(p.size());
  inode->set_blocks(p.blocks());
  inode->set_mtime(p.mtime());
  inode->set_ctime(p.ctime());
  for (int i = 0; i < (int)Inode::kNBlks; ++i) inode->set_blkptr(i, p.blkptr(i));
  return true;
}

bool InodeStore::Write(const Inode* inode) {
  assert(this->log_ != NULL);
  assert(inode != NULL);
  assert(inode->version() != InodeVer_invalid);
  ino_t ino = inode->ino();
  if (!this->SetInUse(ino, true)) return false;

  uint8_t vecbuf[16] = {0};
  lfs::physical::InodeTblEntry p;
  *(uint64_t*)(vecbuf + 8) = htobe64(inode->atime());
  *(uint32_t*)(vecbuf + 0) = htobe32(inode->version());
  p.set_mode(inode->mode());
  p.set_nlink(inode->nlink());
  p.set_uid(inode->uid());
  p.set_gid(inode->gid());
  p.set_size(inode->size());
  p.set_blocks(inode->blocks());
  p.set_mtime(inode->mtime());
  p.set_ctime(inode->ctime());
  for (int i = 0; i < (int)Inode::kNBlks; ++i) {
    p.add_blkptr(inode->blkptr(i));
  }
  
  size_t vecoff = 16 * ino;
  if (!this->file_vec()->Write(vecoff, 16, vecbuf)) return false;
  uint8_t tblbuf[256] = {0};
  if (!pb_save(&p, tblbuf, 256)) return false;
  size_t tbloff = 256 * ino;
  if (!this->file_tbl()->Write(tbloff, 256, tblbuf)) return false;
  return true;
}

bool InodeStore::Delete(ino_t ino) {
  assert(this->log_ != NULL);
  return this->SetInUse(ino, false);
}

InodeVer InodeStore::ReadVersion(ino_t ino) const {
  assert(this->log_ != NULL);
  size_t vecoff = 16 * ino;
  if (vecoff + 16 > this->inode_vec_.size()) return 0;
  InodeVer value;
  if (!this->file_vec()->Read(vecoff + 0, sizeof(value), (uint8_t*)&value)) return 0;
  return be32toh(value);
}

inline void InodeStore::CalcMapOffsetMask(ino_t ino, size_t* off, uint8_t* mask) const {
  *off = ino / 8;
  *mask = 0x01 << (ino % 8);
}

bool InodeStore::SetInUse(ino_t ino, bool in_use) {
  size_t off; uint8_t mask; this->CalcMapOffsetMask(ino, &off, &mask);
  uint8_t buf[1] = {0};
  if (off < this->inode_map_.size() && !this->file_map()->Read(off, 1, buf)) return false;

  buf[0] &= ~mask;
  if (in_use) buf[0] |= mask;
  return this->file_map()->Write(off, 1, buf);
}

};//namespace lfs
