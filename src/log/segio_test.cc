#include "log/segio.h"
#include <cstring>
#include "disk/memdisk.h"
#include "log/superblock.h"
#include "exttool/gtest.h"
namespace lfs {

class LogTestSegIOSuperblock : public Superblock {
  public:
    LogTestSegIOSuperblock(size_t noctet_sector, DiskSectorNum nsector_blk, BlkNum nblk_seg, SegNum nseg_disk) {
      this->set_noctet_sector(noctet_sector);
      this->set_nsector_blk(nsector_blk);
      this->set_nblk_seg(nblk_seg);
      this->set_nseg_disk(nseg_disk);
    }
};

TEST(LogTest, SegIO) {

  const uint16_t sector_size = 4;
  const DiskSectorNum segment_size = 4;
  const size_t segment_count = 4;
  const size_t cache_capacity = 2;
  const SegNum seg1 = 1;
  uint8_t buf1[segment_size * sector_size];
  uint8_t buf2[segment_size * sector_size];
  charbuf buf3;
  
  MemDisk md;
  md.Init(sector_size, segment_size * segment_count);
  LogTestSegIOSuperblock superblock(sector_size, 1, segment_size, segment_count);
  SegIO segio(&md, &superblock, cache_capacity);
  ASSERT_EQ(sector_size * segment_size, superblock.noctet_seg());
  
  std::memset(buf1, 0x50, superblock.noctet_seg());
  ASSERT_FALSE(segio.Write(0, buf1));
  segio.set_protect_seg0(false);
  ASSERT_TRUE(segio.Write(0, buf1));
  segio.set_protect_seg0(true);

  std::memset(buf1, 0x50, superblock.noctet_seg());
  //ASSERT_FALSE(segio.Write(seg1, buf1));
  
  //segio.set_protect_seg0(false);

  ASSERT_TRUE(segio.Write(seg1, buf1));
  md.Seek(seg1 * segment_size);
  md.Read(segment_size, buf2);
  EXPECT_EQ(0, std::memcmp(buf1, buf2, superblock.noctet_seg()));
  
  ASSERT_TRUE(segio.Read(seg1, buf2));
  EXPECT_EQ(0, std::memcmp(buf1, buf2, superblock.noctet_seg()));
  
  ASSERT_TRUE(segio.Read(seg1, buf2));
  EXPECT_EQ(0, std::memcmp(buf1, buf2, superblock.noctet_seg()));
  
  EXPECT_FALSE(segio.Read(seg1, sector_size * segment_size, 1, buf2));
  EXPECT_FALSE(segio.Read(seg1, 0, sector_size * segment_size + 1, buf2));
  buf3.clear();
  EXPECT_TRUE(segio.Read(seg1, 0, 1, &buf3));
  EXPECT_EQ((size_t)1, buf3.size());

  std::memset(buf2, 0xEE, superblock.noctet_seg());
  md.Seek(seg1 * segment_size);
  md.Write(segment_size, buf2);
  ASSERT_TRUE(segio.Read(seg1, buf2));//seg1 is cached, the change directly on disk is not reflected
  EXPECT_EQ(0, std::memcmp(buf1, buf2, superblock.noctet_seg()));
}

};//namespace lfs
