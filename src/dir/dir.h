#ifndef LFS_DIR_DIR_H_
#define LFS_DIR_DIR_H_
#include "util/defs.h"
#include <string>
#include <vector>
#include "file/file.h"
namespace lfs {

class Directory {
  public:
    Inode* inode(void) const { return this->file()->inode(); }
    File* file(void) const { return this->file_; }
    virtual bool Open(File* file) = 0;
    virtual bool Save(void) = 0;//save to file
    virtual bool Close(void) = 0;
    virtual bool IsEmpty(void) = 0;
    virtual bool List(std::vector<std::string>* names) = 0;//list filenames into names
    virtual ino_t Get(const std::string name) = 0;//find inode by filename, returns 0 if not exist
    virtual bool Add(ino_t ino, const std::string name) = 0;
    virtual bool Remove(const std::string name) = 0;
  
  protected:
    Directory(void) {}
    void set_file(File* value) { this->file_ = value; }
  
  private:
    File* file_;
    DISALLOW_COPY_AND_ASSIGN(Directory);
};

};//namespace lfs
#endif//LFS_DIR_DIR_H_

