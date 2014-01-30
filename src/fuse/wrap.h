#ifndef LFS_FUSE_WRAP_H_
#define LFS_FUSE_WRAP_H_
#ifdef __cplusplus
#include "util/defs.h"
#endif

#include "fuse/fuse.h"

#ifdef __cplusplus
#include "fuse/dispatch.h"
lfs::FuseDispatch* lfsfuse_dispatch(void);
void lfsfuse_set_dispatch(lfs::FuseDispatch* value);
#endif

#ifdef __cplusplus
extern "C" {
#endif

//---- fileops ----
int lfsfuse_getattr(const char* path, struct stat* stbuf);
int lfsfuse_utimens(const char* path, const struct timespec ts[2]);
int lfsfuse_chmod(const char* path, mode_t mode);
int lfsfuse_truncate(const char* path, off_t size);
int lfsfuse_link(const char* from, const char* to);
int lfsfuse_unlink(const char* path);
int lfsfuse_rename(const char* from, const char* to);
int lfsfuse_symlink(const char* to, const char* from);
int lfsfuse_readlink(const char* path, char* buf, size_t size);

//---- streamops ----
int lfsfuse_create(const char* path, mode_t mode, struct fuse_file_info* fi);
int lfsfuse_open(const char* path, struct fuse_file_info* fi);
int lfsfuse_read(const char* path, char* buf, size_t size, off_t offset, struct fuse_file_info* fi);
int lfsfuse_write(const char* path, const char* buf, size_t size, off_t offset, struct fuse_file_info* fi);
int lfsfuse_release(const char* path, struct fuse_file_info* fi);

//---- dirops ----
int lfsfuse_opendir(const char* path, struct fuse_file_info* fi);
int lfsfuse_readdir(const char* path, void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi);
int lfsfuse_releasedir(const char* path, struct fuse_file_info *fi);
int lfsfuse_mkdir(const char* path, mode_t mode);
int lfsfuse_rmdir(const char* path);

//---- fsops ----
int lfsfuse_statfs(const char* path, struct statvfs* stbuf);
void lfsfuse_destroy(void* private_data);


#ifdef __cplusplus
}
#endif

#endif//LFS_FUSE_WRAP_H_

