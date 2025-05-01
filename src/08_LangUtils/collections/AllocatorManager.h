#pragma once

#include <array>
#include <variant>

#include "VuAllocator.h"
#include "08_LangUtils/TypeDefs.h"


using AllocatorVariant = std::variant<DebugAllocator, InvalidAllocator>;

constinit static std::array<AllocatorVariant, 256> GLOBAL_ALLOCATOR_ARRAY =
    []() constexpr
    {
        std::array<AllocatorVariant, 256> allocators{};

        for (auto& allocator : allocators)
        {
            allocator = AllocatorVariant{InvalidAllocator{}};
        }

        allocators[1] = AllocatorVariant{DebugAllocator{}};

        return allocators;
    }();

// enum class AllocatorType : u8orNull
// {
//     Invalid  = 0,
//     Standard = 1,
// };

struct AllocatorHandle
{
    u8orNull index;

    static consteval AllocatorHandle Invalid() { return {}; }
    static consteval AllocatorHandle Default() { return {1_u8}; }
};

struct AllocatorManager
{
    // static void* allocate(AllocatorType allocator, size_t size)
    // {
    //     u8orNull        index = static_cast<u8orNull>(allocator);
    //     AllocatorHandle handle{index};
    //     return allocate(handle, size);
    // }
    //
    // static void deallocate(AllocatorType allocator, void* ptr)
    // {
    //     u8orNull        index = static_cast<u8orNull>(allocator);
    //     AllocatorHandle handle{index};
    //     return deallocate(handle, ptr);
    // }
    //
    // static void init(AllocatorType allocator)
    // {
    //     u8orNull        index = static_cast<u8orNull>(allocator);
    //     AllocatorHandle handle{index};
    //     init(handle);
    // }
    //
    // static void uninit(AllocatorType allocator)
    // {
    //     u8orNull        index = static_cast<u8orNull>(allocator);
    //     AllocatorHandle handle{index};
    //     uninit(handle);
    // }

    //////////////////////////////////////////////////////////////////////////
    static void* allocate(AllocatorHandle handle, size_t size)
    {
        return std::visit([size](auto& alloc) -> void* {
            return alloc.allocate(size);
        }, GLOBAL_ALLOCATOR_ARRAY[handle.index]);
    }

    static void deallocate(AllocatorHandle handle, void* ptr)
    {
        std::visit([ptr](auto& alloc)
        {
            alloc.deallocate(ptr);
        }, GLOBAL_ALLOCATOR_ARRAY[handle.index]);
    }

    static void init(AllocatorHandle handle)
    {
        std::visit([](auto& alloc)
        {
            alloc.init();
        }, GLOBAL_ALLOCATOR_ARRAY[handle.index]);
    }

    static void uninit(AllocatorHandle handle)
    {
        std::visit([](auto& alloc)
        {
            alloc.uninit();
        }, GLOBAL_ALLOCATOR_ARRAY[handle.index]);
    }
};
