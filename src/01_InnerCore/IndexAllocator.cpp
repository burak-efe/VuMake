#include "IndexAllocator.h"

#include <stdexcept>

IndexAllocator::IndexAllocator(const uint32_t cap, std::pmr::memory_resource* memoryResource) :
    m_freeIndices {memoryResource},
    m_capacity {cap},
    m_nextIndex {0} {
  m_freeIndices.reserve(m_capacity);
}

IndexAllocator::IndexAllocator(IndexAllocator&& other) noexcept :
    m_freeIndices {std::move(other.m_freeIndices)},
    m_capacity {other.m_capacity},
    m_nextIndex {other.m_nextIndex} {}

IndexAllocator&
IndexAllocator::operator=(IndexAllocator&& other) noexcept {
  if (this == &other) return *this;
  m_freeIndices = std::move(other.m_freeIndices);
  m_capacity    = other.m_capacity;
  m_nextIndex   = other.m_nextIndex;
  return *this;
}

uint32_t
IndexAllocator::allocate() {
  std::lock_guard<std::mutex> lock(m_mtx);
  if (!m_freeIndices.empty()) {
    uint32_t idx = m_freeIndices.back();
    m_freeIndices.pop_back();
    return idx;
  }
  if (m_nextIndex < m_capacity) { return m_nextIndex++; }
  throw std::runtime_error("IndexAllocator: capacity exhausted");
}

void
IndexAllocator::deallocate(uint32_t idx) {
  std::lock_guard<std::mutex> lock(m_mtx);

  if (idx >= m_capacity) { throw std::runtime_error("IndexAllocator: Trying to free a index out of range!"); }
  if (idx + 1 == m_nextIndex) {
    --m_nextIndex;
  } else {
    m_freeIndices.push_back(idx);
  }
}