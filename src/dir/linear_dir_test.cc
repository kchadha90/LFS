#include "dir/pathacc.h"
#include "dir/linear_dir.h"
#include <cstdio>
#include <unordered_set>
#include "dir/dir_testf.h"
namespace lfs {

TEST_F(DirTest, LinearRootDirectory) {
  LinearRootDirectory lrd(this->istore);
  PathAccess pa(this->istore, &lrd);
  Inode inode_root; inode_root.set_ino(Inode::kIno_Root);
  ASSERT_TRUE(this->istore->Read(&inode_root));
  File file_rootdir(&inode_root, this->log);
  
  std::vector<std::string> names; names.push_back("dummy");
  ASSERT_TRUE(lrd.Open(&file_rootdir));
  EXPECT_TRUE(lrd.List(&names));
  EXPECT_EQ((size_t)0, names.size());
  EXPECT_TRUE(lrd.Close());
  
  EXPECT_EQ((ino_t)Inode::kIno_Root, pa.Resolve("/"));

  int32_t filenums[20] = { 49, 17, 13, 1, 14, 44, 30, 22, 2, 16, 27, 12, 15, 42, 23, 57, 20, 7, 35, 31 };
  const mode_t allperms = S_IRUSR | S_IWUSR | S_IXUSR | S_IRGRP | S_IWGRP | S_IXGRP | S_IROTH | S_IWOTH | S_IXOTH;
  char path[12]; Inode inode;
  for (int i = 0; i < 20; ++i) {
    snprintf(path, 12, "/%"PRId32, filenums[i]);
    EXPECT_TRUE(this->istore->AllocateIno(S_IFREG | allperms, path, &inode));
    inode.set_version(filenums[i]);
    EXPECT_EQ((ino_t)0, pa.Resolve(path));
    this->istore->Write(&inode);
    EXPECT_EQ(inode.ino(), pa.Resolve(path));
    EXPECT_FALSE(this->istore->AllocateIno(S_IFREG | allperms, path, &inode));
  }
  
  std::unordered_set<std::string> expected_names;
  for (int i = 0; i < 20; ++i) {
    snprintf(path, 12, "%"PRId32, filenums[i]);
    expected_names.insert(path);
  }
  names.push_back("dummy");
  ASSERT_TRUE(lrd.Open(&file_rootdir));
  EXPECT_TRUE(lrd.List(&names));
  ASSERT_EQ((size_t)20, names.size());
  EXPECT_TRUE(lrd.Close());
  for (int i = 0; i < 20; ++i) {
    EXPECT_EQ(1U, expected_names.erase(names[i]));
  }

  inode.Clear();
  inode.set_ino(Inode::kIno_Good);
  inode.set_version(9999);
  this->istore->Write(&inode);
  EXPECT_EQ((ino_t)0, pa.Resolve("/0"));
  
  EXPECT_EQ((ino_t)0, pa.Resolve("/0"));
  EXPECT_EQ((ino_t)0, pa.Resolve("/hello"));
  EXPECT_EQ((ino_t)0, pa.Resolve(""));
  EXPECT_FALSE(this->istore->AllocateIno(S_IFREG | allperms, "/", &inode));
  EXPECT_FALSE(this->istore->AllocateIno(S_IFREG | allperms, "/0", &inode));
  EXPECT_FALSE(this->istore->AllocateIno(S_IFREG | allperms, "/hello", &inode));
  EXPECT_FALSE(this->istore->AllocateIno(S_IFREG | allperms, "", &inode));

  EXPECT_FALSE(this->istore->AllocateIno(S_IFDIR | allperms, "/62", &inode));
  EXPECT_FALSE(this->istore->AllocateIno(S_IFCHR | allperms, "/62", &inode));
  EXPECT_FALSE(this->istore->AllocateIno(S_IFBLK | allperms, "/62", &inode));
  EXPECT_TRUE(this->istore->AllocateIno(S_IFREG | allperms, "/62", &inode));
  EXPECT_FALSE(this->istore->AllocateIno(S_IFIFO | allperms, "/62", &inode));
  EXPECT_TRUE(this->istore->AllocateIno(S_IFLNK | allperms, "/62", &inode));
  EXPECT_FALSE(this->istore->AllocateIno(S_IFSOCK | allperms, "/62", &inode));
}

};//namespace lfs
