#include "fuse/fileops.h"
#include <unistd.h>
namespace lfs {

FuseFileOps::FuseFileOps(Filesystem* fs) {
  assert(fs != NULL);
  this->set_fs(fs);
}

int FuseFileOps::FuseFileOps::getattr(const std::string path, struct stat* stbuf) {
  ino_t ino = this->fs()->pa()->Resolve(path);
  if (ino == 0) return -ENOENT;
  
  memset(stbuf, 0, sizeof(struct stat));
  Inode* inode = this->fs()->GetInode(ino);
  if (inode == NULL) return -EIO;
  stbuf->st_ino = this->fs()->istore()->TransformIno(inode->ino());
  stbuf->st_mode = inode->mode();
  stbuf->st_nlink = inode->nlink();
  stbuf->st_uid = getuid();
  stbuf->st_gid = getgid();
  stbuf->st_size = inode->size();
  stbuf->st_blksize = this->fs()->log()->superblock()->noctet_blk();
  stbuf->st_blocks = inode->blocks();
  stbuf->st_atime = DateTime_time(inode->atime());
  stbuf->st_mtime = DateTime_time(inode->mtime());
  stbuf->st_ctime = DateTime_time(inode->ctime());
  this->fs()->ReleaseInode(inode);
  return 0;
}

int FuseFileOps::utimens(const std::string path, const struct timespec ts[2]) {
  ino_t ino = this->fs()->pa()->Resolve(path);
  if (ino == 0) return -ENOENT;
  
  Inode* inode = this->fs()->GetInode(ino);
  inode->set_atime(DateTime_make(ts[0]));
  inode->set_mtime(DateTime_make(ts[1]));
  this->fs()->ReleaseInode(inode);
  return 0;
}

int FuseFileOps::chmod(const std::string path, mode_t mode) {
  ino_t ino = this->fs()->pa()->Resolve(path);
  if (ino == 0) return -ENOENT;
  
  Inode* inode = this->fs()->GetInode(ino);
  mode_t new_mode = (inode->mode() & S_IFMT) | (mode & ~S_IFMT);
  inode->set_mode(new_mode);
  this->fs()->ReleaseInode(inode);
  return 0;
}

int FuseFileOps::truncate(const std::string path, off_t size) {
  ino_t ino = this->fs()->pa()->Resolve(path);
  if (ino == 0) return -ENOENT;
  File* file = this->fs()->GetFile(ino);
  file->Truncate(size);
  this->fs()->ReleaseFile(file);
  return 0;
}

int FuseFileOps::link(const std::string from, const std::string to) {
  if (0 != this->fs()->pa()->Resolve(to)) return -EEXIST;
  ino_t ino = this->fs()->pa()->Resolve(from);
  if (0 == ino) return -ENOENT;
  std::string parent_name = PathAccess::GetDirName(to);
  ino_t parent_ino = this->fs()->pa()->Resolve(parent_name);
  if (parent_ino == 0) return -ENOENT;
  Directory* parent_dir = this->fs()->GetDirectory(parent_ino);
  if (parent_dir == NULL) return -ENOTDIR;
  Inode* inode = this->fs()->GetInode(ino);
  if (inode == NULL) {
    this->fs()->ReleaseDirectory(parent_dir);
    return -ENOENT;
  }

  this->fs()->log()->TransactionStart();
  int res = 0;

  if (!inode->inc_nlink()) res = -EMLINK;
  
  std::string name = this->fs()->pa()->GetBaseName(to);
  if (res == 0 && !parent_dir->Add(ino, name)) res = -EIO;
  
  this->fs()->ReleaseInode(inode);
  this->fs()->ReleaseDirectory(parent_dir);
  this->fs()->log()->TransactionFinish();
  return res;
}

int FuseFileOps::unlink(const std::string path) {
  ino_t ino = this->fs()->pa()->Resolve(path);
  if (ino == 0) return -ENOENT;
  std::string parent_name = PathAccess::GetDirName(path);
  ino_t parent_ino = this->fs()->pa()->Resolve(parent_name);
  if (parent_ino == 0 || ino == parent_ino) return -EACCES;
  
  Directory* parent_dir = this->fs()->GetDirectory(parent_ino);
  if (parent_dir == NULL) return -EACCES;
  
  Inode* inode = this->fs()->GetInode(ino);
  this->fs()->log()->TransactionStart();
  int res = 0;
  
  if (S_ISDIR(inode->mode())) res = -EISDIR;
  
  std::string name = this->fs()->pa()->GetBaseName(path);
  if (res == 0 && !parent_dir->Remove(name)) res = -EIO;
  if (res == 0) inode->dec_nlink();
  
  this->fs()->ReleaseInode(inode);
  this->fs()->ReleaseDirectory(parent_dir);
  this->fs()->log()->TransactionFinish();
  return res;
}

int FuseFileOps::rename(const std::string from, const std::string to) {
  this->unlink(to);
  int res = this->link(from, to);
  if (res == 0) res = this->unlink(from);
  return res;
}

int FuseFileOps::symlink(const std::string to, const std::string from) {
  if (0 != this->fs()->pa()->Resolve(from)) return -EEXIST;
  std::string parent_name = PathAccess::GetDirName(from);
  ino_t parent_ino = this->fs()->pa()->Resolve(parent_name);
  if (parent_ino == 0) return -ENOENT;
  Directory* parent_dir = this->fs()->GetDirectory(parent_ino);
  if (parent_dir == NULL) return -ENOTDIR;

  this->fs()->log()->TransactionStart();
  const mode_t mode = S_IFLNK | S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH;
  int res = 0;

  Inode inode;
  if (!this->fs()->istore()->AllocateIno(mode, from, &inode)) res = -EACCES;
  inode.set_nlink(1); inode.SetTimes(DateTime_now());
  ino_t ino = inode.ino();
  
  File file(&inode, this->fs()->log());
  //write NULL terminated string
  if (!file.Write(0, to.size() + 1, (uint8_t*)to.c_str())) res = -EIO;
  
  if (res == 0 && !this->fs()->istore()->Write(&inode)) res = -EIO;
  
  std::string name = this->fs()->pa()->GetBaseName(from);
  if (res == 0 && !parent_dir->Add(ino, name)) res = -EIO;
  
  this->fs()->ReleaseDirectory(parent_dir);
  this->fs()->log()->TransactionFinish();
  return res;
}

int FuseFileOps::readlink(const std::string path, char* buf, size_t size) {
  ino_t ino = this->fs()->pa()->Resolve(path);
  if (ino == 0) return -ENOENT;
  File* file = this->fs()->GetFile(ino);
  memset(buf, 0, size);
  if (size > file->inode()->size()) size = file->inode()->size();

  int res = 0;
  
  if (!file->Read(0, size, (uint8_t*)buf)) res = -EIO;
  
  this->fs()->ReleaseFile(file);
  return res;
}

};//namespace lfs

