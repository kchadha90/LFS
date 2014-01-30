#include "log/segio.h"
#include <cstring>
namespace lfs {

SegIO::SegIO(Disk* disk, Superblock* superblock, SegNum cache_capacity) {
  assert(disk != NULL);
  assert(superblock != NULL);
  this->disk_ = disk;
  this->superblock_ = superblock;
  this->cache_ = new SegCache(cache_capacity);
  this->set_protect_seg0(true);
}

SegIO::~SegIO(void) {
  delete this->cache_;
}

bool SegIO::Read(SegNum seg, uint8_t* buf) {
  assert(buf != NULL);

  charbuf* c = this->ReadSeg(seg);
  if (c == NULL) return false;

  c->copy(buf, this->superblock()->noctet_seg());
  return true;
}

bool SegIO::ReadMulti(SegNum seg_start, SegNum seg_count, charbuf* cb) {
  for (SegNum i = 0; i < seg_count; ++i) {
    if (!this->Read(seg_start + i, 0, this->superblock()->noctet_seg(), cb)) return false;
  }
  return true;
}

bool SegIO::Read(SegNum seg, size_t start, size_t count, uint8_t* buf) {
  assert(buf != NULL);
  charbuf s;
  if (!this->Read(seg, start, count, &s)) return false;
  s.copy(buf, count);
  return true;
}

bool SegIO::Read(SegNum seg, size_t start, size_t count, charbuf* cb) {
  assert(cb != NULL);
  if (start + count > this->superblock()->noctet_seg()) return false;

  charbuf* c = this->ReadSeg(seg);
  if (c == NULL) return false;

  cb->append(*c, start, count);
  return true;
}

bool SegIO::Write(SegNum seg, const uint8_t* buf) {
  assert(buf != NULL);
  this->cache()->Remove(seg);
  if (seg == 0 && this->protect_seg0()) return false;
  if (!this->SeekTo(seg)) return false;
  return this->disk()->Write(this->superblock()->nsector_seg(), buf);
}

bool SegIO::WriteMulti(SegNum seg_start, SegNum seg_count, const uint8_t* buf) {
  for (SegNum i = 0; i < seg_count; ++i) {
    if (!this->Write(seg_start + i, buf + i * this->superblock()->noctet_seg())) return false;
  }
  return true;
}

bool SegIO::SeekTo(SegNum seg) {
  DiskSectorNum sector = seg * this->superblock()->nsector_seg();
  return this->disk()->Seek(sector);
}

charbuf* SegIO::ReadSeg(SegNum seg) {
  charbuf* c = this->cache()->Get(seg, NULL);
  if (c == NULL) {
    if (!this->SeekTo(seg)) return false;
    uint8_t* buf = (uint8_t*)malloc(this->superblock()->noctet_seg());
    if (buf == NULL) return NULL;
    if (!this->disk()->Read(this->superblock()->nsector_seg(), buf)) {
      free(buf);
      return NULL;
    }
    c = new charbuf(buf, this->superblock()->noctet_seg());
    free(buf);
    this->cache()->Put(seg, c);
  }
  return c;
}


};//namespace lfs
