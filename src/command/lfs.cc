#include <getopt.h>
#include <cstdlib>
#include "disk/vdisk.h"
#include "fs/fs.h"
#include "cleaner/cleaner.h"
#include "fuse/start.h"
#include "fuse/wrap.h"
namespace lfs {

struct lfs_cmdopts {
  SegNum nseg_cache;
  SegNum checkpoint_interval;
  SegNum cleaning_start;
  SegNum cleaning_stop;
  char* filename;
  char* mountpoint;
};

bool lfs_getopt(int argc, char** argv, struct lfs_cmdopts* opts) {
  opts->nseg_cache = SegNum_invalid;
  while (1) {
    int option_index = 0;
    static struct option long_options[] = {
      {"cache", required_argument, 0, 0},
      {"interval", required_argument, 0, 0},
      {"start", required_argument, 0, 0},
      {"stop", required_argument, 0, 0}
    };
    int c = ::getopt_long(argc, argv, "s:i:c:C:", long_options, &option_index);
    if (c == -1) break;
    switch (c) {
      case 0: {
        switch (option_index) {
          case 0: opts->nseg_cache = ::atol(optarg); break;
          case 1: opts->checkpoint_interval = ::atol(optarg); break;
          case 2: opts->cleaning_start = ::atol(optarg); break;
          case 3: opts->cleaning_stop = ::atol(optarg); break;
        }
      } break;
      case 's': opts->nseg_cache = ::atol(optarg); break;
      case 'i': opts->checkpoint_interval = ::atol(optarg); break;
      case 'c': opts->cleaning_start = ::atol(optarg); break;
      case 'C': opts->cleaning_stop = ::atol(optarg); break;
    }
  }
  if (optind + 1 < argc) {
    opts->filename = argv[optind];
    opts->mountpoint = argv[optind + 1];
  }
  if (opts->nseg_cache == SegNum_invalid) opts->nseg_cache = 16;
  if (opts->checkpoint_interval == 0) opts->checkpoint_interval = 4;
  if (opts->cleaning_start == 0) opts->cleaning_start = 4;
  if (opts->cleaning_stop == 0) opts->cleaning_stop = 8;
  return opts->filename != NULL && opts->mountpoint != NULL;
}

int lfs_main(int argc, char** argv) {
  struct lfs_cmdopts opts = {0};
  if (!lfs_getopt(argc, argv, &opts)) {
    ::printf("lfs, a Log-structured File System on FUSE.\n"
             "Usage: lfs [OPTION]... filename mountpoint\n"
             "\n"
             "  -s, --cache=nseg_cache\n"
             "  -i, --interval=checkpoint_interval\n"
             "  -c, --start=cleaning_start\n"
             "  -C, --stop=cleaning_stop\n");
    return 1;
  }

  VDisk disk;
  disk.Open(opts.filename);
  Log log;
  log.set_segcache_capacity(opts.nseg_cache);
  log.set_checkpoint_interval(opts.checkpoint_interval);
  log.set_cleaning_bounds(opts.cleaning_start, opts.cleaning_stop);
  log.Open(&disk);
  Filesystem fs;
  fs.Open(&log);
  Cleaner cleaner(&fs);
  
  FuseDispatch dispatch;
  dispatch.set_fileops(new FuseFileOps(&fs));
  dispatch.set_streamops(new FuseStreamOps(&fs));
  dispatch.set_dirops(new FuseDirOps(&fs));
  dispatch.set_fsops(new FuseFsOps(&fs));
  lfsfuse_set_dispatch(&dispatch);

  int res = lfsfuse_run(opts.mountpoint, false);
  
  fs.Close();
  log.Close();
  disk.Close();
  
  return res;
}

};//namespace lfs
