#include "disk/memdisk.h"
#include <cstring>
#include "exttool/gtest.h"
namespace lfs {

TEST(DiskTest, MemDisk) {

  const uint16_t sector_size = 512;
  const DiskSectorNum sector_count = 16;
  DiskSectorNum sector1 = 7;
  uint8_t buf11[sector_size * 2];
  uint8_t buf12[sector_size * 2];
  DiskSectorNum sector2 = 15;
  uint8_t buf21[sector_size];
  uint8_t buf22[sector_size];
  
  MemDisk md;
  ASSERT_TRUE(md.Init(sector_size, sector_count));

  ASSERT_FALSE(md.Seek(sector_count + 0));
  ASSERT_FALSE(md.Seek(sector_count + 1));
  ASSERT_TRUE(md.Seek(sector_count - 1));

  ASSERT_TRUE(md.Seek(sector1));
  std::memset(buf11, 0x50, sizeof(buf11));
  ASSERT_TRUE(md.Write(2, buf11));

  ASSERT_TRUE(md.Seek(sector2));
  std::memset(buf21, 0x35, sizeof(buf21));
  ASSERT_TRUE(md.Write(1, buf21));

  ASSERT_TRUE(md.Seek(sector1));
  ASSERT_TRUE(md.Read(1, buf12));
  ASSERT_TRUE(md.Seek(sector1 + 1));
  ASSERT_TRUE(md.Read(1, buf12 + sector_size));
  ASSERT_EQ(0, std::memcmp(buf11, buf12, sizeof(buf11)));

  ASSERT_TRUE(md.Seek(sector2));
  ASSERT_TRUE(md.Read(1, buf22));
  ASSERT_EQ(0, std::memcmp(buf21, buf22, sizeof(buf21)));
}

};//namespace lfs
