#include "file/file.h"
#include <cstdlib>
#include <cstring>
#include "util/logging.h"
namespace lfs {

File::File(Inode* inode, Log* log) {
  assert(inode != NULL);
  assert(log != NULL);
  this->set_inode(inode);
  this->set_log(log);
  this->writebuf_ = NULL;
}

File::~File(void) {
  if (this->writebuf_ != NULL) free(this->writebuf_);
}

bool File::Read(size_t off, size_t count, charbuf* cb) {
  if (count == 0) return true;
  if (off + count > this->inode()->size()) {
    Logging::Log(kLLInfo, kLCFile, "File::Read(ino=%ju,size=%zu,off=%zu,count=%zu) read beyond end", (uintmax_t)this->inode()->ino(), this->inode()->size(), off, count);
    return false;
  }
  BlkOffset blkoff_start = this->CalcBlkOffset(off);
  BlkOffset blkoff_end = this->CalcBlkOffset(off + count - 1);

  for (BlkOffset blkoff = blkoff_start; blkoff <= blkoff_end; ++blkoff) {
    SegBlkNum addr = this->GetAddress(blkoff);
    size_t octet_start = blkoff == blkoff_start ? this->CalcOctetOffset(off) : 0,
           octet_end = blkoff == blkoff_end ? this->CalcOctetOffset(off + count - 1) : this->noctet_blk() - 1;
    if (!this->log()->br()->Read(addr, octet_start, 1 + octet_end - octet_start, cb)) return false;
  }
  return true;
}

bool File::Read(size_t off, size_t count, uint8_t* buf) {
  charbuf cb;
  if (!this->Read(off, count, &cb)) return false;
  cb.copy(buf, count);
  return true;
}

bool File::Write(size_t off, size_t count, const uint8_t* buf) {
  if (count == 0) return true;
  BlkOffset blkoff_start = this->CalcBlkOffset(off);
  BlkOffset blkoff_end = this->CalcBlkOffset(off + count - 1);
  if (blkoff_end == BlkOffset_invalid) {
    Logging::Log(kLLWarn, kLCFile, "File::Write(ino=%"PRIuMAX",off=%"PRIuMAX",count="PRIuMAX") fails for length limit", (uintmax_t)this->inode()->ino(), (uintmax_t)off, (uintmax_t)count);
    return false;
  }
  if (off + count > this->inode()->size()) this->inode()->set_size(off + count);
  const uint8_t* buf_base = buf - this->CalcOctetOffset(off);

  for (BlkOffset blkoff = blkoff_start; blkoff <= blkoff_end; ++blkoff) {
    const uint8_t* wbuf;
    const uint8_t* buf_blk_base = buf_base + (blkoff - blkoff_start) * this->noctet_blk();
    
    if (blkoff == blkoff_start || blkoff == blkoff_end) {
      uint8_t* writebuf = this->writebuf();
      SegBlkNum oldaddr = this->GetAddress(blkoff);
      if (!this->log()->br()->Read(oldaddr, writebuf)) return false;
      size_t octet_start = blkoff==blkoff_start ? this->CalcOctetOffset(off) : 0,
             octet_end = blkoff==blkoff_end ? this->CalcOctetOffset(off+count-1) : this->noctet_blk()-1;
      memcpy(writebuf+octet_start, buf_blk_base+octet_start, octet_end+1-octet_start);
      wbuf = writebuf;
    } else {
      wbuf = buf_blk_base;
    }
    SegBlkNum addr = this->log()->bw()->Put(this->MakeBlkInfo(kBTData, blkoff), wbuf);
    if (addr == SegBlkNum_invalid) return false;
    if (!this->SetAddress(blkoff, addr)) return false;
  }
  this->inode()->set_mtime(DateTime_now());
  return true;
}

bool File::Truncate(size_t size) {
  if (size < this->inode()->size()) {//file size decrease, die unused blocks
    BlkOffset die_begin = size==0 ? 0 : this->CalcBlkOffset(size - 1) + 1;
    BlkOffset die_end = this->CalcBlkOffset(this->inode()->size() - 1);
    for (BlkOffset blkoff = die_begin; blkoff <= die_end; ++blkoff) {
      this->SetAddress(blkoff, SegBlkNum_unassigned);
    }
    if (die_begin <= (BlkOffset)Inode::kNDirBlks) {
      SegBlkNum oldindaddr = this->inode()->blkptr(Inode::kIndBlk);
      if (oldindaddr != SegBlkNum_unassigned) {
        this->inode()->set_blkptr(Inode::kIndBlk, SegBlkNum_unassigned);
        this->log()->bw()->Die(oldindaddr);
      }
    }
  }
  if (size > 0 && this->CalcBlkOffset(size-1) == BlkOffset_invalid) {
    Logging::Log(kLLWarn, kLCFile, "File::Truncate(ino=%"PRIuMAX",size=%"PRIuMAX") fails for length limit", (uintmax_t)this->inode()->ino(), (uintmax_t)size);
    return false;
  }
  this->inode()->set_size(size);
  this->inode()->set_mtime(DateTime_now());
  if (size == 0) this->inode()->set_version(1+this->inode()->version());
  return true;
}

uint8_t* File::writebuf(void) {
  if (this->writebuf_ == NULL) this->writebuf_ = (uint8_t*)malloc(this->noctet_blk());
  return this->writebuf_;
}

BlkOffset File::CalcBlkOffset(size_t off) {
  BlkOffset blkoff = off / this->noctet_blk();
  if (blkoff >= (BlkOffset)(Inode::kNDirBlks + this->nblkptr_blk())) return BlkOffset_invalid;
  return blkoff;
}

SegBlkNum File::GetAddress(BlkOffset blkoff) {
  if (blkoff == BlkOffset_invalid) return SegBlkNum_invalid;

  if (blkoff < (BlkOffset)Inode::kNDirBlks) {//direct
    return this->inode()->blkptr(blkoff);
  }
  
  if (blkoff < (BlkOffset)(Inode::kNDirBlks + this->nblkptr_blk())) {//singly indirect
    SegBlkNum ind_addr = this->inode()->blkptr(Inode::kIndBlk);
    if (ind_addr == SegBlkNum_unassigned) return SegBlkNum_unassigned;
    size_t blkptr_off = sizeof(SegBlkNum) * (blkoff - Inode::kNDirBlks);
    SegBlkNum addr;
    if (!this->log()->br()->Read(ind_addr, blkptr_off, sizeof(SegBlkNum), (uint8_t*)&addr)) return SegBlkNum_invalid;
    return be64toh(addr);
  }
  
  return SegBlkNum_invalid;
}

SegBlkNum File::GetIndAddress(BlkOffset blkoff) {
  if (blkoff == (BlkOffset)Inode::kNDirBlks) {//singly indirect
    return this->inode()->blkptr(Inode::kIndBlk);
  }
  
  return SegBlkNum_invalid;
}

bool File::SetAddress(BlkOffset blkoff, SegBlkNum addr) {
  if (blkoff == BlkOffset_invalid) return false;

  if (blkoff < (BlkOffset)Inode::kNDirBlks) {//direct
    SegBlkNum oldaddr = this->inode()->blkptr(blkoff);
    this->inode()->set_blkptr(blkoff, addr);
    this->log()->bw()->Die(oldaddr);
    return true;
  }
  
  if (blkoff < (BlkOffset)(Inode::kNDirBlks + this->nblkptr_blk())) {//singly indirect
    SegBlkNum ind_oldaddr = this->inode()->blkptr(Inode::kIndBlk);
    uint8_t* indblk = this->writebuf();
    if (ind_oldaddr == SegBlkNum_unassigned) {
      SegBlkNum* indblk_p = (SegBlkNum*)indblk;
      SegBlkNum* indblk_end = indblk_p + this->nblkptr_blk();
      for (; indblk_p < indblk_end; ++indblk_p) *indblk_p = be64toh(SegBlkNum_unassigned);
    } else {
      if (!this->log()->br()->Read(ind_oldaddr, indblk)) { free(indblk); return false; }//read old indblk
    }
    size_t blkptr_off = sizeof(SegBlkNum) * (blkoff - Inode::kNDirBlks);
    SegBlkNum oldaddr = htobe64(*(SegBlkNum*)(indblk + blkptr_off));
    *(SegBlkNum*)(indblk + blkptr_off) = htobe64(addr);//overwrite blkptr
    BlkInfo* bi = this->MakeBlkInfo(kBTInd, Inode::kNDirBlks);
    SegBlkNum ind_addr = this->log()->bw()->Put(bi, indblk);//write new indblk
    if (ind_addr == SegBlkNum_invalid) return false;
    this->inode()->set_blkptr(Inode::kIndBlk, ind_addr);//update indblkptr
    this->log()->bw()->Die(ind_oldaddr);
    this->log()->bw()->Die(oldaddr);
    return true;
  }
  
  return false;
}

BlkInfo* File::MakeBlkInfo(BlkType type, BlkOffset blkoff, DateTime mtime) {
  BlkInfo* bi = new BlkInfo();
  bi->set_type(type);
  bi->set_ino(this->inode()->ino());
  bi->set_ver(this->inode()->version());
  bi->set_off(blkoff);
  bi->set_mtime(mtime==DateTime_invalid ? DateTime_now() : mtime);
  return bi;
}

bool File::MoveDataBlk(BlkOffset blkoff) {
  SegBlkNum oldaddr = this->GetAddress(blkoff);
  if (oldaddr == SegBlkNum_invalid) return false;
  if (oldaddr == SegBlkNum_unassigned) return true;
  if (!this->log()->br()->Read(oldaddr, this->writebuf())) return false;
  BlkInfo oldbi;
  if (!this->log()->br()->GetMeta(oldaddr, &oldbi)) return false;

  SegBlkNum addr = this->log()->bw()->Put(this->MakeBlkInfo(kBTData, blkoff, oldbi.mtime()), this->writebuf());
  if (addr == SegBlkNum_invalid) return false;
  if (!this->SetAddress(blkoff, addr)) return false;
  return true;
}

bool File::MoveIndBlk(BlkOffset blkoff) {
  SegBlkNum oldaddr = this->GetIndAddress(blkoff);
  if (oldaddr == SegBlkNum_invalid) return false;
  if (oldaddr == SegBlkNum_unassigned) return true;
  if (!this->log()->br()->Read(oldaddr, this->writebuf())) return false;
  BlkInfo oldbi;
  if (!this->log()->br()->GetMeta(oldaddr, &oldbi)) return false;

  SegBlkNum addr = this->log()->bw()->Put(this->MakeBlkInfo(kBTInd, blkoff, oldbi.mtime()), this->writebuf());
  if (addr == SegBlkNum_invalid) return false;

  if (blkoff == (BlkOffset)Inode::kNDirBlks) {//singly indirect
    this->inode()->set_blkptr(Inode::kIndBlk, addr);
    this->log()->bw()->Die(oldaddr);
    return true;
  }
  return false;
}

};//namespace lfs
