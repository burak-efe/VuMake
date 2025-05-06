#pragma once

#include <vector>
#include <cstdint>
#include <memory_resource>
#include <mutex>
#include <stdexcept>

#include "TypeDefs.h"

class IndexAllocator
{
public:

private:
    vector<uint32_t> freeIndices{};
    uint32_t         capacity{};
    uint32_t         nextIndex{};
    std::mutex       mtx{};

public:
    IndexAllocator() = default;

    explicit IndexAllocator(const uint32_t             cap,
                            std::pmr::memory_resource* memoryResource = std::pmr::new_delete_resource())
        : freeIndices(memoryResource), capacity(cap), nextIndex(0)
    {
        freeIndices.reserve(capacity);
    }

    IndexAllocator(IndexAllocator&& other) noexcept
        : freeIndices{std::move(other.freeIndices)},
          capacity{other.capacity},
          nextIndex{other.nextIndex}
    {
    }

    IndexAllocator& operator=(IndexAllocator&& other) noexcept
    {
        if (this == &other)
            return *this;
        freeIndices = std::move(other.freeIndices);
        capacity    = other.capacity;
        nextIndex   = other.nextIndex;
        return *this;
    }

    uint32_t allocate()
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (!freeIndices.empty())
        {
            uint32_t idx = freeIndices.back();
            freeIndices.pop_back();
            return idx;
        }
        if (nextIndex < capacity)
        {
            return nextIndex++;
        }
        throw std::runtime_error("IndexAllocator: capacity exhausted");
    }

    void deallocate(uint32_t idx)
    {
        std::lock_guard<std::mutex> lock(mtx);

        if (idx >= capacity)
        {
            throw std::runtime_error("IndexAllocator: Trying to free a index out of range!");
        }
        if (idx + 1 == nextIndex)
        {
            --nextIndex;
        }
        else
        {
            freeIndices.push_back(idx);
        }
    }
};
