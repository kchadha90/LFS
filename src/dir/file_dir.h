#ifndef LFS_DIR_FILE_DIR_H_
#define LFS_DIR_FILE_DIR_H_
#include "util/defs.h"
#include <unordered_map>
#include "dir/dir.h"
namespace lfs {

class FileDirectory : public Directory {
  public:
    FileDirectory(void);
    bool Open(File* file);
    bool Save(void);
    bool Close(void);
    bool IsEmpty(void);
    bool List(std::vector<std::string>* names);
    ino_t Get(const std::string name);
    bool Add(ino_t ino, const std::string name);
    bool Remove(const std::string name);
    
  private:
    bool dirty_;//true if there is unsaved changes
    std::unordered_map<std::string,ino_t> list_;//name=>ino
    bool Load(void);
    DISALLOW_COPY_AND_ASSIGN(FileDirectory);
};

};//namespace lfs
#endif//LFS_DIR_FILE_DIR_H_

