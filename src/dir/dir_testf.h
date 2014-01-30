#ifndef LFS_DIR_DIR_TESTF_H_
#define LFS_DIR_DIR_TESTF_H_
#include "file/file_testf.h"
#include "file/inode_store.h"
namespace lfs {

//Dir test fixture, provides Log for file tests
class DirTest : public FileTest {
  protected:
    virtual void SetUp();
    virtual void TearDown();
    InodeStore* istore;
};

};//namespace lfs
#endif//LFS_DIR_DIR_TESTF_H_
