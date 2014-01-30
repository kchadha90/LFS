#include "dir/linear_dir.h"
#include <cstdio>
namespace lfs {

LinearRootDirectory::LinearRootDirectory(InodeStore* istore) {
  this->set_istore(istore);
  istore->set_linear_rootdir(this);
}

bool LinearRootDirectory::Open(File* file) {
  assert(file != NULL);
  this->set_file(file);
  return file->inode()->ino() == Inode::kIno_Root;
}

bool LinearRootDirectory::List(std::vector<std::string>* names) {
  names->clear();
  size_t map_size = this->file_inomap()->inode()->size();
  size_t chunk_size = this->file_inomap()->noctet_blk();
  ino_t ino = 0; charbuf chunk; char name[12];
  for (size_t chunk_base = 0; chunk_base < map_size; chunk_base += chunk_size) {
    chunk.clear();
    if (chunk_base + chunk_size > map_size) chunk_size = map_size - chunk_base;
    if (!this->file_inomap()->Read(chunk_base, chunk_size, &chunk)) return false;
    for (size_t pos = 0; pos < chunk_size; ++pos, ino+=8) {
      uint8_t octet = chunk[pos];
      for (int bit = 0; bit < 8; ++bit) {
        if ((octet & (0x01 << bit)) != 0) {
          int32_t name_number = (int32_t)ino + bit - Inode::kIno_Good;
          if (name_number > 0) {
            snprintf(name, 12, "%"PRId32, name_number);
            names->push_back(name);
          }
        }
      }
    }
  }
  return true;
}

ino_t LinearRootDirectory::Get(const std::string name) {
  int32_t num = atoll(name.c_str());
  if (num <= 0) return 0;
  char normalized[11]; snprintf(normalized, 11, "%"PRId32, num);
  if (0 != name.compare(normalized)) return 0;
  
  ino_t ino = Inode::kIno_Good + num;
  if (this->istore()->IsInUse(ino)) return ino;
  else return 0;
}

ino_t LinearRootDirectory::AllocateIno(mode_t mode, const std::string path) const {
  if (S_ISDIR(mode)) return 0;

  if (path.size() < 2 || path[0] != '/') return 0;
  int32_t num = atoll(path.substr(1).c_str());
  if (num <= 0) return 0;
  char normalized[12]; snprintf(normalized, 12, "/%"PRId32, num);
  if (0 != path.compare(normalized)) return 0;
  
  ino_t ino = Inode::kIno_Good + num;
  if (!this->istore()->IsInUse(ino)) return ino;
  else return 0;
}

ino_t LinearRootDirectory::TransformIno(ino_t ino) const {
  if (ino <= Inode::kIno_Good) {
    return 0xFFFFFF00 + ino;
  } else {
    return ino - Inode::kIno_Good;
  }
}

};//namespace lfs
