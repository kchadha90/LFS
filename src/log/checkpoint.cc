#include "log/checkpoint.h"
#include "cleaner/segusage.h"
#include "util/protobuf.h"
#include "util/logging.h"
namespace lfs {

Checkpoint::Checkpoint(SegIO* segio) {
  assert(segio != NULL);
  this->segio_ = segio;
  this->p_ = new lfs::physical::Checkpoint();
  this->segusage_ = new SegUsage(this, this->p_);
}

Checkpoint::~Checkpoint(void) {
  delete this->p_;
}

bool Checkpoint::Read(int regionIndex) {
  charbuf cb;
  if (!this->segio()->ReadMulti(this->superblock()->checkpoint_seg(regionIndex), this->superblock()->nseg_checkpoint(), &cb)) return false;
  if (!pb_load(this->p(), &cb)) this->p()->Clear();

  DateTime dup_timestamp = be64toh(*(DateTime*)(cb.data() + this->off_dup_timestamp()));
  if (dup_timestamp != this->timestamp()) this->p()->Clear();
  if (!this->IsValid()) return false;
  this->segusage()->Read();
  return true;
}

bool Checkpoint::Write(int regionIndex) {
  Logging::Log(kLLInfo, kLCLog, "Checkpoint::Write(region=%d)", regionIndex);
  this->segusage()->Write();
  charbuf cb;
  pb_save(this->p(), &cb);
  cb.resize(this->superblock()->noctet_seg() * this->superblock()->nseg_checkpoint());
  DateTime dup_timestamp = htobe64(this->timestamp());
  cb.replace(this->off_dup_timestamp(), sizeof(dup_timestamp), (uint8_t*)&dup_timestamp, sizeof(dup_timestamp));
  bool res = this->segio()->WriteMulti(this->superblock()->checkpoint_seg(regionIndex), this->superblock()->nseg_checkpoint(), cb.data());
  this->segusage()->on_Checkpoint();
  return res;
}

bool Checkpoint::IsValid(void) {
  return this->timestamp() != 0;
}

bool Checkpoint::IsPreferred(Checkpoint* other) {
  assert(other != NULL);
  if (!this->IsValid()) return false;
  if (!other->IsValid()) return true;
  return this->timestamp() > other->timestamp();
}

DateTime Checkpoint::timestamp(void) const {
  return this->p()->timestamp();
}

void Checkpoint::set_timestamp(DateTime value) {
  this->p()->set_timestamp(value);
}

SegNum Checkpoint::next_seg(void) const {
  return this->p()->next_seg();
}

void Checkpoint::set_next_seg(SegNum value) {
  this->p()->set_next_seg(value);
}

void Checkpoint::get_inode_inomap(Inode* inode) const {
  this->get_inode(this->p()->mutable_inode_inomap(), Inode::kIno_InoMap, inode);
}

void Checkpoint::set_inode_inomap(Inode* inode) {
  this->set_inode(this->p()->mutable_inode_inomap(), Inode::kIno_InoMap, inode);
}

void Checkpoint::get_inode_inovec(Inode* inode) const {
  this->get_inode(this->p()->mutable_inode_inovec(), Inode::kIno_InoVec, inode);
}

void Checkpoint::set_inode_inovec(Inode* inode) {
  this->set_inode(this->p()->mutable_inode_inovec(), Inode::kIno_InoVec, inode);
}

void Checkpoint::get_inode_inotbl(Inode* inode) const {
  this->get_inode(this->p()->mutable_inode_inotbl(), Inode::kIno_InoTbl, inode);
}

void Checkpoint::set_inode_inotbl(Inode* inode) {
  this->set_inode(this->p()->mutable_inode_inotbl(), Inode::kIno_InoTbl, inode);
}

size_t Checkpoint::off_dup_timestamp(void) const {
  return (this->superblock()->nblk_seg() * this->superblock()->nseg_checkpoint() - 1)
         * this->superblock()->noctet_blk();
}

void Checkpoint::get_inode(lfs::physical::InodeSimp* p, ino_t ino, Inode* inode) const {
  assert(p != NULL);
  assert(inode != NULL);
  inode->Clear();
  inode->set_ino(ino);
  inode->set_mode(S_IFREG);
  inode->set_nlink(1);
  inode->set_size(p->size());
  inode->set_blocks(inode->size() / this->superblock()->noctet_blk() + 1);//inaccurate but sufficient
  for (int i = 0; i < Inode::kNBlks; ++i) inode->set_blkptr(i, p->blkptr(i));
  inode->set_version(p->ver());
}

void Checkpoint::set_inode(lfs::physical::InodeSimp* p, ino_t ino, Inode* inode) const {
  assert(p != NULL);
  assert(inode != NULL);
  assert(ino == inode->ino());
  p->set_size(inode->size());
  p->clear_blkptr();
  for (int i = 0; i < Inode::kNBlks; ++i) p->add_blkptr(inode->blkptr(i));
  p->set_ver(inode->version());
}

Checkpoints::Checkpoints(SegIO* segio) {
  assert(segio != NULL);
  this->set_segio(segio);
  this->set_c(NULL);
}

Checkpoints::~Checkpoints(void) {
  if (this->c() != NULL) {
    delete this->c();
  }
}

bool Checkpoints::Read(void) {
  if (this->c() != NULL) {
    delete this->c();
  }
  Checkpoint* c0 = new Checkpoint(this->segio()); c0->Read(0);
  Checkpoint* c1 = new Checkpoint(this->segio()); c1->Read(1);
  if (c0->IsValid() || c1->IsValid()) {
    if (c0->IsPreferred(c1)) {
      this->set_preferred_region(0);
      this->set_c(c0);
      delete c1;
    } else {
      this->set_preferred_region(1);
      this->set_c(c1);
      delete c0;
    }
    return true;
  } else {
    delete c0; delete c1;
    this->set_c(NULL);
    return false;
  }
}

bool Checkpoints::Write(void) {
  int write_region = 1 - this->preferred_region();
  this->c()->set_timestamp(DateTime_now());
  if (!this->c()->Write(write_region)) return false;
  this->set_preferred_region(write_region);
  return true;
}


};//namespace lfs
