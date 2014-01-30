//protobuf helper functions
#ifndef LFS_UTIL_PROTOBUFLEN_H_
#define LFS_UTIL_PROTOBUFLEN_H_
#include "util/defs.h"
#include <google/protobuf/message_lite.h>
namespace lfs {

//save a message to buf (at most buflen octets), first 32-bit is protobuf length
bool pb_save(google::protobuf::MessageLite* msg, uint8_t* buf, size_t buflen);
void pb_save(google::protobuf::MessageLite* msg, charbuf* cb);

//read a message from buf (at most buflen octets), first 32-bit is protobuf length
bool pb_load(google::protobuf::MessageLite* msg, const uint8_t* buf, size_t buflen);
bool pb_load(google::protobuf::MessageLite* msg, charbuf* cb);

};//namespace lfs
#endif//LFS_UTIL_PROTOBUFLEN_H_
