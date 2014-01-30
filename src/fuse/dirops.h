#ifndef LFS_FUSE_DIROPS_H_
#define LFS_FUSE_DIROPS_H_
#include "util/defs.h"
#include <string>
#include "fuse/fuse.h"
#include "fs/fs.h"
namespace lfs {

class FuseDirOps {
  public:
    FuseDirOps(Filesystem* fs);

    int opendir(const std::string path, struct fuse_file_info* fi);
    int readdir(void* buf, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info* fi);
    int releasedir(struct fuse_file_info *fi);
    
    int mkdir(const std::string path, mode_t mode);
    int rmdir(const std::string path);
    
  private:
    Filesystem* fs_;
    Filesystem* fs(void) const { return this->fs_; }
    void set_fs(Filesystem* value) { this->fs_ = value; }
    DISALLOW_COPY_AND_ASSIGN(FuseDirOps);
};


};//namespace lfs
#endif//LFS_FUSE_DIROPS_H_

