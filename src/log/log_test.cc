#include "log/log.h"
#include <unistd.h>
#include "disk/memdisk.h"
#include "log/logcreator.h"
#include "exttool/gtest.h"
namespace lfs {

static BlkInfo* make_blkinfo(void) {
  BlkInfo* bi = new BlkInfo();
  bi->set_type(kBTData);
  bi->set_ino(1);
  bi->set_ver(0);
  bi->set_off(0);
  bi->set_mtime(DateTime_now());
  return bi;
}

TEST(LogTest, Log) {
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
  ASSERT_TRUE(lc.WriteCheckpoints());
  Log log;
  log.set_checkpoint_interval(3);
  ASSERT_TRUE(log.Open(&md));
  uint8_t buf[512];

  DateTime t1 = log.checkpoint()->timestamp();
  ::usleep(100);
  for (int i = 1; i <= 90; ++i) {
    EXPECT_NE(SegBlkNum_invalid, log.bw()->Put(make_blkinfo(), buf));
  }
  EXPECT_EQ(t1, log.checkpoint()->timestamp());
  //trigger checkpoint by reaching interval
  EXPECT_NE(SegBlkNum_invalid, log.bw()->Put(make_blkinfo(), buf));//block 91
  EXPECT_LT(t1, log.checkpoint()->timestamp());
  
  t1 = log.checkpoint()->timestamp();
  ::usleep(100);
  for (int i = 92; i <= 179; ++i) {
    EXPECT_NE(SegBlkNum_invalid, log.bw()->Put(make_blkinfo(), buf));
  }
  log.TransactionStart();
  for (int i = 180; i <= 250; ++i) {
    EXPECT_NE(SegBlkNum_invalid, log.bw()->Put(make_blkinfo(), buf));
  }
  //postpone checkpoint when in transaction
  EXPECT_EQ(t1, log.checkpoint()->timestamp());
  log.TransactionFinish();
  EXPECT_LT(t1, log.checkpoint()->timestamp());
}

};//namespace lfs
