// wujian@2018

// A simple memory pool to hold frequent used objects, such as Tokens

#ifndef HOLDER_H
#define HOLDER_H

#include <queue>
#include <set>
#include "decoder/common.h"

const Int32 chunk_size = 1024;

template <class T>
class Holder {
 public:
  Holder() { InitChunk(); }

  ~Holder() {
    for (T* ptr : holder_)
      if (ptr) delete[] ptr;
    for (Bool* ptr : free_map_)
      if (ptr) delete[] ptr;
  }

  void InitChunk() {
    T* object_ptr = new T[chunk_size];
    holder_.push_back(object_ptr);
    for (Int32 i = 0; i < chunk_size; i++) unused_.push(object_ptr + i);
    Bool* free_map = new Bool[chunk_size];
    for (Int32 i = 0; i < chunk_size; i++) free_map[i] = true;
    free_map_.push_back(free_map);
  }

  void Free(T* addr) {
    if (!IsFree(addr)) {
      unused_.push(addr);
      Set(addr, true);
    } else {
      LOG_WARN << "Double free for object " << addr;
    }
  }

  T* New() {
    if (!unused_.size()) InitChunk();
    T* head = Pop();
    Set(head, false);
    return head;
  }

  Int32 NumUnused() { return unused_.size(); }

  Int32 NumActive() { return holder_.size() * chunk_size - unused_.size(); }

 private:
  // Get a free object
  T* Pop() {
    ASSERT(unused_.size() && "Unused queue is empty");
    T* head = unused_.front();
    unused_.pop();
    return head;
  }

  void Index(T* addr, Int32* index, Int32* offset) {
    Bool check = false;
    for (Int32 i = 0; i < holder_.size(); i++) {
      *index = i, *offset = addr - holder_[i];
      if (*offset >= 0 && *offset < chunk_size) {
        check = true;
        break;
      }
    }
    if (!check) LOG_FAIL << "Seems some bugs existed";
  }

  Bool IsFree(T* addr) {
    Int32 index, offset;
    Index(addr, &index, &offset);
    return free_map_[index][offset];
  }

  void Set(T* addr, Bool state) {
    Int32 index, offset;
    Index(addr, &index, &offset);
    free_map_[index][offset] = state;
  }

  // Cache allocated chunk
  std::vector<T*> holder_;
  std::vector<Bool*> free_map_;
  // Record unused items
  std::queue<T*> unused_;
};

#endif