#pragma once
#include <memory_resource>
#include <mutex>
#include "TypeDefs.h"

struct IndexAllocator {
public:
private:
  vector<uint32_t> freeIndices = {};
  uint32_t         capacity    = {};
  uint32_t         nextIndex   = {};
  std::mutex       mtx         = {};

public:
  IndexAllocator();

  explicit IndexAllocator(const uint32_t             cap,
                          std::pmr::memory_resource* memoryResource = std::pmr::new_delete_resource());

  IndexAllocator(IndexAllocator&& other) noexcept;

  IndexAllocator&
  operator=(IndexAllocator&& other) noexcept;

  uint32_t
  allocate();

  void
  deallocate(uint32_t idx);
};
