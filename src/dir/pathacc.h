#ifndef LFS_DIR_PATHACC_H_
#define LFS_DIR_PATHACC_H_
#include "util/defs.h"
#include <string>
#include "file/inode_store.h"
#include "dir/dir.h"
namespace lfs {

class PathAccess {
  public:
    PathAccess(InodeStore* istore, Directory* dir);
    
    static std::string GetDirName(const std::string path);
    static std::string GetBaseName(const std::string path);
    
    ino_t Resolve(const std::string path);//find inode by path, returns 0 if not exist
  
  private:
    InodeStore* istore_;
    Directory* dir_;

    Log* log(void) const { return this->istore()->log(); }
    InodeStore* istore(void) const { return this->istore_; }
    void set_istore(InodeStore* value) { this->istore_ = value; }
    Directory* dir(void) const { return this->dir_; }
    void set_dir(Directory* value) { this->dir_ = value; }

    DISALLOW_COPY_AND_ASSIGN(PathAccess);
};

};//namespace lfs
#endif//LFS_DIR_PATHACC_H_

