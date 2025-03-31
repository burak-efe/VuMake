#pragma once
#include <array>
#include <iostream>
#include <variant>

#include "10_Core/VuLogger.h"

struct IAllocator
{
    virtual       ~IAllocator() = default;
    virtual void* allocate(size_t size) =0;
    virtual void  deallocate(void* ptr) =0;
    virtual void  init() =0;
    virtual void  uninit() =0;
};

class InvalidAllocator final : public IAllocator
{
public:
    void* allocate(size_t size) override
    {
        Vu::Logger::Error("An invalid allocator was used to allocate memory");
    }

    void deallocate(void* ptr) override
    {
        Vu::Logger::Error("An invalid allocator was used to deallocate memory");
    }

    void init() override
    {
        Vu::Logger::Error("An invalid allocator canot be initialized");
    }

    void uninit() override
    {
        Vu::Logger::Error("An invalid allocator canot be uninitialized");
    }
};

class DebugAllocator final : public IAllocator
{
    int64_t allocationCount = 0;

public:
    void* allocate(size_t sizeInBytes) override
    {
        void* ptr = std::malloc(sizeInBytes);
        if (ptr == nullptr)
        {
            std::cerr << "Failed to allocate " << sizeInBytes << " bytes" << std::endl;
        }
        else
        {
            std::cout << sizeInBytes << " bytes allocated" << std::endl;
        }
        allocationCount++;
        return ptr;
    }

    void deallocate(void* ptr) override
    {
        std::free(ptr);
        allocationCount--;
    }

    ~DebugAllocator() override = default;

    void init() override
    {
    }

    void uninit() override
    {
        if (allocationCount != 0)
        {
            std::cerr << "Memory leaked for " << allocationCount << " allocations" << std::endl;
        }
    }
};

using AllocatorVariant = std::variant<DebugAllocator, InvalidAllocator>;

constexpr static std::array<AllocatorVariant, 256> ALLOCATORS = []() constexpr
{
    std::array<AllocatorVariant, 256> allocators{};

    for (auto& allocator : allocators)
    {
        allocator = AllocatorVariant{InvalidAllocator()};
    }

    // Set DebugAllocator at index 1
    allocators[1] = DebugAllocator{};

    return allocators;
}();
