#pragma once

#include "VuBuffer.h"


// namespace Vu
// {
//     struct VuBindlessConfigInfo;
//
//     struct VuResourceManager
//     {
//     private:
//         VuBuffer bufferOfStorageBuffer{};
//
//     public:
//         void init(const VuBindlessConfigInfo& info);
//
//         void uninit();
//
//         void writeStorageBufferToDescriptor(const VuBuffer& buffer, uint32 binding);
//
//         void registerStorageBuffer(const VuBuffer& buffer, uint32 writeIndex);
//
//         void writeSampledImageToGlobalPool(const VkImageView& imageView, uint32 writeIndex);
//
//         void writeSamplerToGlobalPool(const VkSampler& sampler, uint32 writeIndex);
//
//         void writeUBO_ToGlobalPool(const VuBuffer& buffer, uint32 writeIndex, uint32 setIndex);
//     };
// }
