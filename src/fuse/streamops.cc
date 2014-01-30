#include "fuse/streamops.h"
namespace lfs {

FuseStreamOps::FuseStreamOps(Filesystem* fs) {
  assert(fs != NULL);
  this->set_fs(fs);
}

int FuseStreamOps::create(const std::string path, mode_t mode, struct fuse_file_info* fi) {
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

  mode = S_IFREG | (mode & ~S_IFMT);
  Inode inode;
  if (!this->fs()->istore()->AllocateIno(mode, path, &inode)) res = -ENOENT;
  inode.set_nlink(1); inode.SetTimes(DateTime_now());
  ino_t ino = inode.ino();
  
  if (res == 0 && !this->fs()->istore()->Write(&inode)) res = -EIO;
  
  if (res == 0 && !parent_dir->Add(ino, name)) res = -EIO;
  
  this->fs()->ReleaseDirectory(parent_dir);
  this->fs()->log()->TransactionFinish();

  if (res != 0) return res;
  return this->OpenInternal(ino, fi);
}

int FuseStreamOps::open(const std::string path, struct fuse_file_info* fi) {
  ino_t ino = this->fs()->pa()->Resolve(path);
  if (ino == 0) return -ENOENT;

  return this->OpenInternal(ino, fi);
}

int FuseStreamOps::OpenInternal(ino_t ino, struct fuse_file_info* fi) {
  File* file = this->fs()->GetFile(ino);
  if (!S_ISREG(file->inode()->mode())) {
    this->fs()->ReleaseFile(file);
    return -EISDIR;
  }
  fi->fh = (uint64_t)file;
  return 0;
}

int FuseStreamOps::read(char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
  File* file = (File*)fi->fh;
  if (file == NULL) return -EBADF;
  
  size_t read_size = size;
  if ((size_t)offset > file->inode()->size()) read_size = 0;
  else if ((size_t)offset + size > file->inode()->size()) read_size = file->inode()->size() - offset;

  if (!file->Read(offset, read_size, (uint8_t*)buf)) return -EIO;
  return read_size;
}

int FuseStreamOps::write(const char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
  File* file = (File*)fi->fh;
  if (file == NULL) return -EBADF;
  if (this->fs()->disk_almost_full()) return -ENOSPC;
  
  if (!file->Write(offset, size, (const uint8_t*)buf)) return -EIO;
  return size;
}

int FuseStreamOps::release(struct fuse_file_info* fi) {
  File* file = (File*)fi->fh;
  if (file == NULL) return -EBADF;
  
  this->fs()->ReleaseFile(file);
  return 0;
}

};//namespace lfs

