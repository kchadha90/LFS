#ifndef LFS_FS_VERIFIER_H_
#define LFS_FS_VERIFIER_H_
#include "util/defs.h"
#include <string>
#include <vector>
#include <unordered_map>
#include "disk/rodisk.h"
#include "fs/fs.h"
namespace lfs {

//unverified: meta of indirect pointers block
class FsVerifier {
  public:
    FsVerifier(void);
    ~FsVerifier(void);
    const std::vector<std::string>* msgs(void) const { return &this->msgs_; }
    bool Verify(Disk* disk);
    
  private:
    std::vector<std::string> msgs_;
    ReadOnlyDisk* disk_;
    Log* log_;
    Filesystem* fs_;
    std::unordered_map<SegNum,BlkNum> live_blocks_;//seg=>number of live blocks, counted from block pointers
    std::unordered_map<ino_t,nlink_t> inode_nlink_;//ino=>nlink, counted from inodes
    std::unordered_map<ino_t,nlink_t> file_nlink_;//ino=>nlink, counted from directory entries
    
    Disk* disk(void) const { return this->disk_; }
    Log* log(void) const { return this->log_; }
    Filesystem* fs(void) const { return this->fs_; }
    
    void AddMsg(const char* format, ...);
    void AddLiveBlock(SegBlkNum segblk);
    void AddFileNlink(ino_t ino);

    bool InitLog(void);//initialize Log
    bool InitFs(void);//initialize Filesystem
    
    bool IterateInodes(void);//iterate over inodes
    ino_t DetermineMaxIno(void);//determine max ino by inspecting inode_inomap.size()
    bool VerifyFile(ino_t ino);//get File and call VerifyFile(file)
    bool VerifySystemFile(ino_t ino);//construct File from Inode in checkpoint and call VerifyFile(file)
    bool VerifyFile(File* file);//verify file blocks has correct meta, counting live_blocks
    bool VerifyBlock(SegBlkNum segblk, Inode* inode, BlkType type, BlkOffset blkoff);//verify block has correct meta
    bool VerifyDirectory(Directory* dir);//verify directory structure, counting file_nlink
    
    bool VerifyNlink(void);//verify inode_nlink matches file_nlink
    bool VerifySegUsage(void);//verify segment usage table matches live_blocks
    
    DISALLOW_COPY_AND_ASSIGN(FsVerifier);
};

};//namespace lfs
#endif//LFS_FS_VERIFIER_H_

