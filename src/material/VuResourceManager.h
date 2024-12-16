#pragma once

#include "VuBuffer.h"


namespace Vu {
    struct VuBindlessConfigInfo;

    //TObj should implement uninit()
    //this class is not incrementing or decrementing refCount itselfs, object that receives(or sends it) should do that instead
    template<typename TObj>
    struct VuPool {
        inline static std::vector<TObj> data;
        inline static std::stack<uint32> freeList;
        inline static std::vector<uint32> refCounts;

        static uint32 getUsedSlotCount() {
            return data.size() - freeList.size();
        }

        //allocate, set refCount to one, and return index
        static uint32 allocate() {
            if (!freeList.empty()) {
                uint32 i = freeList.top();
                freeList.pop();
                refCounts[i] = 0;
                return i;
            }
            data.push_back(TObj{});
            refCounts.push_back(1);
            return data.size() - 1;
        }

        static void increaseRefCount(uint32 index) {
            refCounts[index] += 1;
        }

        static void decreaseRefCount(uint32 index) {
            refCounts[index] -= 1;

            if (refCounts[index] == 0) {
                //delete
                freeList.push(index);
                data[index].uninit();
            }
            if (refCounts[index] < 0) {
                std::cerr << "Referance count of object below zero" << std::endl;
            }
        }
    };

    template<typename T>
    struct VuHandle {
        uint32 index;

        //alloc a slot from pool and return the unitialized object
        T& createHandle() {
            index = VuPool<T>::allocate();
            return get();
        }

        //return true if reference count drops == 0, which meand you need to uninit the object
        void destroyHandle() {
            //return
            VuPool<T>::decreaseRefCount(index);
        }

        // void increaseRefCount() {
        //     VuPool<T>::increaseRefCount(index);
        // }

        ////return true if reference count drops below 1, which meany you need to uninit the object
        // VkBool32 decreaseRefCount() {
        //     return VuPool<T>::decreaseRefCount(index);
        // }

        T& get() const {
            return VuPool<T>::data.at(index);
        }
    };


    ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    struct VuResourceManager {
    private:
        inline static VuBuffer bufferOfStorageBuffer;
        inline static VuBuffer bufferOfUniformBuffer;

    public:
        static void init(const VuBindlessConfigInfo& info);

        static void uninit();

        ////////////////////////////////////////////////////////////////////////////////////////////////////////////////

        static void registerStorageBuffer(uint32 writeIndex, const VuBuffer& buffer);

        static void registerUniformBuffer(uint32 writeIndex, const VuBuffer& buffer);

        static void writeStorageBuffer(const VuBuffer& buffer, uint32 binding);

        static void writeSampledImageToGlobalPool(uint32 writeIndex, const VkImageView& imageView);

        static void writeSamplerToGlobalPool(uint32 writeIndex, const VkSampler& sampler);

        static void writeUBO_ToGlobalPool(uint32 writeIndex, uint32 setIndex, const VuBuffer& buffer);
    };
}
