#ifndef LFS_FILE_FILE_TESTF_H_
#define LFS_FILE_FILE_TESTF_H_
#include "log/log.h"
#include "exttool/gtest.h"
namespace lfs {

//File test fixture, provides Log for file tests
class FileTest : public ::testing::Test {
  protected:
    virtual void SetUp();
    virtual void TearDown();
    Disk* disk;
    Log* log;
};

};//namespace lfs
#endif//LFS_FILE_FILE_TESTF_H_
