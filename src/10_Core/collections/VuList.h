#pragma once

#include <cstdint>
#include <iostream>

#include "VuAllocator.h"

template <typename T_Element>
struct VuList
{
    T_Element* data            = nullptr;
    uint32_t   count           = 0;
    uint32_t   capacity        = 0;
    uint8_t    allocatorHandle = 0;

    VuList(uint8_t allocatorHandle)
    {
        data                  = nullptr;
        count                 = 0;
        capacity              = 0;
        this->allocatorHandle = allocatorHandle;
    }

    // Subscript operator (non-const)
    T_Element& operator[](uint32_t index)
    {
        if (index >= count)
        {
            std::cerr << "Index out of bounds" << std::endl;
            std::terminate(); // Crash if out of bounds, or handle it appropriately.
        }
        return data[index];
    }

    // Subscript operator (const)
    const T_Element& operator[](uint32_t index) const
    {
        if (index >= count)
        {
            std::cerr << "Index out of bounds" << std::endl;
            std::terminate();
        }
        return data[index];
    }

    // Add a new element to the list
    void add(const T_Element& element)
    {
        if (count == capacity)
        {
            // Resize: Allocate new memory and move existing elements
            uint32_t newCapacity = count == 0 ? 2 : count * 2;
            resize(newCapacity); // Double the size or start with 2
        }

        data[count] = element; // Add element at the end
        count++;
    }

    // Remove an element from the list at a given index
    void remove(uint32_t index)
    {
        if (index >= count)
        {
            std::cerr << "Index out of bounds" << std::endl;
            return;
        }

        // Shift elements to remove the one at `index`
        std::memmove(data + index, data + index + 1, (count - index - 1) * sizeof(T_Element));
        count--;
    }

    // Resize the internal data array
    void resize(uint32_t newCapacity)
    {
        IAllocator* allocator     = ALLOCATORS[allocatorHandle];
        void*       newAllocation = allocator->allocate(newCapacity * sizeof(T_Element));
        T_Element*  newData       = static_cast<T_Element*>(newAllocation);

        if (data != nullptr)
        {
            std::memcpy(newData, data, count * sizeof(T_Element));
            allocator->deallocate(data);
        }

        data     = newData;
        capacity = newCapacity;
    }

    // Destructor to clean up memory
    ~VuList()
    {
        if (data != nullptr)
        {
            IAllocator* allocator = ALLOCATORS[allocatorHandle];
            allocator->deallocate(data);
        }
    }
};
