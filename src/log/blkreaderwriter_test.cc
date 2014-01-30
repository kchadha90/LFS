#include "log/blkreader.h"
#include "log/blkwriter.h"
#include "cleaner/seqsegalloc.h"
#include "log/logcreator.h"
#include "disk/memdisk.h"
#include "exttool/gtest.h"
namespace lfs {

static BlkInfo* make_blkinfo(BlkOffset off) {
  BlkInfo* bi = new BlkInfo();
  bi->set_type(kBTData);
  bi->set_ino(1);
  bi->set_ver(0);
  bi->set_off(off);
  bi->set_mtime(DateTime_now());
  return bi;
}

TEST(LogTest, BlkReaderWriter) {
  LogCreator lc;
  lc.set_noctet_sector(512);
  lc.set_nsector_blk(1);
  lc.set_nblk_seg(32);
  lc.set_nseg_disk(20);
  ASSERT_TRUE(lc.VerifyMetrics());
  MemDisk md;
  ASSERT_TRUE(md.Init(lc.noctet_sector(), lc.noctet_disk() / lc.noctet_sector()));
  ASSERT_TRUE(lc.set_disk(&md));
  ASSERT_TRUE(lc.WriteSuperblock());
  
  Superblock superblock;
  ASSERT_TRUE(superblock.Read(&md));
  ASSERT_EQ((BlkNum)2, superblock.nblk_segsummary());
  SegIO segio(&md, &superblock, 2);
  SequentialSegAllocator ssa(&superblock, 5);
  
  BlkReader br(&segio);
  BlkWriter bw(&segio, &ssa, 4);
  br.set_bw(&bw);
  EXPECT_EQ((SegNum)4, bw.cur_seg());
  
  uint8_t buf1[512];
  uint8_t buf2[512];
  SegBlkNum blks[70];
  
  memset(buf1, 0x00, 512);
  memset(buf2, 0xFF, 512);
  EXPECT_TRUE(br.Read(SegBlkNum_unassigned, buf2));
  EXPECT_EQ(0, memcmp(buf1, buf2, 512));
  memset(buf2, 0xFF, 512);
  EXPECT_TRUE(br.Read(SegBlkNum_unassigned, 3, 17, buf2));
  EXPECT_EQ(0, memcmp(buf1, buf2, 17));

  for (int i = 1; i <= 30; ++i) {
    buf1[0] = (uint8_t)i;
    blks[i] = bw.Put(make_blkinfo(i), buf1);
    EXPECT_NE(SegBlkNum_invalid, blks[i]);
    EXPECT_EQ((SegNum)4, bw.cur_seg());
  }
  for (int i = 1; i <= 30; ++i) {
    EXPECT_TRUE(br.Read(blks[i], 0, 1, buf2));
    EXPECT_EQ((uint8_t)i, buf2[0]);
  }

  buf1[0] = 31;
  blks[31] = bw.Put(make_blkinfo(31), buf1);
  EXPECT_NE(SegBlkNum_invalid, blks[31]);
  EXPECT_EQ((SegNum)5, bw.cur_seg());
  for (int i = 1; i <= 31; ++i) {
    EXPECT_TRUE(br.Read(blks[i], 0, 1, buf2));
    EXPECT_EQ((uint8_t)i, buf2[0]);
  }
  
  for (int i = 32; i <= 60; ++i) {
    buf1[0] = (uint8_t)i;
    blks[i] = bw.Put(make_blkinfo(i), buf1);
    EXPECT_NE(SegBlkNum_invalid, blks[i]);
    EXPECT_EQ((SegNum)5, bw.cur_seg());
  }
  for (int i = 1; i <= 60; ++i) {
    EXPECT_TRUE(br.Read(blks[i], 0, 1, buf2));
    EXPECT_EQ((uint8_t)i, buf2[0]);
  }

  EXPECT_FALSE(bw.Die(blks[20]));
  EXPECT_TRUE(bw.Die(blks[50]));
  EXPECT_TRUE(bw.Die(blks[45]));
  EXPECT_FALSE(bw.Die(blks[50]));
  for (int i = 61; i <= 65; ++i) {
    buf1[0] = (uint8_t)i;
    blks[i] = bw.Put(make_blkinfo(i), buf1);
    EXPECT_NE(SegBlkNum_invalid, blks[i]);
    EXPECT_EQ((SegNum)(i<=62?5:6), bw.cur_seg());
  }
  BlkInfo bi;
  for (int i = 1; i <= 65; ++i) {
    if (i == 50 || i == 45) continue;
    EXPECT_TRUE(br.Read(blks[i], 0, 1, buf2));
    EXPECT_EQ((uint8_t)i, buf2[0]);
    EXPECT_TRUE(br.GetMeta(blks[i], &bi));
    EXPECT_EQ((BlkOffset)i, bi.off());
  }
}

};//namespace lfs
