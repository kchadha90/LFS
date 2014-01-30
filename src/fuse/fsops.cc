#include "fuse/fsops.h"
namespace lfs {

FuseFsOps::FuseFsOps(Filesystem* fs) {
  assert(fs != NULL);
  this->set_fs(fs);
}

int FuseFsOps::statfs(struct statvfs* stbuf) {
  memset(stbuf, 0, sizeof(struct statvfs));
  Superblock* superblock = this->fs()->log()->superblock();
  stbuf->f_bsize = superblock->noctet_blk();
  stbuf->f_frsize = superblock->noctet_blk();
  stbuf->f_blocks = superblock->nblk_seg() * superblock->nseg_disk();
  //TODO bfree,bavail read from segusage
  //TODO files,ffree,favail read from inomap
  return 0;
}

void FuseFsOps::destroy(void) {
  this->fs()->log()->ForceCheckpoint();
}

};//namespace lfs

