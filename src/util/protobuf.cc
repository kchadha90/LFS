#include "util/protobuf.h"
#include "util/endian.h"
namespace lfs {

bool pb_save(google::protobuf::MessageLite* msg, uint8_t* buf, size_t buflen) {
  assert(msg != NULL);
  assert(buf != NULL);
  uint32_t sz = msg->ByteSize();
  if (sizeof(uint32_t) + sz > buflen) return false;
  *(uint32_t*)buf = htobe32(sz);
  msg->SerializeWithCachedSizesToArray(buf + sizeof(uint32_t));
  return true;
}

void pb_save(google::protobuf::MessageLite* msg, charbuf* cb) {
  assert(msg != NULL);
  assert(cb != NULL);
  std::string s = msg->SerializeAsString();
  uint32_t sz = htobe32(s.size());
  cb->append((uint8_t*)(&sz), sizeof(sz));
  cb->append((uint8_t*)s.data(), s.size());
}

bool pb_load(google::protobuf::MessageLite* msg, const uint8_t* buf, size_t buflen) {
  assert(msg != NULL);
  assert(buf != NULL);
  uint32_t sz = be32toh(*(uint32_t*)buf);
  if (sizeof(uint32_t) + sz > buflen) return false;
  return msg->ParsePartialFromArray(buf + sizeof(uint32_t), sz);
}

bool pb_load(google::protobuf::MessageLite* msg, charbuf* cb) {
  assert(cb != NULL);
  return pb_load(msg, cb->data(), cb->size());
}

};//namespace lfs
