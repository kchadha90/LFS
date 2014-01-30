#include "dir/file_dir.h"
#include "dir/physical.pb.h"
#include "util/protobuf.h"
namespace lfs {

FileDirectory::FileDirectory(void) {
  this->dirty_ = false;
}

bool FileDirectory::Open(File* file) {
  this->set_file(file);
  return this->Load();
}

bool FileDirectory::Load(void) {
  assert(this->file() != NULL);
  charbuf cb;
  if (!this->file()->Read(0, this->file()->inode()->size(), &cb)) return false;
  
  lfs::physical::DirFile p;
  if (!pb_load(&p, &cb)) return false;
  
  this->list_.clear();
  for (int i = 0; i < p.rec_size(); ++i) {
    lfs::physical::DirFileRec rec = p.rec(i);
    this->list_[rec.name()] = (ino_t)rec.ino();
  }
  return true;
}

bool FileDirectory::Save(void) {
  assert(this->file() != NULL);
  if (!this->dirty_) return true;
  
  lfs::physical::DirFile p;
  for (std::unordered_map<std::string,ino_t>::const_iterator it = this->list_.cbegin(); it != this->list_.cend(); ++it) {
    lfs::physical::DirFileRec* rec = p.add_rec();
    rec->set_ino(it->second);
    rec->set_name(it->first);
  }
  
  charbuf cb;
  pb_save(&p, &cb);
  if (!this->file()->Truncate(0)) return false;
  if (!this->file()->Write(0, cb.size(), cb.data())) return false;
  this->dirty_ = false;
  return true;
}

bool FileDirectory::Close(void) {
  if (this->file() == NULL) return true;
  if (!this->Save()) return false;
  this->set_file(NULL);
  return true;
}

bool FileDirectory::IsEmpty(void) {
  assert(this->file() != NULL);
  return this->list_.empty();
}

bool FileDirectory::List(std::vector<std::string>* names) {
  assert(this->file() != NULL);
  names->clear();
  for (std::unordered_map<std::string,ino_t>::const_iterator it = this->list_.cbegin(); it != this->list_.cend(); ++it) {
    names->push_back(it->first);
  }
  return true;
}

ino_t FileDirectory::Get(const std::string name) {
  assert(this->file() != NULL);
  std::unordered_map<std::string,ino_t>::const_iterator it = this->list_.find(name);
  if (it == this->list_.end()) return 0;
  return it->second;
}

bool FileDirectory::Add(ino_t ino, const std::string name) {
  assert(this->file() != NULL);
  if (this->list_.count(name) > 0) return false;
  this->list_[name] = ino;
  this->dirty_ = true;
  return true;
}

bool FileDirectory::Remove(const std::string name) {
  assert(this->file() != NULL);
  if (this->list_.erase(name) < 1) return false;
  this->dirty_ = true;
  return true;
}

};//namespace lfs
