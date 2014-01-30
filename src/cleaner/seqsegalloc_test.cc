#include "cleaner/seqsegalloc.h"
#include "exttool/gtest.h"
namespace lfs {

class CleanerTestSequentialSegAllocatorSuperblock : public Superblock {
  public:
    CleanerTestSequentialSegAllocatorSuperblock() {
      this->set_first_logseg(4);
      this->set_nseg_disk(10);
    }
};

TEST(CleanerTest, SequentialSegAllocator) {
  CleanerTestSequentialSegAllocatorSuperblock sb;
  SequentialSegAllocator ssa(&sb, 7);
  
  EXPECT_EQ((SegNum)7, ssa.Next());
  EXPECT_EQ((SegNum)8, ssa.Next());
  EXPECT_EQ((SegNum)9, ssa.Next());
  EXPECT_EQ(SegNum_invalid, ssa.Next());
  EXPECT_EQ(SegNum_invalid, ssa.Next());
}

};//namespace lfs
