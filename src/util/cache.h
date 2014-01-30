#ifndef LFS_UTIL_CACHE_H_
#define LFS_UTIL_CACHE_H_
#include "util/defs.h"
#include <list>
#include <utility>
namespace lfs {

template <typename TKey, typename TValue>
class Cache {//LRU cache
  public:
    Cache(size_t capacity);
    ~Cache(void);
    TValue Get(TKey key, TValue default_value);
    void Put(TKey key, TValue value);
    void Remove(TKey key);
    
  protected:
    virtual void DestroyValue(TValue x) {}
    
  private:
    std::list<std::pair<TKey,TValue>> l_;//most recent in the front
    size_t capacity_;
    DISALLOW_COPY_AND_ASSIGN(Cache);
};

#define IN_LFS_UTIL_CACHE_INL_
#include "cache-inl.h"
#undef  IN_LFS_UTIL_CACHE_INL_

};//namespace lfs
#endif//LFS_UTIL_CACHE_H_
