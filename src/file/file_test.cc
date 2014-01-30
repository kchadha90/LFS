#include "file/file.h"
#include "file/file_testf.h"
#include <cstdlib>
namespace lfs {

TEST_F(FileTest, File) {
  Inode inode;
  inode.set_ino(120);
  File file(&inode, this->log);
  
  charbuf cb;
  uint8_t buf[1024];

  memset(buf, 0x01, sizeof(buf));
  EXPECT_TRUE(file.Write(700, 40, buf));
  EXPECT_EQ((size_t)740, inode.size());
  EXPECT_NE(SegBlkNum_unassigned, inode.blkptr(1));
  
  cb.clear();
  EXPECT_FALSE(file.Read(680, 61, &cb));//cannot read beyond file end
  EXPECT_TRUE(file.Read(680, 60, &cb));
  EXPECT_EQ((size_t)60, cb.size());
  EXPECT_EQ((size_t)20, cb.find_first_not_of((uint8_t)0x00, 0));
  EXPECT_EQ(charbuf::npos, cb.find_first_not_of((uint8_t)0x01, 20));

  memset(buf, 0x02, sizeof(buf));
  EXPECT_TRUE(file.Write(29500, 500, buf));
  memset(buf, 0x03, sizeof(buf));
  EXPECT_TRUE(file.Write(30000, 1000, buf));
  EXPECT_EQ((size_t)31000, inode.size());
  EXPECT_NE(SegBlkNum_unassigned, inode.blkptr(Inode::kIndBlk));
  
  cb.clear();
  EXPECT_TRUE(file.Read(29400, 1600, &cb));
  EXPECT_EQ((size_t)100, cb.find_first_not_of((uint8_t)0x00, 0));
  EXPECT_EQ((size_t)600, cb.find_first_not_of((uint8_t)0x02, 100));
  EXPECT_EQ(charbuf::npos, cb.find_first_not_of((uint8_t)0x03, 600));
  
  EXPECT_TRUE(file.MoveDataBlk(58));
  EXPECT_TRUE(file.MoveIndBlk(Inode::kNDirBlks));
  cb.clear();
  EXPECT_TRUE(file.Read(29400, 1600, &cb));
  EXPECT_EQ((size_t)100, cb.find_first_not_of((uint8_t)0x00, 0));
  EXPECT_EQ((size_t)600, cb.find_first_not_of((uint8_t)0x02, 100));
  EXPECT_EQ(charbuf::npos, cb.find_first_not_of((uint8_t)0x03, 600));
}

};//namespace lfs
