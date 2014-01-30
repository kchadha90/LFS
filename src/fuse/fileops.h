#ifndef LFS_FUSE_FILEOPS_H_
#define LFS_FUSE_FILEOPS_H_
#include "util/defs.h"
#include <string>
#include "fuse/fuse.h"
#include "fs/fs.h"
namespace lfs {

class FuseFileOps {
  public:
    FuseFileOps(Filesystem* fs);
    
    int getattr(const std::string path, struct stat* stbuf);
    int utimens(const std::string path, const struct timespec ts[2]);
    int chmod(const std::string path, mode_t mode);
    
    int truncate(const std::string path, off_t size);
    
    int link(const std::string from, const std::string to);
    int unlink(const std::string path);
    int rename(const std::string from, const std::string to);

    int symlink(const std::string to, const std::string from);
    int readlink(const std::string path, char* buf, size_t size);
    
  private:
    Filesystem* fs_;
    Filesystem* fs(void) const { return this->fs_; }
    void set_fs(Filesystem* value) { this->fs_ = value; }
    DISALLOW_COPY_AND_ASSIGN(FuseFileOps);
};

};//namespace lfs
#endif//LFS_FUSE_FILEOPS_H_

