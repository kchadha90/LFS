#ifndef LFS_FUSE_STREAMOPS_H_
#define LFS_FUSE_STREAMOPS_H_
#include "util/defs.h"
#include <string>
#include "fuse/fuse.h"
#include "fs/fs.h"
namespace lfs {

class FuseStreamOps {
  public:
    FuseStreamOps(Filesystem* fs);

    int create(const std::string path, mode_t mode, struct fuse_file_info* fi);
    int open(const std::string path, struct fuse_file_info* fi);
    int read(char* buf, size_t size, off_t offset, struct fuse_file_info* fi);
    int write(const char* buf, size_t size, off_t offset, struct fuse_file_info* fi);
    int release(struct fuse_file_info* fi);

  private:
    Filesystem* fs_;
    Filesystem* fs(void) const { return this->fs_; }
    void set_fs(Filesystem* value) { this->fs_ = value; }
    
    int OpenInternal(ino_t ino, struct fuse_file_info* fi);
    
    DISALLOW_COPY_AND_ASSIGN(FuseStreamOps);
};

};//namespace lfs
#endif//LFS_FUSE_STREAMOPS_H_

