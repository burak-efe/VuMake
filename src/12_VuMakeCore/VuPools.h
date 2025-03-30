#pragma once

#include "10_Core/VuCommon.h"


namespace Vu
{
    template <typename T>
    struct VuHnd
    {
        u32 index;
        u32 generation;
    };

    struct ResourceCounters
    {
        //counter of total references
        u32 referenceCounter;
        //generation counter incremented whe de-allocation of that slot
        u32 generationCounter;
    };


    // Concept for types that have a member function uninit()
    template <typename T>
    concept Has_uninit = requires(T t) {
        { t.uninit() };  // Expression that must be valid
    };

    //Calls uninit when zero ref count IF T implements it
    template <typename T, u32 capacity>
    struct VuResourcePool
    {
        std::array<T, capacity>                 data{};
        std::array<ResourceCounters, capacity> counters{};
        std::array<u32, capacity>            freeList{};
        u32                                  allocationCounter = 0;
        u32                                  freeListCounter   = 0;

        //Handle functions
        T* getResource(const VuHnd<T> handle)
        {
            return get(handle.index, handle.generation);
        }

        //alloc a slot from pool and return the unitialized object
        VuHnd<T> createHandle()
        {
            VuHnd<T> handle{};
            allocate(handle.index, handle.generation);
            return handle;
        }

        //return true if reference count drops == 0, which meand you need to uninit the object
        bool destroyHandle(const VuHnd<T> handle)
        {
            return decreaseRefCount(handle.index);
        }


        // NULLABLE:  get the data that this handle points to, it can be null
        T* get(u32 index, u32 generation)
        {
            T* result = (T*)nullptr;
            if (generation == counters[index].generationCounter)
            {
                result = &data[index];
            }
            return result;
        }

        [[nodiscard]] u32 getUsedSlotCount() const
        {
            return allocationCounter - freeListCounter;
        }

        [[nodiscard]] u32 getFreeSlotCount() const
        {
            return capacity - getUsedSlotCount();
        }

        //allocate, set refCount to one, and return index
        void allocate(u32& outIndex, u32& outGeneration)
        {
            u32 newDataIndex = 0;

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


        void increaseRefCount(u32 index)
        {
            counters[index].referenceCounter += 1;
        }

        //Returns true if object reference count reached zero and deallocated.
        [[maybe_unused]] bool decreaseRefCount(u32 index)
        {
            bool isDeallocated = false;
            counters[index].referenceCounter -= 1;

            if (counters[index].referenceCounter == 0)
            {
                //delete
                if constexpr (Has_uninit<T>) {
                    data[index].uninit();
                }

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
