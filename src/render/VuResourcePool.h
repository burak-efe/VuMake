#pragma once
#include <stack>

#include "Common.h"

namespace Vu {
    template<typename T>
    struct VuResourcePool {
        std::vector<T> pool;
        std::stack<uint32> freeList;

        uint32 Insert(T obj) {
            pool.push_back(obj);
            return pool.size() - 1;
        }
    };
}
