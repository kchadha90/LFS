#include <cstring>

namespace lfs {
int lfs_main(int argc, char** argv);
int mklfs_main(int argc, char** argv);
int lfsck_main(int argc, char** argv);
};//namespace lfs

int main(int argc, char** argv) {
  char* progname = ::basename(argv[0]);
  if (0 == strcmp(progname, "lfs")) {
    return lfs::lfs_main(argc, argv);
  }
  if (0 == strcmp(progname, "mklfs")) {
    return lfs::mklfs_main(argc, argv);
  }
  if (0 == strcmp(progname, "lfsck")) {
    return lfs::lfsck_main(argc, argv);
  }
  return 254;
}

