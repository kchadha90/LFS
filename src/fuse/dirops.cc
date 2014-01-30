#include "fuse/dirops.h"
#include "dir/physical.pb.h"
#include "util/protobuf.h"
namespace lfs {

FuseDirOps::FuseDirOps(Filesystem* fs) {
  assert(fs != NULL);
  this->set_fs(fs);
}

int FuseDirOps::opendir(const std::string path, struct fuse_file_info* fi) {
  ino_t ino = this->fs()->pa()->Resolve(path);
  if (ino == 0) return -ENOENT;
  Directory* dir = this->fs()->GetDirectory(ino);
  if (dir == NULL) return -ENOTDIR;
  fi->fh = (uint64_t)dir;
  return 0;
}

int FuseDirOps::readdir(void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi) {
  Directory* dir = (Directory*)fi->fh;
  if (dir == NULL) return -EBADF;
  
  std::vector<std::string> names;
  if (!dir->List(&names)) return -EBADF;
  
  if (0 != filler(buf, ".", NULL, 0)) return 0;
  if (0 != filler(buf, "..", NULL, 0)) return 0;
  for (std::vector<std::string>::iterator it = names.begin(); it < names.end(); ++it) {
    if (0 != filler(buf, it->c_str(), NULL, 0)) return 0;
  }
  return 0;
}

int FuseDirOps::releasedir(struct fuse_file_info *fi) {
  Directory* dir = (Directory*)fi->fh;
  if (dir == NULL) return -EBADF;
  
  this->fs()->ReleaseDirectory(dir);
  return 0;
}

int FuseDirOps::mkdir(const std::string path, mode_t mode) {
  std::string parent_name = PathAccess::GetDirName(path);
  ino_t parent_ino = this->fs()->pa()->Resolve(parent_name);
  if (parent_ino == 0) return -ENOENT;
  Directory* parent_dir = this->fs()->GetDirectory(parent_ino);
  if (parent_dir == NULL) return -ENOTDIR;
  std::string name = this->fs()->pa()->GetBaseName(path);
  if (0 != parent_dir->Get(name)) return -EEXIST;
  if (this->fs()->disk_almost_full()) return -ENOSPC;

  this->fs()->log()->TransactionStart();
  int res = 0;

  mode = S_IFDIR | (mode & ~S_IFMT);
  Inode inode;
  if (!this->fs()->istore()->AllocateIno(mode, path, &inode)) res = -ENOENT;
  inode.set_nlink(1); inode.SetTimes(DateTime_now());
  ino_t ino = inode.ino();
  
  File file(&inode, this->fs()->log());
  lfs::physical::DirFile p;
  charbuf cb;
  pb_save(&p, &cb);
  if (!file.Write(0, cb.size(), cb.data())) res = -EIO;
  
  if (res == 0 && !this->fs()->istore()->Write(&inode)) res = -EIO;
  if (res == 0 && !parent_dir->Add(ino, name)) res = -EIO;
  
  this->fs()->ReleaseDirectory(parent_dir);
  this->fs()->log()->TransactionFinish();

  return res;
}

int FuseDirOps::rmdir(const std::string path) {
  ino_t ino = this->fs()->pa()->Resolve(path);
  if (ino == 0) return -ENOENT;
  std::string parent_name = PathAccess::GetDirName(path);
  ino_t parent_ino = this->fs()->pa()->Resolve(parent_name);
  if (parent_ino == 0 || ino == parent_ino) return -EACCES;
  
  Directory* dir = this->fs()->GetDirectory(ino);
  if (dir == NULL) return -ENOTDIR;

  Directory* parent_dir = this->fs()->GetDirectory(parent_ino);
  if (parent_dir == NULL) return -EACCES;
  
  Inode* inode = this->fs()->GetInode(ino);
  this->fs()->log()->TransactionStart();
  int res = 0;
  
  if (!dir->IsEmpty()) res = -ENOTEMPTY;
  this->fs()->ReleaseDirectory(dir);
  
  std::string name = this->fs()->pa()->GetBaseName(path);
  if (res == 0 && !parent_dir->Remove(name)) res = -EIO;
  if (res == 0) inode->dec_nlink();
  
  this->fs()->ReleaseInode(inode);
  this->fs()->ReleaseDirectory(parent_dir);
  this->fs()->log()->TransactionFinish();
  return res;
}


};//namespace lfs

