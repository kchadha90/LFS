#ifndef LFS_FS_FS_H_
#define LFS_FS_FS_H_
#include "util/defs.h"
#include <map>
#include "file/inode_store.h"
#include "dir/dir.h"
#include "dir/pathacc.h"
namespace lfs {

class Filesystem {
  public:
    Filesystem(void);
    virtual bool Open(Log* log);
    virtual bool Close(void);

    Log* log(void) const { return this->log_; }
    InodeStore* istore(void) const { return this->istore_; }
    PathAccess* pa(void) const { return this->pa_; }
    bool disk_almost_full(void) const { return this->log()->last_cleaning_fail(); }//if true, upper layer is advised to avoid inserting more data
    
    //concurrent opens of the same file/directory should operate on the same instance, so that every client can see the same state
    Inode* GetInode(ino_t ino);
    void ReleaseInode(Inode* inode);
    File* GetFile(ino_t ino);
    void ReleaseFile(File* file);
    Directory* GetDirectory(ino_t ino);
    void ReleaseDirectory(Directory* directory);
    
  protected:
    class OpenFile {
      public:
        OpenFile(Filesystem* fs, Inode* inode);
        ~OpenFile(void);
        void Close(void);
        int refcount(void) const { return this->refcount_; }
        void inc_refcount(void) { ++this->refcount_; }
        void dec_refcount(void) { --this->refcount_; }
        Inode* inode(void) const { return this->inode_; }
        File* file(void);
        Directory* directory(void);
        bool IsInodeChanged(void);//determine whether inode is changed since open or last SetInodeSaved
        void SetInodeSaved(void);
      private:
        Filesystem* fs_;
        int refcount_;
        Inode* inode_;
        Inode* inode_orig_;
        File* file_;
        Directory* directory_;
        Filesystem* fs(void) const { return this->fs_; }
        DISALLOW_COPY_AND_ASSIGN(OpenFile);
    };
    std::map<ino_t, OpenFile*> opens_;
    OpenFile* GetOpen(ino_t ino);
    void ReleaseOpen(ino_t ino);
    
    virtual InodeStore* create_istore(void);
    virtual PathAccess* create_pa(void);
    virtual Directory* create_directory(void);
  
  private:
    Log* log_;
    InodeStore* istore_;
    PathAccess* pa_;

    void set_log(Log* value) { this->log_ = value; }
    void set_istore(InodeStore* value);
    void set_pa(PathAccess* value);
    
    DISALLOW_COPY_AND_ASSIGN(Filesystem);
};

};//namespace lfs
#endif//LFS_FS_FS_H_

