#ifndef LFS_FILE_FILE_H_
#define LFS_FILE_FILE_H_
#include "util/defs.h"
#include "log/log.h"
#include "file/inode.h"
namespace lfs {

//represents an open file; one instance even if opened multiple times
//this class writes data blocks and indirect pointers blocks to log, but only updates inode in memory
//unimplemented: doubly/triply indirect pointers
//unimplemented: store file body in inode
//unimplemented: update inode.blocks
class File {
  public:
    File(Inode* inode, Log* log);
    ~File(void);
    Inode* inode(void) const { return this->inode_; }
    Log* log(void) const { return this->log_; }
    size_t noctet_blk(void) const { return this->log()->superblock()->noctet_blk(); }

    bool Read(size_t off, size_t count, charbuf* cb);//off+count cannot go beyond file size
    bool Read(size_t off, size_t count, uint8_t* buf);//(charbuf version is preferred)
    bool Write(size_t off, size_t count, const uint8_t* buf);
    bool Truncate(size_t size);

    //for FsVerifier and Cleaner
    BlkOffset CalcBlkOffset(size_t off);//calculate block index from beginning of file
    SegBlkNum GetAddress(BlkOffset blkoff);//get address of block from inode or indirect pointers block
    SegBlkNum GetIndAddress(BlkOffset blkoff);//get address of indirect pointers block
    bool MoveDataBlk(BlkOffset blkoff);//read and write data block
    bool MoveIndBlk(BlkOffset blkoff);//read and write indirect pointers block

  private:
    Inode* inode_;
    Log* log_;
    uint8_t* writebuf_;//write block buffer, [noctet_blk()]
    void set_inode(Inode* value) { this->inode_ = value; }
    void set_log(Log* value) { this->log_ = value; }
    size_t nblkptr_blk(void) const { return this->noctet_blk() / sizeof(SegBlkNum); }
    uint8_t* writebuf(void);

    size_t CalcOctetOffset(size_t off) { return off % this->noctet_blk(); }//calculate octet offset within block
    bool SetAddress(BlkOffset blkoff, SegBlkNum addr);//set address of block to inode, creating indirect pointers block if needed
    BlkInfo* MakeBlkInfo(BlkType type, BlkOffset blkoff, DateTime mtime = DateTime_invalid);
    
    DISALLOW_COPY_AND_ASSIGN(File);
};

};//namespace lfs
#endif//LFS_FILE_FILE_H_

