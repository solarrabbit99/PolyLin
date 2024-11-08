#pragma once

#include <vector>

#define MEM_ALLOC_SIZE 64

namespace polylin {

// Represents non-shrinking memory_allocator for fast alloc and free
template <typename T>
class memory_allocator {
 protected:
  memory_allocator() : memory_allocator(MEM_ALLOC_SIZE) {}

  memory_allocator(size_t size)
      : size{size}, data{(T*)malloc(sizeof(T) * size)}, free_list{size} {
    for (int i = 0; i < size; ++i) free_list[i] = &(data.back()[i]);
  }

  ~memory_allocator() {
    for (T* block : data) std::free(block);
  }

  T* alloc() {
    [[unlikely]] if (free_list.empty()) {
      data.push_back((T*)malloc(sizeof(T) * size));
      free_list.resize(size);
      for (int i = 0; i < size; ++i) free_list[i] = &(data.back()[i]);
      size <<= 1;
    }

    T* top = free_list.back();
    free_list.pop_back();
    return top;
  }

  void free(T* slot) {
    slot->~T();
    free_list.push_back(slot);
  }

 private:
  std::vector<T*> data;
  std::vector<T*> free_list;
  size_t size;
};

};  // namespace polylin