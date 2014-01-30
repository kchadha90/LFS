#ifdef  IN_LFS_UTIL_CACHE_INL_

template <typename TKey, typename TValue>
Cache<TKey,TValue>::Cache(size_t capacity) {
  this->capacity_ = capacity;
}

template <typename TKey, typename TValue>
Cache<TKey,TValue>::~Cache(void) {
}

template <typename TKey, typename TValue>
TValue Cache<TKey,TValue>::Get(TKey key, TValue default_value) {
  if (this->l_.empty()) return default_value;
  typename std::list<std::pair<TKey,TValue>>::iterator it;
  for (it = this->l_.begin(); it != this->l_.end(); ++it) {
    std::pair<TKey,TValue> p = *it;
    if (p.first == key) {
      TValue value = p.second;
      this->l_.erase(it);
      this->l_.push_front(std::make_pair(key, value));
      return value;
    }
  }
  return default_value;
}


template <typename TKey, typename TValue>
void Cache<TKey,TValue>::Put(TKey key, TValue value) {
  if (this->l_.size() >= this->capacity_) {
    this->DestroyValue(this->l_.back().second);
    this->l_.pop_back();
  }
  this->l_.push_front(std::make_pair(key, value));
}

template <typename TKey, typename TValue>
void Cache<TKey,TValue>::Remove(TKey key) {
  typename std::list<std::pair<TKey,TValue>>::iterator it;
  for (it = this->l_.begin(); it != this->l_.end(); ++it) {
    std::pair<TKey,TValue> p = *it;
    if (p.first == key) {
      this->DestroyValue(p.second);
      this->l_.erase(it);
      return;
    }
  }
}

#endif//IN_LFS_UTIL_CACHE_INL_
