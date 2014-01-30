#include "file/sysfile_creator.h"
#include "dir/physical.pb.h"
#include "util/protobuf.h"
namespace lfs {

SystemFileCreator::SystemFileCreator(InodeStore* istore) {
  this->set_istore(istore);
}

bool SystemFileCreator::CreateAll(void) {
  bool res = true;
  this->log()->TransactionStart();
  res = res && this->CreateFile(Inode::kIno_Root);
  res = res && this->CreateFile(Inode::kIno_UnlinkedDir);
  res = res && this->CreateFile(Inode::kIno_InoMap);
  res = res && this->CreateFile(Inode::kIno_InoVec);
  res = res && this->CreateFile(Inode::kIno_InoTbl);
  this->log()->TransactionFinish();
  this->log()->ForceCheckpoint();
  return res;
}

bool SystemFileCreator::WriteInoMap(File* file) {
  assert(Inode::kIno_Good % 8 == 0);
  const size_t buflen = Inode::kIno_Good / 8;
  uint8_t buf[buflen]; memset(buf, 0xFF, buflen);
  return file->Write(0, buflen, buf);
}

bool SystemFileCreator::CreateFile(ino_t ino) {
  bool is_dir = ino == Inode::kIno_Root;
  bool in_checkpoint = ino == Inode::kIno_InoMap || ino == Inode::kIno_InoVec || ino == Inode::kIno_InoTbl;
  
  if (in_checkpoint) {
    if (ino == Inode::kIno_InoMap) return this->istore()->InitializeInoFiles(this);
    return true;
  } else {
    mode_t mode = (is_dir ? S_IFDIR : S_IFREG) | S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH;
    Inode inode; inode.set_ino(ino); inode.set_mode(mode); inode.set_nlink(1);
    inode.SetTimes(DateTime_now()); inode.set_version(1);
    if (is_dir) {
      File file(&inode, this->log());
      if (!this->WriteDirectory(&file)) return false;
    }
    return this->istore()->Write(&inode);
  }
}

bool SystemFileCreator::WriteDirectory(File* file) {
  lfs::physical::DirFile p;
  charbuf cb;
  pb_save(&p, &cb);
  return file->Write(0, cb.size(), cb.data());
}

};//namespace lfs
