#ifndef LFS_FILE_SYSFILE_CREATOR_H_
#define LFS_FILE_SYSFILE_CREATOR_H_
#include "util/defs.h"
#include "file/inode_store.h"
namespace lfs {

class SystemFileCreator {
  public:
    SystemFileCreator(InodeStore* istore);
    bool CreateAll(void);
    bool WriteInoMap(File* file);//called by InodeStore

  private:
    InodeStore* istore_;
    InodeStore* istore(void) const { return this->istore_; }
    void set_istore(InodeStore* value) { this->istore_ = value; }
    Log* log(void) const { return this->istore()->log(); }

    bool CreateFile(ino_t ino);
    bool WriteDirectory(File* file);
    DISALLOW_COPY_AND_ASSIGN(SystemFileCreator);
};

};//namespace lfs
#endif//LFS_FILE_SYSFILE_CREATOR_H_
