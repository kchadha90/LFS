#ifndef LFS_DIR_LINEAR_DIR_H_
#define LFS_DIR_LINEAR_DIR_H_
#include "util/defs.h"
#include "dir/dir.h"
#include "file/inode_store.h"
namespace lfs {

class LinearRootDirectory : public Directory {
  public:
    LinearRootDirectory(InodeStore* istore);
    bool Open(File* file);
    bool Save(void) { return true; }
    bool Close(void) { return true; }
    bool IsEmpty(void) { return false; }
    bool List(std::vector<std::string>* names);
    ino_t Get(const std::string name);
    bool Add(ino_t ino, const std::string name) { return true; }
    bool Remove(const std::string name) { return true; }
    
    void set_file_inomap(File* value) { this->file_inomap_ = value; }
    ino_t AllocateIno(mode_t mode, const std::string path) const;
    ino_t TransformIno(ino_t ino) const;
  
  private:
    File* file_inomap_;
    InodeStore* istore_;

    InodeStore* istore(void) const { return this->istore_; }
    void set_istore(InodeStore* value) { this->istore_ = value; }
    File* file_inomap(void) const { return this->file_inomap_; }

    DISALLOW_COPY_AND_ASSIGN(LinearRootDirectory);
};

};//namespace lfs
#endif//LFS_DIR_LINEAR_DIR_H_

