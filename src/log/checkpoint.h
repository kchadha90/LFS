#ifndef LFS_LOG_CHECKPOINT_H_
#define LFS_LOG_CHECKPOINT_H_
#include "util/defs.h"
#include "log/superblock.h"
#include "log/segio.h"
#include "log/physical.pb.h"
#include "file/inode.h"
namespace lfs {

class SegUsage;

class Checkpoint {
  public:
    Checkpoint(SegIO* segio);
    ~Checkpoint(void);
    Superblock* superblock(void) const { return this->segio()->superblock(); }
    bool Read(int regionIndex);//read from region 0 or 1
    bool Write(int regionIndex);//write to region 0 or 1, does not set timestamp
    bool IsValid(void);//determines whether this checkpoint is valid
    bool IsPreferred(Checkpoint* other);//compares whether this checkpoint is preferred over the other
    
    DateTime timestamp(void) const;
    void set_timestamp(DateTime value);
    SegNum next_seg(void) const;
    void set_next_seg(SegNum value);
    void get_inode_inomap(Inode* inode) const;
    void set_inode_inomap(Inode* inode);
    void get_inode_inovec(Inode* inode) const;
    void set_inode_inovec(Inode* inode);
    void get_inode_inotbl(Inode* inode) const;
    void set_inode_inotbl(Inode* inode);
    SegUsage* segusage(void) const { return this->segusage_; }

  private:
    SegIO* segio_;
    lfs::physical::Checkpoint* p_;
    SegUsage* segusage_;
    SegIO* segio(void) const { return this->segio_; }
    lfs::physical::Checkpoint* p(void) const { return this->p_; }
    size_t off_dup_timestamp(void) const;//offset of duplicate timestamp
    void get_inode(lfs::physical::InodeSimp* p, ino_t ino, Inode* inode) const;//copy from InodeSimp to Inode
    void set_inode(lfs::physical::InodeSimp* p, ino_t ino, Inode* inode) const;//copy from Inode to InodeSimp
    DISALLOW_COPY_AND_ASSIGN(Checkpoint);
};

class Checkpoints {
  public:
    Checkpoints(SegIO* segio);
    ~Checkpoints(void);
    Checkpoint* c(void) const { return this->c_; }//return value is owned by Checkpoints
    bool Read(void);//read from both regions, returns true if either region has a valid checkpoint
    bool Write(void);//write to non-preferred region; will update timestamp
    
  private:
    SegIO* segio_;
    int preferred_region_;//preferred region on disk, next write goes to the other region
    Checkpoint* c_;//current checkpoint
    SegIO* segio(void) const { return this->segio_; }
    void set_segio(SegIO* value) { this->segio_ = value; }
    int preferred_region(void) const { return this->preferred_region_; }
    void set_preferred_region(int value) { this->preferred_region_ = value; }
    void set_c(Checkpoint* value) { this->c_ = value; }
    DISALLOW_COPY_AND_ASSIGN(Checkpoints);
};

};//namespace lfs
#endif//LFS_LOG_CHECKPOINT_H_
