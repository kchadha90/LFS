#ifndef LFS_FILE_INODE_STORE_H_
#define LFS_FILE_INODE_STORE_H_
#include "util/defs.h"
#include <string>
#include "file/file.h"
namespace lfs {

class LinearRootDirectory;
class SystemFileCreator;

//read and write inodes
//unimplemented: don't write InodeTbl when only atime changes
class InodeStore {
  public:
    InodeStore(void);
    bool Open(Log* log);
    void Close(void);
    Log* log(void) const { return this->log_; }

    void set_linear_rootdir(LinearRootDirectory* value);//attach LinearRootDirectory for special allocation and ino transforming policy; called by LinearRootDirectory
    bool InitializeInoFiles(SystemFileCreator* sysfile_creator);//initialize inomap,inovec,inomap; called by SystemFileCreator
    File* GetInoFile(ino_t ino);//get file of inomap,inovec,inomap; called by Cleaner
    void SaveInoFiles(void);//save inodes of inomap,inovec,inomap to checkpoint; called by Log
    ino_t TransformIno(ino_t ino) const;//transform ino for external display
    
    bool AllocateIno(mode_t mode, const std::string path, Inode* inode) const;//allocate free inode number
    bool IsInUse(ino_t ino) const;//determine whether inode number is in use

    bool Read(Inode* inode) const;//read full inode, caller should set inode->ino before
    bool Write(const Inode* inode);//write full inode
    bool Delete(ino_t ino);//delete inode, inode number becomes free
    
    InodeVer ReadVersion(ino_t ino) const;//read inode version; can also use on free ino to read out old version

  private:
    Log* log_;
    Inode inode_map_;
    Inode inode_vec_;
    Inode inode_tbl_;
    File* file_map_;
    File* file_vec_;
    File* file_tbl_;
    LinearRootDirectory* linear_rootdir_;
    
    Checkpoint* checkpoint(void) const { return this->log()->checkpoint(); }
    File* file_map(void) const { return this->file_map_; }
    File* file_vec(void) const { return this->file_vec_; }
    File* file_tbl(void) const { return this->file_tbl_; }
    LinearRootDirectory* linear_rootdir(void) const { return this->linear_rootdir_; }
    
    ino_t AllocateIno(mode_t mode, const std::string path) const;//allocate free inode number, 0=failure
    void CalcMapOffsetMask(ino_t ino, size_t* off, uint8_t* mask) const;
    bool SetInUse(ino_t ino, bool in_use);
    
    DISALLOW_COPY_AND_ASSIGN(InodeStore);
};

};//namespace lfs
#endif//LFS_FILE_INODE_STORE_H_
