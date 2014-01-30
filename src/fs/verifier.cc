#include "fs/verifier.h"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <unordered_set>
namespace lfs {

FsVerifier::FsVerifier(void) {
  this->disk_ = NULL;
  this->log_ = NULL;
  this->fs_ = NULL;
}

FsVerifier::~FsVerifier(void) {
  if (this->fs_ != NULL) {
    this->fs_->Close();
    delete this->fs_;
  }
  if (this->log_ != NULL) {
    this->log_->Close();
    delete this->log_;
  }
  if (this->disk_ != NULL) {
    delete this->disk_;
  }
}

bool FsVerifier::Verify(Disk* disk) {
  assert(disk != NULL);
  this->disk_ = new ReadOnlyDisk(disk);
  this->msgs_.clear();
  this->live_blocks_.clear();
  this->inode_nlink_.clear();
  this->file_nlink_.clear();
  
  bool res = true;
  res = res && this->InitLog();
  res = res && this->InitFs();
  res = res && this->IterateInodes();
  res = res && this->VerifyNlink();
  res = res && this->VerifySegUsage();
  return res;
}

void FsVerifier::AddMsg(const char* format, ...) {
  char* str;
  va_list argptr;
  va_start(argptr, format);
  vasprintf(&str, format, argptr);
  va_end(argptr);
  this->msgs_.push_back(str);
  free(str);
}

inline void FsVerifier::AddLiveBlock(SegBlkNum segblk) {
  SegNum seg = SegBlkNum_seg(segblk);
  this->live_blocks_[seg] += 1;
}

inline void FsVerifier::AddFileNlink(ino_t ino) {
  this->file_nlink_[ino] += 1;
}

bool FsVerifier::InitLog(void) {
  this->log_ = new Log();
  this->log()->set_segcache_capacity(128);
  if (this->log()->Open(this->disk())) return true;
  this->AddMsg("Log::Open fails: problem with superblock or checkpoint");
  return false;
}

bool FsVerifier::InitFs(void) {
  this->fs_ = new Filesystem();
  if (this->fs()->Open(this->log())) return true;
  this->AddMsg("Filesystem::Open fails");
  return false;
}

bool FsVerifier::IterateInodes(void) {
  bool res = true;
  res = res && this->VerifyFile(Inode::kIno_Root);
  res = res && this->VerifyFile(Inode::kIno_UnlinkedDir);
  res = res && this->VerifySystemFile(Inode::kIno_InoMap);
  res = res && this->VerifySystemFile(Inode::kIno_InoVec);
  res = res && this->VerifySystemFile(Inode::kIno_InoTbl);
  
  for (ino_t ino = Inode::kIno_Good, ubound = this->DetermineMaxIno(); ino < ubound; ++ino) {
    if (this->fs()->istore()->IsInUse(ino)) {
      res = res && this->VerifyFile(ino);
    }
  }
  return res;
}

ino_t FsVerifier::DetermineMaxIno(void) {
  Inode inode;
  this->log()->checkpoint()->get_inode_inomap(&inode);
  return inode.size() * 8;
}

bool FsVerifier::VerifyFile(ino_t ino) {
  File* file = this->fs()->GetFile(ino);
  if (file == NULL) {
    this->AddMsg("Filesystem::GetFile(%"PRIuMAX") returns NULL", (uintmax_t)ino);
    return false;
  }
  nlink_t nlink = file->inode()->nlink();
  this->inode_nlink_[ino] = nlink;
  
  bool res = this->VerifyFile(file);
  this->fs()->ReleaseFile(file);
  return res;
}

bool FsVerifier::VerifySystemFile(ino_t ino) {
  Inode inode;
  switch (ino) {
    case Inode::kIno_InoMap: this->log()->checkpoint()->get_inode_inomap(&inode); break;
    case Inode::kIno_InoVec: this->log()->checkpoint()->get_inode_inovec(&inode); break;
    case Inode::kIno_InoTbl: this->log()->checkpoint()->get_inode_inotbl(&inode); break;
    default: return false;
  }
  File file(&inode, this->log());
  return this->VerifyFile(&file);
}

bool FsVerifier::VerifyFile(File* file) {
  bool res = true;
  Inode* inode = file->inode(); ino_t ino = inode->ino(); mode_t mode = inode->mode();
  if (S_ISDIR(mode)) {
    Directory* dir = this->fs()->GetDirectory(ino);
    if (dir == NULL) {
      this->AddMsg("Filesystem::GetDirectory(%"PRIuMAX") returns NULL", (uintmax_t)ino);
      res = false;
    } else {
      res = res && this->VerifyDirectory(dir);
      this->fs()->ReleaseDirectory(dir);
    }
  } else if (S_ISREG(mode) || S_ISLNK(mode)) {
  } else {
    this->AddMsg("inode(%"PRIuMAX") unsupported mode %07"PRIo16, (uintmax_t)ino, (uint16_t)mode);
    res = false;
  }
  
  if (inode->size() == 0) return res;
  
  BlkOffset last_blkoff = file->CalcBlkOffset(inode->size() - 1);
  if (last_blkoff >= (BlkOffset)Inode::kNDirBlks) {
    SegBlkNum segblk = inode->blkptr(Inode::kIndBlk);
    res = res && this->VerifyBlock(segblk, inode, kBTInd, Inode::kNDirBlks);
  }
  for (BlkOffset blkoff = 0; blkoff <= last_blkoff; ++blkoff) {
    SegBlkNum segblk = file->GetAddress(blkoff);
    res = res && this->VerifyBlock(segblk, inode, kBTData, blkoff);
  }
  
  return res;
}

bool FsVerifier::VerifyBlock(SegBlkNum segblk, Inode* inode, BlkType type, BlkOffset blkoff) {
  char* type_str;
  if (type == kBTData) type_str = (char*)"Data";
  else if (type == kBTInd) type_str = (char*)"Ind";
  else assert(false);
  ino_t ino = inode->ino();
  
  if (segblk == SegBlkNum_invalid) {
    this->AddMsg("file(%"PRIuMAX") blkoff(%s %"PRIu32") is SegBlkNum_invalid", (uintmax_t)inode->ino(), type_str, blkoff);
    return false;
  } else if (segblk == SegBlkNum_unassigned) {
  } else {
    BlkInfo bi;
    if (!this->log()->br()->GetMeta(segblk, &bi)) {
      this->AddMsg("BlkReader::GetMeta(%016"PRIx64") fails for file(%"PRIuMAX") blkoff(%s %"PRIu32")", segblk, (uintmax_t)ino, type_str, blkoff);
      return false;
    } else if (bi.type() != type || bi.ino() != ino || bi.ver() != inode->version() || bi.off() != blkoff) {
      this->AddMsg("meta(%016"PRIx64") mismatches for file(%"PRIuMAX") blkoff(%s %"PRIu32")", segblk, (uintmax_t)ino, type_str, blkoff);
      return false;
    } else {
      this->AddLiveBlock(segblk);
    }
  }
  return true;
}

bool FsVerifier::VerifyDirectory(Directory* dir) {
  ino_t ino = dir->inode()->ino();
  std::vector<std::string> names;
  if (!dir->List(&names)) {
    this->AddMsg("directory(%"PRIuMAX")->List() fails", (uintmax_t)ino);
    return false;
  }

  bool res = true;
  std::unordered_set<std::string> nameset;//detecting duplicate names
  for (std::vector<std::string>::iterator it = names.begin(); it < names.end(); ++it) {
    std::string name = *it;
    if (nameset.count(name) == 1) {
      this->AddMsg("directory(%"PRIuMAX") has duplicate name '%s'", (uintmax_t)ino, name.c_str());
    }
    nameset.insert(name);
    ino_t name_ino = dir->Get(name);
    if (name_ino == 0) {
      this->AddMsg("directory(%"PRIuMAX")->Get('%s') returns 0", (uintmax_t)ino, name.c_str());
      res = false;
    } else {
      this->AddFileNlink(name_ino);
    }
  }
  return res;
}

bool FsVerifier::VerifyNlink(void) {
  this->file_nlink_[(ino_t)Inode::kIno_Root] = 1;
  this->file_nlink_[(ino_t)Inode::kIno_UnlinkedDir] = 1;
  bool res = true;

  for (std::unordered_map<ino_t,nlink_t>::const_iterator it = this->inode_nlink_.cbegin(); it != this->inode_nlink_.cend(); ++it) {
    ino_t ino = it->first;
    nlink_t nlink_inode = it->second;
    nlink_t nlink_dir;
    std::unordered_map<ino_t,nlink_t>::iterator it2 = this->file_nlink_.find(ino);
    if (it2 == this->file_nlink_.end()) {
      nlink_dir = 0;
    } else {
      nlink_dir = it2->second;
    }
    if (nlink_inode != nlink_dir) {
      this->AddMsg("inode(%"PRIuMAX").nlink=%"PRIuMAX" mismatches %"PRIuMAX" counted from directories", (uintmax_t)ino, (uintmax_t)nlink_inode, (uintmax_t)nlink_dir);
    }
    if (nlink_inode == 0) {
      this->AddMsg("inode(%"PRIuMAX") should be deleted since nlink=%"PRIuMAX, (uintmax_t)ino, (uintmax_t)nlink_inode);
    }
  }
  return res;
}

bool FsVerifier::VerifySegUsage(void) {
  bool res = true;
  lfs::physical::Checkpoint* p = this->log()->checkpoint()->segusage()->p();
  for (SegNum seg = this->log()->superblock()->first_logseg(); seg < (SegNum)p->segusage_size(); ++seg) {
    BlkNum nblk_live = p->segusage(seg).nblk_live();
    if (nblk_live != this->live_blocks_[seg]) {
      this->AddMsg("segusage(%"PRIu32").nblk_live=%"PRIu32" mismatches %"PRIu32" counted from files", seg, nblk_live, this->live_blocks_[seg]);
      res = false;
    }
  }
  return res;
}


};//namespace lfs
