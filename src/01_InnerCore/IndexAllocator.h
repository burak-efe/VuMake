#pragma once
#include <memory_resource>
#include <mutex>

struct IndexAllocator {
private:
  std::pmr::vector<uint32_t> m_freeIndices {};
  uint32_t                   m_capacity {};
  uint32_t                   m_nextIndex {};
  std::mutex                 m_mtx {};

public:
  IndexAllocator() = delete;

  explicit IndexAllocator(uint32_t cap, std::pmr::memory_resource* memoryResource = std::pmr::new_delete_resource());

  IndexAllocator(IndexAllocator&& other) noexcept;

  IndexAllocator&
  operator=(IndexAllocator&& other) noexcept;

  uint32_t
  allocate();

  void
  deallocate(uint32_t idx);
};
