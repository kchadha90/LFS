#include "fuse/wrap.h"

lfs::FuseDispatch* lfsfuse_dispatch_;
lfs::FuseDispatch* lfsfuse_dispatch(void) { return lfsfuse_dispatch_; }
void lfsfuse_set_dispatch(lfs::FuseDispatch* value) { lfsfuse_dispatch_ = value; }

//---- fileops ----
int lfsfuse_getattr(const char* path, struct stat* stbuf) {
  return lfsfuse_dispatch()->fileops()->getattr(path, stbuf);
}
int lfsfuse_utimens(const char* path, const struct timespec ts[2]) {
  return lfsfuse_dispatch()->fileops()->utimens(path, ts);
}
int lfsfuse_chmod(const char* path, mode_t mode) {
  return lfsfuse_dispatch()->fileops()->chmod(path, mode);  
}
int lfsfuse_truncate(const char* path, off_t size) {
  return lfsfuse_dispatch()->fileops()->truncate(path, size);
}
int lfsfuse_link(const char* from, const char* to) {
  return lfsfuse_dispatch()->fileops()->link(from, to);
}
int lfsfuse_unlink(const char* path) {
  return lfsfuse_dispatch()->fileops()->unlink(path);
}
int lfsfuse_rename(const char* from, const char* to) {
  return lfsfuse_dispatch()->fileops()->rename(from, to);
}
int lfsfuse_symlink(const char* to, const char* from) {
  return lfsfuse_dispatch()->fileops()->symlink(to, from);
}
int lfsfuse_readlink(const char* path, char* buf, size_t size) {
  return lfsfuse_dispatch()->fileops()->readlink(path, buf, size);
}

//---- streamops ----
int lfsfuse_create(const char* path, mode_t mode, struct fuse_file_info* fi) {
  return lfsfuse_dispatch()->streamops()->create(path, mode, fi);
}
int lfsfuse_open(const char* path, struct fuse_file_info* fi) {
  return lfsfuse_dispatch()->streamops()->open(path, fi);
}
int lfsfuse_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
  return lfsfuse_dispatch()->streamops()->read(buf, size, offset, fi);
}
int lfsfuse_write(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info* fi) {
  return lfsfuse_dispatch()->streamops()->write(buf, size, offset, fi);
}
int lfsfuse_release(const char* path, struct fuse_file_info* fi) {
  return lfsfuse_dispatch()->streamops()->release(fi);
}

//---- dirops ----
int lfsfuse_opendir(const char* path, struct fuse_file_info* fi) {
  return lfsfuse_dispatch()->dirops()->opendir(path, fi);
}
int lfsfuse_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi) {
  return lfsfuse_dispatch()->dirops()->readdir(buf, filler, offset, fi);
}
int lfsfuse_releasedir(const char* path, struct fuse_file_info *fi) {
  return lfsfuse_dispatch()->dirops()->releasedir(fi);
}
int lfsfuse_mkdir(const char* path, mode_t mode) {
  return lfsfuse_dispatch()->dirops()->mkdir(path, mode);
}
int lfsfuse_rmdir(const char* path) {
  return lfsfuse_dispatch()->dirops()->rmdir(path);
}

//---- fsops ----
int lfsfuse_statfs(const char* path, struct statvfs* stbuf) {
  return lfsfuse_dispatch()->fsops()->statfs(stbuf);
}
void lfsfuse_destroy(void* private_data) {
  return lfsfuse_dispatch()->fsops()->destroy();
}


