#ifndef LFS_FUSE_START_H_
#define LFS_FUSE_START_H_

#ifdef __cplusplus
extern "C" {
#else
typedef int bool;
#endif

int lfsfuse_run(char* mountpoint, bool debuglog);

#ifdef __cplusplus
}
#endif

#endif//LFS_FUSE_START_H_

