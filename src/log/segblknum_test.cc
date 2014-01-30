#include "log/segblknum.h"
#include "exttool/gtest.h"
namespace lfs {

TEST(LogTest, SegBlkNum) {

  SegNum seg = 0x9000DDDD;
  BlkNum blk = 0x0509;
  SegBlkNum segblk = UINT64_C(0x9000DDDD00000509);

  EXPECT_EQ(segblk, SegBlkNum_make(seg, blk));
  EXPECT_EQ(seg, SegBlkNum_seg(segblk));
  EXPECT_EQ(blk, SegBlkNum_blk(segblk));
}

};//namespace lfs
