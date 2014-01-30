#include "log/logcreator.h"
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include "disk/vdisk.h"
#include "log/checkpoint.h"
#include "exttool/gtest.h"
namespace lfs {

//this case covers: LogCreator, Superblock, Checkpoint, Checkpoints
TEST(LogTest, LogCreator) {
  LogCreator lc;

  lc.set_noctet_sector(DISK_SECTOR_SIZE);
  lc.set_nsector_blk(4);
  lc.set_nblk_seg(12);
  lc.set_nseg_disk(1000);
  ASSERT_TRUE(lc.VerifyMetrics());
  ASSERT_EQ((size_t)(DISK_SECTOR_SIZE * 4 * 12 * 1000), lc.noctet_disk());

  char vdname[L_tmpnam] = P_tmpdir "/vdiskXXXXXX";
  int fd = ::mkstemp(vdname);
  ::ftruncate(fd, lc.noctet_disk());
  ::close(fd);
  VDisk disk;
  disk.Open(vdname);
  ASSERT_TRUE(lc.set_disk(&disk));
  ASSERT_TRUE(lc.WriteSuperblock());

  Superblock sb;
  ASSERT_TRUE(sb.Read(&disk));
  EXPECT_EQ((size_t)DISK_SECTOR_SIZE, sb.noctet_sector());
  EXPECT_EQ((DiskSectorNum)4, sb.nsector_blk());
  EXPECT_EQ((size_t)(DISK_SECTOR_SIZE * 4), sb.noctet_blk());
  EXPECT_EQ((BlkNum)12, sb.nblk_seg());
  EXPECT_EQ((SegNum)1000, sb.nseg_disk());
  
  EXPECT_LE((SegNum)1, sb.checkpoint_seg(0));
  EXPECT_LE(sb.checkpoint_seg(0) + sb.nseg_checkpoint(), sb.checkpoint_seg(1));
  EXPECT_LE(sb.checkpoint_seg(1) + sb.nseg_checkpoint(), sb.first_logseg());
  
  SegIO segio0(&disk, &sb, 4);
  Checkpoints cps0(&segio0);
  EXPECT_FALSE(cps0.Read());
  //cannot reuse segio0 and cps0 later, because segio0 has invalid checkpoints cached, and lc.WriteCheckpoints uses its own SegIO
  DateTime t1 = DateTime_now();
  ASSERT_TRUE(lc.WriteCheckpoints());
  DateTime t2 = DateTime_now();

  SegIO segio(&disk, &sb, 4);
  Checkpoint cp0(&segio); Checkpoint cp1(&segio);
  EXPECT_TRUE(cp0.Read(0));
  EXPECT_TRUE(cp0.IsValid());
  EXPECT_FALSE(cp1.Read(1));
  EXPECT_FALSE(cp1.IsValid());
  EXPECT_TRUE(cp0.IsPreferred(&cp1));
  EXPECT_FALSE(cp1.IsPreferred(&cp0));

  EXPECT_GE(cp0.timestamp(), t1);
  EXPECT_LE(cp0.timestamp(), t2);
  EXPECT_GE(cp0.next_seg(), sb.first_logseg());
  
  Checkpoints cps(&segio);
  ASSERT_TRUE(cps.Read());
  EXPECT_EQ(cp0.timestamp(), cps.c()->timestamp());
  EXPECT_TRUE(cps.Write());
  EXPECT_TRUE(cp1.Read(1));
  EXPECT_TRUE(cp1.IsValid());
  EXPECT_FALSE(cp0.IsPreferred(&cp1));
  EXPECT_TRUE(cp1.IsPreferred(&cp0));
  EXPECT_TRUE(cps.Read());
  EXPECT_LT(cp0.timestamp(), cps.c()->timestamp());

  disk.Close();
  ::unlink(vdname);
}

};//namespace lfs
