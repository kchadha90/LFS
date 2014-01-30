#include "file/inode_store.h"
#include "file/file_testf.h"
#include <cstdio>
#include <set>
namespace lfs {

TEST_F(FileTest, InodeStore) {
  InodeStore istore;
  ASSERT_TRUE(istore.Open(this->log));
  Inode inode; ino_t ino;
  char path[12];

  std::set<ino_t> ino_set;
  for (int i = 0; i < 72; ++i) {//don't set large upperbound before implementing doubly indirect
    snprintf(path, 12, "/%d", i);
    ASSERT_TRUE(istore.AllocateIno(0100644U, path, &inode));
    ino = inode.ino();
    ASSERT_EQ(0U, ino_set.count(ino));
    ASSERT_FALSE(istore.IsInUse(ino));
    ino_set.insert(ino);
    inode.set_mode(ino%2==0 ? 0040777U : 0100644U);
    inode.set_atime(DateTime_now());
    inode.set_version(100000 + ino * 7);
    inode.set_blkptr(ino % 10, ino * 97 % 5);
    ASSERT_TRUE(istore.Write(&inode));
  }
  
  DateTime atime = 0;
  for (std::set<ino_t>::const_iterator it = ino_set.cbegin(); it != ino_set.cend(); ++it) {
    ino_t ino = *it;
    EXPECT_TRUE(istore.IsInUse(ino));
    inode.Clear();
    inode.set_ino(ino);
    EXPECT_TRUE(istore.Read(&inode));
    EXPECT_EQ(ino%2==0 ? 0040777U : 0100644U, inode.mode());
    EXPECT_LE(atime, inode.atime());
    atime = inode.atime();
    EXPECT_EQ(100000 + ino * 7, inode.version());
    EXPECT_EQ(ino * 97 % 5, inode.blkptr(ino % 10));
    istore.Delete(ino);
    EXPECT_FALSE(istore.Read(&inode));
    EXPECT_FALSE(istore.IsInUse(ino));
  }
}

};//namespace lfs
