#ifndef LFS_FUSE_DISPATCH_H_
#define LFS_FUSE_DISPATCH_H_
#include "util/defs.h"
#include "fuse/fuse.h"
#include "fuse/fileops.h"
#include "fuse/streamops.h"
#include "fuse/dirops.h"
#include "fuse/fsops.h"
namespace lfs {

class FuseDispatch {
  public:
    FuseDispatch(void) {}
    FuseFileOps* fileops(void) const { return this->fileops_; }
    void set_fileops(FuseFileOps* value) { this->fileops_ = value; }
    FuseStreamOps* streamops(void) const { return this->streamops_; }
    void set_streamops(FuseStreamOps* value) { this->streamops_ = value; }
    FuseDirOps* dirops(void) const { return this->dirops_; }
    void set_dirops(FuseDirOps* value) { this->dirops_ = value; }
    FuseFsOps* fsops(void) const { return this->fsops_; }
    void set_fsops(FuseFsOps* value) { this->fsops_ = value; }

  private:
    FuseFileOps* fileops_;
    FuseStreamOps* streamops_;
    FuseDirOps* dirops_;
    FuseFsOps* fsops_;
    DISALLOW_COPY_AND_ASSIGN(FuseDispatch);
};

};//namespace lfs
#endif//LFS_FUSE_DISPATCH_H_

