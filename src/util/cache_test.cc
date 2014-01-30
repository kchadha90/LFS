#include "util/cache.h"
#include "exttool/gtest.h"
namespace lfs {

TEST(UtilTest, Cache) {

  Cache<int,int> c1(2);
  EXPECT_EQ(0, c1.Get(0, 0));
  EXPECT_EQ(50, c1.Get(0, 50));
  c1.Put(1, 11);
  c1.Put(2, 12);
  EXPECT_EQ(11, c1.Get(1, 0));
  EXPECT_EQ(11, c1.Get(1, 0));
  EXPECT_EQ(12, c1.Get(2, 0));

  Cache<int,int> c2(2);
  c2.Put(1, 11);//list=1
  c2.Put(2, 12);//list=2,1
  c2.Put(3, 13);//list=3,2
  EXPECT_EQ(0, c2.Get(1, 0));
  EXPECT_EQ(12, c2.Get(2, 0));//list=2,3
  EXPECT_EQ(13, c2.Get(3, 0));//list=3,2
  EXPECT_EQ(12, c2.Get(2, 0));//list=2,3
  c2.Put(4, 14);//list=4,2
  EXPECT_EQ(0, c2.Get(3, 0));
}

};//namespace lfs
