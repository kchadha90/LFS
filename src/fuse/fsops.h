#ifndef LFS_FUSE_FSOPS_H_
#define LFS_FUSE_FSOPS_H_
#include "util/defs.h"
#include <string>
#include "fuse/fuse.h"
#include "fs/fs.h"
namespace lfs {

class FuseFsOps {
  public:
    FuseFsOps(Filesystem* fs);

    int statfs(struct statvfs* stbuf);
    void destroy(void);
    
  private:
    Filesystem* fs_;
    Filesystem* fs(void) const { return this->fs_; }
    void set_fs(Filesystem* value) { this->fs_ = value; }
    DISALLOW_COPY_AND_ASSIGN(FuseFsOps);
};


};//namespace lfs
#endif//LFS_FUSE_FSOPS_H_

