#include "disk/vdisk.h"
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "exttool/gtest.h"
namespace lfs {

TEST(DiskTest, VDisk) {

  const uint16_t noctet_sector = DISK_SECTOR_SIZE;
  const DiskSectorNum nsector = 16;
  DiskSectorNum sector1 = 7;
  uint8_t buf11[noctet_sector * 2];
  uint8_t buf12[noctet_sector * 2];
  DiskSectorNum sector2 = 15;
  uint8_t buf21[noctet_sector];
  uint8_t buf22[noctet_sector];
  
  char vdname[L_tmpnam] = P_tmpdir "/vdiskXXXXXX";
  int fd = ::mkstemp(vdname);
  ASSERT_NE(-1, fd);
  ASSERT_EQ(0, ::ftruncate(fd, noctet_sector * nsector));

  VDisk vd;
  ASSERT_TRUE(vd.Open(vdname));
  ASSERT_EQ(nsector, vd.nsector());
  ASSERT_EQ(noctet_sector, vd.noctet_sector());

  ASSERT_FALSE(vd.Seek(nsector + 0));
  ASSERT_FALSE(vd.Seek(nsector + 1));
  ASSERT_TRUE(vd.Seek(nsector - 1));

  ASSERT_TRUE(vd.Seek(sector1));
  std::memset(buf11, 0x50, sizeof(buf11));
  ASSERT_TRUE(vd.Write(2, buf11));

  ASSERT_TRUE(vd.Seek(sector2));
  std::memset(buf21, 0x35, sizeof(buf21));
  ASSERT_TRUE(vd.Write(1, buf21));

  ASSERT_TRUE(vd.Seek(sector1));
  ASSERT_TRUE(vd.Read(1, buf12));
  ASSERT_TRUE(vd.Seek(sector1 + 1));
  ASSERT_TRUE(vd.Read(1, buf12 + noctet_sector));
  ASSERT_EQ(0, std::memcmp(buf11, buf12, sizeof(buf11)));

  ASSERT_TRUE(vd.Seek(sector2));
  ASSERT_TRUE(vd.Read(1, buf22));
  ASSERT_EQ(0, std::memcmp(buf21, buf22, sizeof(buf21)));
  
  ASSERT_TRUE(vd.Close());
  ASSERT_EQ(0, ::close(fd));
  ASSERT_EQ(0, ::unlink(vdname));
}

};//namespace lfs
