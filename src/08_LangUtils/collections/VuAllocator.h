#pragma once

#include <iostream>
#include <cstdlib>

#include "08_LangUtils/VuLogger.h"
#include "08_LangUtils/TypeDefs.h"

// struct IAllocator
// {
//     virtual       ~IAllocator() = default;
//     virtual void* allocate(size_t size) =0;
//     virtual void  deallocate(void* ptr) =0;
//     virtual void  init() =0;
//     virtual void  uninit() =0;
// };

class InvalidAllocator
{
public:
    void* allocate(size_t size)
    {
        Vu::Logger::Error("An invalid allocator was used to allocate memory");
        return nullptr;
    }

    void deallocate(void* ptr)
    {
        Vu::Logger::Error("An invalid allocator was used to deallocate memory");
    }

    void init()
    {
        Vu::Logger::Error("An invalid allocator cannot be initialized");
    }

    void uninit()
    {
        Vu::Logger::Error("An invalid allocator cannot be uninitialized");
    }
};

class DebugAllocator
{
    int64_t allocationCount = 0;

public:
    void* allocate(size_t sizeInBytes)
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

    void deallocate(void* ptr)
    {
        std::free(ptr);
        allocationCount--;
    }


    void init()
    {
    }

    void uninit()
    {
        if (allocationCount != 0)
        {
            std::cerr << "Memory leaked for " << allocationCount << " allocations" << std::endl;
        }
    }
};