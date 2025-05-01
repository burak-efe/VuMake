#pragma once

#include <cstdint>
#include <cstring>
#include <iostream>

#include "08_LangUtils/TypeDefs.h"

/**
 * @brief A dynamic array-like container for elements of type T_Element.
 *
 * @tparam T_Element The type of elements stored in the list.
 */
template <typename T_Element>
struct VuList
{
    T_Element*      data            = nullptr;
    uint32_t        count           = 0;
    uint32_t        capacity        = 0;
    AllocatorHandle allocatorHandle = {0};

    /**
 * @brief Constructs an empty list with a specified allocator and initial capacity.
 *
 * @param allocatorHandle Handle to the allocator.
 * @param initialCapacity Initial capacity of the list.
 */
    VuList(uint32_t initialCapacity, AllocatorHandle allocatorHandle)
    {
        data                  = nullptr;
        count                 = 0;
        capacity              = 0;
        this->allocatorHandle = allocatorHandle;
        setCapacity(initialCapacity);
    }

    /**
     * @brief Subscript operator to access elements (non-const).
     *
     * @param index Index of the element to access.
     * @return T_Element& Reference to the element at the specified index.
     * @throws std::out_of_range If the index is out of bounds.
     */
    T_Element& operator[](uint32_t index)
    {
        if (index >= count)
        {
            throw std::out_of_range("Index out of bounds");
        }
        return data[index];
    }

    /**
     * @brief Subscript operator to access elements (const).
     *
     * @param index Index of the element to access.
     * @return const T_Element& Const reference to the element at the specified index.
     * @throws std::out_of_range If the index is out of bounds.
     */
    const T_Element& operator[](uint32_t index) const
    {
        if (index >= count)
        {
            throw std::out_of_range("Index out of bounds");
        }
        return data[index];
    }

    /**
     * @brief Adds a new element to the list.
     *
     * @param element The element to add.
     */
    void add(const T_Element& element)
    {
        if (count == capacity)
        {
            // Resize: Allocate new memory and move existing elements
            uint32_t newCapacity = count == 0 ? 2 : count * 2;
            setCapacity(newCapacity); // Double the size or start with 2
        }

        data[count] = element; // Add element at the end
        count++;
    }

    /**
    * @brief Removes an element from the list at a given index.
    *
    * @param index Index of the element to remove.
    * @throws std::out_of_range If the index is out of bounds.
    */
    void remove(uint32_t index)
    {
        if (index >= count)
        {
            throw std::out_of_range("Index out of bounds");
        }

        // Shift elements to remove the one at `index`
        std::memmove(data + index, data + index + 1, (count - index - 1) * sizeof(T_Element));
        count--;
    }

    void ensureCount(uint32_t newCount)
    {
        if (newCount > capacity)
        {
            ensureCapacity(newCount);
        }

        if (newCount > count)
        {
            for (uint32_t i = count; i < newCount; i++)
            {
                //data[i] = T_Element{};
                T_Element* unused =  new(&data[i]) T_Element{};
            }


            //std::memset(data + count, 0, (newCount - count) * sizeof(T_Element));
        }
        count = newCount;
    }

    void ensureCapacity(uint32_t newCapacity)
    {
        if (newCapacity > capacity)
        {
            setCapacity(newCapacity);
        }
    }

    //    /**
    // * @brief Sets the count of elements in the list.
    // *
    // * @param newCount The new count of elements.
    // */
    //    void setCount(uint32_t newCount)
    //    {
    //        if (newCount > capacity)
    //        {
    //            setCapacity(findNextCapacity(capacity, newCount));
    //        }
    //
    //        if (newCount > count)
    //        {
    //            // Initialize new elements to default value
    //            for (uint32_t i = count; i < newCount; ++i)
    //            {
    //                data[i] = T_Element{};
    //            }
    //        }
    //
    //        count = newCount;
    //    }

    /**
 * @brief Resizes the internal data array to a new capacity. It will trim if the new capacity is smaller than the current one.
 *
 * @param newCapacity The new capacity of the list.
 */
    void setCapacity(uint32_t newCapacity)
    {
        if (capacity == newCapacity)
        {
            return;
        }

        void*      newAllocation = AllocatorManager::allocate(allocatorHandle, newCapacity * sizeof(T_Element));
        T_Element* newData       = static_cast<T_Element*>(newAllocation);

        if (data != nullptr)
        {
            std::memcpy(newData, data, capacity * sizeof(T_Element));
            AllocatorManager::deallocate(allocatorHandle, data);
        }

        for (uint32_t i = capacity; i < newCapacity; i++)
        {
            T_Element* unused =  new(&newData[i]) T_Element{};
        }

        data     = newData;
        capacity = newCapacity;
    }

    /**
     * @brief Destructor to clean up memory.
     */
    void uninit()
    {
        if (data != nullptr)
        {
            AllocatorManager::deallocate(allocatorHandle, data);
        }
    }

private:
    u32 findNextCapacity(u32 currentCapacity, u32 newCount)
    {
        while (currentCapacity < newCount)
        {
            currentCapacity *= 2;
        }
        return currentCapacity;
    }
};
