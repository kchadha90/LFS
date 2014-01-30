#include "fuse/start.h"
#include "fuse/wrap.h"

int lfsfuse_run(char* mountpoint, bool debuglog) {
  struct fuse_operations ops = {
    .flag_nullpath_ok = 1,
    //fileops
    .getattr = lfsfuse_getattr,
    .utimens = lfsfuse_utimens,
    .chmod = lfsfuse_chmod,
    .truncate = lfsfuse_truncate,
    .link = lfsfuse_link,
    .unlink = lfsfuse_unlink,
    .rename = lfsfuse_rename,
    .symlink = lfsfuse_symlink,
    .readlink = lfsfuse_readlink,
    //streamops
    .create = lfsfuse_create,
    .open = lfsfuse_open,
    .read = lfsfuse_read,
    .write = lfsfuse_write,
    .release = lfsfuse_release,
    //dirops
    .opendir = lfsfuse_opendir,
    .readdir = lfsfuse_readdir,
    .releasedir = lfsfuse_releasedir,
    .mkdir = lfsfuse_mkdir,
    .rmdir = lfsfuse_rmdir,
    //fsops
    .statfs = lfsfuse_statfs,
    .destroy = lfsfuse_destroy,
  };
  
  char* nargv[16]; int nargc = -1;
  nargv[++nargc] = "lfs";
  nargv[++nargc] = "-f";
  nargv[++nargc] = "-s";
  if (debuglog) nargv[++nargc] = "-d";
  nargv[++nargc] = "-ouse_ino";
  nargv[++nargc] = mountpoint;
  nargv[++nargc] = "";

  return fuse_main(nargc, nargv, &ops, NULL);
}

