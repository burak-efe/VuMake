#include "VuResourceManager.h"

#include "VuDevice.h"
#include "11_Config/VuConfig.h"
#include "11_Config/VuCtx.h"

// void Vu::VuResourceManager::init(const VuBindlessConfigInfo& info) {
//     //ZoneScoped;
//     bufferOfStorageBuffer.init({.length = info.storageBufferCount, .strideInBytes = sizeof(uint64)});
//     //bufferOfUniformBuffer.init({info.uniformBufferCount, sizeof(uint64)});
//
//     writeStorageBufferToDescriptor(bufferOfStorageBuffer, config::BINDLESS_CONFIG_INFO.storageBufferBinding);
//     //writeStorageBuffer(bufferOfUniformBuffer, config::BINDLESS_CONFIG_INFO.uniformBufferBinding);
// }
//
// void Vu::VuResourceManager::uninit() {
//     bufferOfStorageBuffer.uninit();
//     //bufferOfUniformBuffer.uninit();
// }
//
// void Vu::VuResourceManager::registerStorageBuffer(const VuBuffer& buffer, uint32 writeIndex) {
//     VkDeviceAddress address = buffer.getDeviceAddress();
//     bufferOfStorageBuffer.setData(&address, sizeof(VkDeviceAddress), writeIndex * sizeof(VkDeviceAddress) );
// }
//
// void Vu::VuResourceManager::writeStorageBufferToDescriptor(const VuBuffer& buffer, uint32 binding) {
//     VkDescriptorBufferInfo bufferInfo{};
//     bufferInfo.buffer = buffer.buffer;
//     bufferInfo.range = buffer.length * buffer.stride;
//
//     for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
//         VkWriteDescriptorSet descriptorWrite{};
//         descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//         descriptorWrite.dstSet = ctx::vuDevice->globalDescriptorSets[i];
//         descriptorWrite.dstBinding = binding;
//         descriptorWrite.dstArrayElement = 0;
//         descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
//         descriptorWrite.descriptorCount = 1;
//         descriptorWrite.pBufferInfo = &bufferInfo;
//         vkUpdateDescriptorSets(ctx::vuDevice->device, 1, &descriptorWrite, 0, nullptr);
//     }
// }
//
// void Vu::VuResourceManager::writeSampledImageToGlobalPool(const VkImageView& imageView, uint32 writeIndex) {
//
//     VkDescriptorImageInfo imageInfo{
//         .sampler = VK_NULL_HANDLE,
//         .imageView = imageView,
//         .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
//     };
//
//     for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
//         VkWriteDescriptorSet descriptorWrite{};
//         descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//         descriptorWrite.dstSet = ctx::vuDevice->globalDescriptorSets[i];
//         descriptorWrite.dstBinding = config::BINDLESS_CONFIG_INFO.sampledImageBinding;
//         descriptorWrite.dstArrayElement = writeIndex;
//         descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
//         descriptorWrite.descriptorCount = 1;
//         descriptorWrite.pImageInfo = &imageInfo;
//         vkUpdateDescriptorSets(ctx::vuDevice->device, 1, &descriptorWrite, 0, nullptr);
//     }
// }
//
// void Vu::VuResourceManager::writeSamplerToGlobalPool(const VkSampler& sampler, uint32 writeIndex) {
//
//     VkDescriptorImageInfo imageInfo{
//         .sampler = sampler,
//     };
//
//     for (size_t i = 0; i < config::MAX_FRAMES_IN_FLIGHT; i++) {
//         VkWriteDescriptorSet samplerWrite{};
//         samplerWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//         samplerWrite.dstSet = ctx::vuDevice->globalDescriptorSets[i];
//         samplerWrite.dstBinding = config::BINDLESS_CONFIG_INFO.samplerBinding;
//         samplerWrite.dstArrayElement = writeIndex;
//         samplerWrite.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
//         samplerWrite.descriptorCount = 1;
//         samplerWrite.pImageInfo = &imageInfo;
//         vkUpdateDescriptorSets(ctx::vuDevice->device, 1, &samplerWrite, 0, nullptr);
//     }
// }
//
// void Vu::VuResourceManager::writeUBO_ToGlobalPool(const VuBuffer& buffer, uint32 writeIndex, uint32 setIndex) {
//
//     VkDescriptorBufferInfo bufferInfo{
//         .buffer = buffer.buffer,
//         .offset = 0,
//         .range = sizeof(GPU_FrameConst)
//     };
//
//     VkWriteDescriptorSet descriptorWrite{};
//     descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
//     descriptorWrite.dstSet = ctx::vuDevice->globalDescriptorSets[setIndex];
//     descriptorWrite.dstBinding = 0;
//     descriptorWrite.dstArrayElement = 0;
//     descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
//     descriptorWrite.descriptorCount = 1;
//     descriptorWrite.pBufferInfo = &bufferInfo;
//     vkUpdateDescriptorSets(ctx::vuDevice->device, 1, &descriptorWrite, 0, nullptr);
// }
