#include "log/seg_summary.h"
#include <cstring>
#include "exttool/gtest.h"
namespace lfs {

TEST(LogTest, BlkInfo) {

  uint8_t buf1[BlkInfo::kNoctet] = { 0x53, 0x11, 0x00, 0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x01, 0x02, 0x03, 0x04, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0xA1, 0xA2 };
  uint8_t buf2[BlkInfo::kNoctet];
  
  BlkInfo bi;
  bi.Read(buf1);
  bi.Write(buf2);
  ASSERT_EQ(0, memcmp(buf1, buf2, BlkInfo::kNoctet));

  ASSERT_EQ((BlkSeq)0x311, bi.seq());
  ASSERT_EQ(kBTInd, bi.type());
  ASSERT_EQ((ino_t)0x11223344, bi.ino());
  ASSERT_EQ((InodeVer)0x55667788, bi.ver());
  ASSERT_EQ((BlkOffset)0x01020304, bi.off());
  ASSERT_EQ(UINT64_C(0xAABBCCDDEEFFA1A2), bi.mtime());
}

TEST(LogTest, SegInfo) {

  uint8_t buf1[SegInfo::kNoctet] = { 0x53, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF, 0xA1, 0xA2 };
  uint8_t buf2[SegInfo::kNoctet];
  
  SegInfo si;
  si.Read(buf1);
  si.Write(buf2);
  ASSERT_EQ(0, memcmp(buf1, buf2, SegInfo::kNoctet));

  ASSERT_EQ((SegNum)0x53110000, si.next_seg());
  ASSERT_EQ(UINT64_C(0xAABBCCDDEEFFA1A2), si.timestamp());
}


};//namespace lfs
