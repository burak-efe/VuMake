#include "IndexAllocator.h"

IndexAllocator::IndexAllocator(const uint32_t cap, std::pmr::memory_resource* memoryResource) :
    freeIndices {memoryResource},
    capacity {cap},
    nextIndex {0} {
  freeIndices.reserve(capacity);
}

IndexAllocator::IndexAllocator(IndexAllocator&& other) noexcept :
    freeIndices {std::move(other.freeIndices)},
    capacity {other.capacity},
    nextIndex {other.nextIndex} {}

IndexAllocator&
IndexAllocator::operator=(IndexAllocator&& other) noexcept {
  if (this == &other) return *this;
  freeIndices = std::move(other.freeIndices);
  capacity    = other.capacity;
  nextIndex   = other.nextIndex;
  return *this;
}

uint32_t
IndexAllocator::allocate() {
  std::lock_guard<std::mutex> lock(mtx);
  if (!freeIndices.empty()) {
    uint32_t idx = freeIndices.back();
    freeIndices.pop_back();
    return idx;
  }
  if (nextIndex < capacity) { return nextIndex++; }
  throw std::runtime_error("IndexAllocator: capacity exhausted");
}

void
IndexAllocator::deallocate(uint32_t idx) {
  std::lock_guard<std::mutex> lock(mtx);

  if (idx >= capacity) { throw std::runtime_error("IndexAllocator: Trying to free a index out of range!"); }
  if (idx + 1 == nextIndex) {
    --nextIndex;
  } else {
    freeIndices.push_back(idx);
  }
}