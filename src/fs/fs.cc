#include "fs/fs.h"
//#include "dir/linear_dir.h"
#include "dir/file_dir.h"
#include "util/logging.h"
namespace lfs {

Filesystem::Filesystem(void) {
  this->log_ = NULL;
  this->istore_ = NULL;
  this->pa_ = NULL;
}

bool Filesystem::Open(Log* log) {
  assert(log != NULL);
  this->set_log(log);
  this->set_istore(this->create_istore());
  this->set_pa(this->create_pa());
  this->opens_.clear();
  return true;
}

bool Filesystem::Close(void) {
  this->set_pa(NULL);
  this->set_istore(NULL);
  this->set_log(NULL);
  return true;
}

Inode* Filesystem::GetInode(ino_t ino) {
  Filesystem::OpenFile* of = this->GetOpen(ino);
  if (of == NULL) return NULL;
  return of->inode();
}

void Filesystem::ReleaseInode(Inode* inode) {
  assert(inode != NULL);
  this->ReleaseOpen(inode->ino());
}

File* Filesystem::GetFile(ino_t ino) {
  Filesystem::OpenFile* of = this->GetOpen(ino);
  if (of == NULL) return NULL;
  return of->file();
}

void Filesystem::ReleaseFile(File* file) {
  assert(file != NULL);
  this->ReleaseInode(file->inode());
}

Directory* Filesystem::GetDirectory(ino_t ino) {
  Filesystem::OpenFile* of = this->GetOpen(ino);
  if (of == NULL) return NULL;
  Directory* dir = of->directory();
  if (dir == NULL) {
    this->ReleaseOpen(ino);
  }
  return dir;
}

void Filesystem::ReleaseDirectory(Directory* directory) {
  assert(directory != NULL);
  this->ReleaseFile(directory->file());
}

Filesystem::OpenFile* Filesystem::GetOpen(ino_t ino) {
  assert(ino != 0);
  Filesystem::OpenFile* of;
  std::map<ino_t, OpenFile*>::iterator it = this->opens_.find(ino);
  if (it == this->opens_.end()) {
    Inode* inode = new Inode(); inode->set_ino(ino);
    if (!this->istore()->Read(inode)) {
      delete inode;
      return NULL;
    }
    of = new Filesystem::OpenFile(this, inode);
    this->opens_[ino] = of;
  } else {
    of = it->second;
  }
  of->inc_refcount();
  return of;
}

void Filesystem::ReleaseOpen(ino_t ino) {
  assert(ino != 0);
  std::map<ino_t, OpenFile*>::iterator it = this->opens_.find(ino);
  if (it == this->opens_.end()) {
    Logging::Log(kLLWarn, kLCFs, "Filesystem::ReleaseOpen(%ju) not in map", (uintmax_t)ino);
    return;
  }
  Filesystem::OpenFile* of = it->second;
  Inode* inode = of->inode();
  of->dec_refcount();
  if (of->refcount() <= 0) {
    this->opens_.erase(it);
    if (inode->nlink() == 0) {
      of->file()->Truncate(0);//free file blocks
      bool isres = this->istore()->Delete(ino);
      if (!isres) {
        Logging::Log(kLLError, kLCFs, "Filesystem::ReleaseOpen(%ju) inode delete failure", (uintmax_t)ino);
      }
    }
    of->Close();
  }
  if (inode->nlink() > 0 && of->IsInodeChanged()) {
    bool isres = this->istore()->Write(inode);
    if (!isres) {
      Logging::Log(kLLError, kLCFs, "Filesystem::ReleaseOpen(%ju) inode write failure", (uintmax_t)ino);
    }
    of->SetInodeSaved();
  }
  if (of->refcount() <= 0) {
    delete of;
    delete inode;
  }
}

InodeStore* Filesystem::create_istore(void) {
  InodeStore* istore = new InodeStore();
  istore->Open(this->log());
  return istore;
}

PathAccess* Filesystem::create_pa(void) {
  return new PathAccess(this->istore(), this->create_directory());
}

Directory* Filesystem::create_directory(void) {
  //return new LinearRootDirectory(this->istore());
  return new FileDirectory();
}

void Filesystem::set_istore(InodeStore* value) {
  if (this->istore_ != NULL) {
    this->istore_->Close();
    delete this->istore_;
  }
  this->istore_ = value;
}

void Filesystem::set_pa(PathAccess* value) {
  if (this->pa_ != NULL) {
    delete this->pa_;
  }
  this->pa_ = value;
}

Filesystem::OpenFile::OpenFile(Filesystem* fs, Inode* inode) {
  assert(fs != NULL);
  assert(inode != NULL);
  this->refcount_ = 0;
  this->fs_ = fs;
  this->inode_ = inode;
  this->inode_orig_ = new Inode(); this->inode_orig_->CopyFrom(inode);
  this->file_ = NULL;
  this->directory_ = NULL;
}

Filesystem::OpenFile::~OpenFile(void) {
  delete this->inode_orig_;
}

void Filesystem::OpenFile::Close(void) {
  if (this->directory_ != NULL) {
    this->directory_->Close();
    delete this->directory_;
  }
  if (this->file_ != NULL) {
    delete this->file_;
  }
}

File* Filesystem::OpenFile::file(void) {
  if (this->file_ == NULL) {
    this->file_ = new File(this->inode(), this->fs()->log());
  }
  return this->file_;
}

Directory* Filesystem::OpenFile::directory(void) {
  if (this->directory_ == NULL && S_ISDIR(this->inode()->mode())) {
    this->directory_ = this->fs()->create_directory();
    this->directory_->Open(this->file());
  }
  return this->directory_;
}

bool Filesystem::OpenFile::IsInodeChanged(void) {
  return !this->inode()->Equals(this->inode_orig_);
}

void Filesystem::OpenFile::SetInodeSaved(void) {
  this->inode_orig_->CopyFrom(this->inode());
}

};//namespace lfs
