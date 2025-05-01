#pragma once

#include <array>

#include "TypeDefs.h"
#include "VuPools.h"

constinit static std::array<void*, 256> pools{};

struct PoolHandle
{
    u8orNull index;
};

struct VuPoolManager
{
    static void registerPoolToGlobalArray(PoolHandle handle, void* pool)
    {
        pools[handle.index] = pool;
    }

    static void* getPoolPtr(PoolHandle handle)
    {
        if (handle.index == 0)
        {
            throw std::invalid_argument("Invalid pool index! Index 0 is reserved for the invalid pool.");
        }

        if (pools[handle.index] == nullptr)
        {
            throw std::invalid_argument("Pool not registered! Please register the pool before using it.");
        }
        return pools[handle.index];
    }
};


template <typename T>
struct VuHnd
{
    u32 index;
    u16 generation;
    //index of the pool that owns the resource
    PoolHandle poolHandle;

    VuHnd& operator=(const VuHnd& other)     = default;
    VuHnd& operator=(VuHnd&& other) noexcept = default;
    VuHnd(VuHnd&& other) noexcept            = default;


    VuHnd() : index(0), generation(0), poolHandle{0}
    {
    }

    VuHnd(PoolHandle poolHandle)
    {
        this->poolHandle            = poolHandle;
        Vu::VuResourcePool<T>* pool = getPool();
        pool->allocate(index, generation);
    }

    VuHnd(const VuHnd& other)
    {
        initFromOther(other);
    };

    void initFromOther(const VuHnd& other)
    {
        if (other.getResource() == nullptr)
        {
            throw std::runtime_error("VuHnd copy constructor: other handle resource is null!");
        }
        poolHandle = other.poolHandle;
        index      = other.index;
        generation = other.generation;
        getPool()->increaseRefCount(index, generation);
    }


    //Handle functions
    PtrOrNull<T> getResource() const
    {
        Vu::VuResourcePool<T>* pool = getPool();
        return pool->getResource(index, generation);
    }

    //return true if reference count drops == 0
    bool destroyHandle()
    {
        Vu::VuResourcePool<T>* pool = getPool();
        return pool->decreaseRefCount(index, generation);
    }

    Vu::VuResourcePool<T>* getPool() const
    {
        return static_cast<Vu::VuResourcePool<T>*>(VuPoolManager::getPoolPtr({poolHandle}));
    }
};

static_assert(sizeof(VuHnd<u32>) == 8, "VuHnd size must be 8 bytes");
