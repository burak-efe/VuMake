#pragma once
#include "collections/AllocatorManager.h"
#include "collections/VuList.h"

namespace Vu
{
    struct ResourceCounters
    {
        //counter of total references
        u32 referenceCounter;
        //generation counter incremented whe de-allocation of that slot
        u16 generationCounter;
    };


    // Concept for types that have a member function uninit()
    template <typename T>
    concept Has_uninit = requires(T t)
    {
        { t.uninit() }; // Expression that must be valid
    };


    //Calls uninit when zero ref count IF T implements it
    template <typename T>
    struct VuResourcePool
    {
        VuList<T>                data;
        VuList<ResourceCounters> counters;
        VuList<u32>              freeList;
        u32                      poolCapacity;
        u32                      allocationCounter;
        u32                      freeListCounter;

        VuResourcePool(u32 capacity, AllocatorHandle allocatorHnd) :
            data(capacity, allocatorHnd),
            counters(capacity, allocatorHnd),
            freeList(capacity, allocatorHnd),
            poolCapacity(capacity),
            allocationCounter(0),
            freeListCounter(0)
        {
            data.ensureCount(capacity);
            counters.ensureCount(capacity);
            freeList.ensureCount(capacity);
        }


        PtrOrNull<T> getResource(u32 index, u32 generation)
        {
            T* result = nullptr;
            if (isGenerationMatch(index, generation))
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
            return poolCapacity - getUsedSlotCount();
        }

        //allocate, set refCount to one, and return index
        void allocate(u32& outIndex, u16& outGeneration)
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

            //data[newDataIndex]                      = T{};

            T* unused =  new(&data[newDataIndex]) T{};
            counters[newDataIndex].referenceCounter = 1;

            outIndex      = newDataIndex;
            outGeneration = counters[newDataIndex].generationCounter;
        }


        void increaseRefCount(u32 index, u32 generation)
        {
            if (!isGenerationMatch(index, generation))
            {
                throw std::runtime_error("increaseRefCount: generation mismatch!");
            }

            counters[index].referenceCounter += 1;
        }

        //Returns true if object reference count reached zero and deallocated.
        [[maybe_unused]] bool decreaseRefCount(u32 index, u32 generation)
        {
            if (!isGenerationMatch(index, generation))
            {
                throw std::runtime_error("decreaseRefCount: generation mismatch!");
            }

            bool isDeallocated = false;
            counters[index].referenceCounter -= 1;

            if (counters[index].referenceCounter == 0)
            {
                //delete
                if constexpr (Has_uninit<T>)
                {
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

        bool isGenerationMatch(u32 index, u16 generation)
        {
            return counters[index].generationCounter == generation;
        }
    };
}
