#pragma once
#include <iostream>
#include <stack>
#include "Common.h"

namespace Vu
{
    template<typename  T>
    struct VuHandle2
    {
        uint32 index;
        uint32 generation;

        //T* get(){return nullptr;}
    };

    struct ResourceCounters2
    {
        //counter of total references
        uint32 referenceCounter;
        //generation counter incremented whe de-allocation of that slot
        uint32 generationCounter;
    };

    template <typename T, uint32 capacity>
    struct VuPool2
    {
        static_assert(std::is_member_function_pointer_v<decltype(&T::uninit)>, "T must implement uninit()");

        std::array<T, capacity>                 data{};
        std::array<ResourceCounters2, capacity> counters{};
        std::array<uint32, capacity>            freeList{};
        uint32                                  allocationCounter = 0;
        uint32                                  freeListCounter   = 0;

        //Handle functions
        T* get(const VuHandle2<T> handle)
        {
            return get(handle.index, handle.generation);
        }

        //alloc a slot from pool and return the unitialized object
        VuHandle2<T> createHandle()
        {
            VuHandle2<T> handle{};
            allocate(handle.index, handle.generation);
            return handle;
        }

        //return true if reference count drops == 0, which meand you need to uninit the object
        VkBool32 destroyHandle(const VuHandle2<T> handle)
        {
            return decreaseRefCount(handle.index);
        }


        // NULLABLE:  get the data that this handle points to, it can be null
        T* get(uint32 index, uint32 generation)
        {
            T* result = (T*)nullptr;
            if (generation == counters[index].generationCounter)
            {
                result = &data[index];
            }
            return result;
        }

        uint32 getUsedSlotCount() const
        {
            return allocationCounter - freeListCounter;
        }

        uint32 getFreeSlotCount() const
        {
            return capacity - getUsedSlotCount();
        }

        //allocate, set refCount to one, and return index
        void allocate(uint32& outIndex, uint32& outGeneration)
        {
            uint32 newDataIndex = 0;

            if (freeListCounter == 0)
            {
                //alloc with brand-new index
                allocationCounter += 1;
                newDataIndex = allocationCounter - 1;
            }
            else
            {
                //alloc with freelist index
                freeListCounter -= 1;
                newDataIndex = freeList[freeListCounter];
            }

            data[newDataIndex]                      = T{};
            counters[newDataIndex].referenceCounter = 1;

            outIndex      = newDataIndex;
            outGeneration = counters[newDataIndex].generationCounter;
        }


        void increaseRefCount(uint32 index)
        {
            counters[index].referenceCounter += 1;
        }

        //Returns true if object reference count reached zero and deallocated.
        VkBool32 decreaseRefCount(uint32 index)
        {
            VkBool32 isDeallocated = false;
            counters[index].referenceCounter -= 1;

            if (counters[index].referenceCounter == 0)
            {
                //delete
                data[index].uninit();
                counters[index].generationCounter += 1;
                isDeallocated = true;

                if ((allocationCounter - 1) == index)
                {
                    // data was on end of the allocations, no need to add to free list
                    allocationCounter -= 1;
                }
                else
                {
                    //add to free list
                    freeList[freeListCounter] = index;
                    freeListCounter += 1;
                }
            }

            assert(counters[index].referenceCounter != UINT32_MAX && "Ref counter underflow!");

            return isDeallocated;
        }
    };
}
