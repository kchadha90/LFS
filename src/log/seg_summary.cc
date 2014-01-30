#include "log/seg_summary.h"
#include <cstring>
#include "util/endian.h"
namespace lfs {

bool BlkInfo::Read(uint8_t* buf) {
  this->set_seq(((uint16_t)(*(buf + 0) & 0x0F) << 8) + *(buf + 1));
  this->set_type((BlkType)(*(buf + 0) >> 4));
  this->set_ino(be32toh(*(uint32_t*)(buf + 4)));
  this->set_ver(be32toh(*(uint32_t*)(buf + 8)));
  this->set_off(be32toh(*(uint32_t*)(buf + 12)));
  this->set_mtime(be64toh(*(uint64_t*)(buf + 16)));
  return true;
}

bool BlkInfo::Write(uint8_t* buf) const {
  ::memset(buf, 0, this->kNoctet);
  *(buf + 0) = (this->type() << 4) | ((this->seq() >> 8) & 0xF);
  *(buf + 1) = (uint8_t)(this->seq() & 0xFF);
  *(uint32_t*)(buf + 4) = htobe32(this->ino());
  *(uint32_t*)(buf + 8) = htobe32(this->ver());
  *(uint32_t*)(buf + 12) = htobe32(this->off());
  *(uint64_t*)(buf + 16) = htobe64(this->mtime());
  return true;
}

void BlkInfo::CopyFrom(const BlkInfo* other) {
  this->set_segblk(other->segblk());
  this->set_seq(other->seq());
  this->set_type(other->type());
  this->set_ino(other->ino());
  this->set_ver(other->ver());
  this->set_off(other->off());
  this->set_mtime(other->mtime());
}

bool SegInfo::Read(uint8_t* buf) {
  this->set_next_seg(be32toh(*(uint32_t*)(buf + 0)));
  this->set_timestamp(be64toh(*(uint64_t*)(buf + 16)));
  return true;
}

bool SegInfo::Write(uint8_t* buf) const {
  ::memset(buf, 0, this->kNoctet);
  *(uint32_t*)(buf + 0) = htobe32(this->next_seg());
  *(uint64_t*)(buf + 16) = htobe64(this->timestamp());
  return true;
}


};//namespace lfs
