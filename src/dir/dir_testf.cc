#include "dir/dir_testf.h"
#include "file/sysfile_creator.h"
namespace lfs {

void DirTest::SetUp() {
  FileTest::SetUp();
  this->istore = new InodeStore();
  ASSERT_TRUE(this->istore->Open(this->log));
  
  SystemFileCreator sfc(this->istore);
  ASSERT_TRUE(sfc.CreateAll());
}

void DirTest::TearDown() {
  this->istore->Close();
  delete this->istore;
  FileTest::TearDown();
}

};//namespace lfs
