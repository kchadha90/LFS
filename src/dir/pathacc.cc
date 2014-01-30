#include "dir/pathacc.h"
#include <cstring>
#include <libgen.h>
namespace lfs {

PathAccess::PathAccess(InodeStore* istore, Directory* dir) {
  assert(istore != NULL);
  assert(dir != NULL);
  this->set_istore(istore);
  this->set_dir(dir);
}

std::string PathAccess::GetDirName(const std::string path) {
  char* path_c = strdup(path.c_str());
  std::string v = ::dirname(path_c);
  free(path_c);
  return v;
}

std::string PathAccess::GetBaseName(const std::string path) {
  char* path_c = strdup(path.c_str());
  std::string v = ::basename(path_c);
  free(path_c);
  return v;
}

ino_t PathAccess::Resolve(const std::string path) {
  if (path.size() < 1 || path[0] != '/') return 0;
  if (path.size() == 1) return Inode::kIno_Root;
  
  ino_t ino_dir = this->Resolve(PathAccess::GetDirName(path));
  if (ino_dir == (ino_t)0) return 0;
  
  Inode inode_dir; inode_dir.set_ino(ino_dir);
  if (!this->istore()->Read(&inode_dir)) return 0;
  File file_dir(&inode_dir, this->log());
  if (!this->dir()->Open(&file_dir)) return 0;
  ino_t ino = this->dir()->Get(PathAccess::GetBaseName(path));
  this->dir()->Close();
  return ino;
}

};//namespace lfs
