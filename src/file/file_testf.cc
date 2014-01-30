#include "file/file_testf.h"
#include "disk/memdisk.h"
#include "log/logcreator.h"
namespace lfs {

void FileTest::SetUp() {
  LogCreator lc;
  lc.set_noctet_sector(512);
  lc.set_nsector_blk(1);
  lc.set_nblk_seg(32);
  lc.set_nseg_disk(20);
  ASSERT_TRUE(lc.VerifyMetrics());
  MemDisk* md = new MemDisk();
  ASSERT_TRUE(md->Init(lc.noctet_sector(), lc.noctet_disk() / lc.noctet_sector()));
  ASSERT_TRUE(lc.set_disk(md));
  ASSERT_TRUE(lc.WriteSuperblock());
  ASSERT_TRUE(lc.WriteCheckpoints());
  
  this->disk = md;
  this->log = new Log();
  ASSERT_TRUE(this->log->Open(md));
}

void FileTest::TearDown() {
  this->log->Close();
  delete this->log;
  delete this->disk;
}

};//namespace lfs
