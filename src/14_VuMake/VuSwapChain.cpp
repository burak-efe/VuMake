#include "VuSwapChain.h"

#include <algorithm>
#include <format>
#include <iostream>

#include "11_Config/VuCtx.h"

#include "VuDevice.h"

namespace Vu
{
    VuSwapChain::VuSwapChain(): vuDevice(nullptr), surface(nullptr), gBufferPass(), lightningPass(), colorFormat(),
                                colorSpace(), swapChainImageFormat()
    {
    }

    VuSwapChain::VuSwapChain(VuDevice* vuDevice, VkSurfaceKHR surface) : vuDevice{vuDevice}, surface{surface}
    {
        createSwapChain(surface);
        createImageViews(vuDevice->device);

        colorHnd =
            vuDevice->createImage(
                                  {
                                      .width = swapChainExtent.width,
                                      .height = swapChainExtent.height,
                                      .format = VK_FORMAT_R8G8B8A8_UNORM,
                                      .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                  }
                                 );

        normalHnd =
            vuDevice->createImage(
                                  {
                                      .width = swapChainExtent.width,
                                      .height = swapChainExtent.height,
                                      .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                                      .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                  }
                                 );

        armHnd =
            vuDevice->createImage(
                                  {
                                      .width = swapChainExtent.width,
                                      .height = swapChainExtent.height,
                                      .format = VK_FORMAT_R32G32B32A32_SFLOAT,
                                      .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
                                      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                                  }
                                 );

        depthStencilHnd =
            vuDevice->createImage(
                                  {
                                      .width = swapChainExtent.width,
                                      .height = swapChainExtent.height,
                                      .format = VK_FORMAT_D32_SFLOAT,
                                      .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                      .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                                  }
                                 );

        VuImage* dsImage = depthStencilHnd.get();

        gBufferPass.initAsGBufferPass(vuDevice->device,
                                      VK_FORMAT_R8G8B8A8_UNORM,
                                      VK_FORMAT_R32G32B32A32_SFLOAT,
                                      VK_FORMAT_R32G32B32A32_SFLOAT,
                                      dsImage->lastCreateInfo.format);

        lightningPass.initAsLightningPass(vuDevice->device, swapChainImageFormat);

        createFramebuffers();
    }

    // void VuSwapChain::init(VuDevice* vuDevice, VkSurfaceKHR surface)
    // {
    //     this->vuDevice = vuDevice;
    //     this->surface  = surface;
    //
    //     createSwapChain(surface);
    //     createImageViews(vuDevice->device);
    //
    //     colorHnd =
    //         vuDevice->createImage(
    //                               {
    //                                   .width = swapChainExtent.width,
    //                                   .height = swapChainExtent.height,
    //                                   .format = VK_FORMAT_R8G8B8A8_UNORM,
    //                                   .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    //                                   .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
    //                               }
    //                              );
    //
    //     normalHnd =
    //         vuDevice->createImage(
    //                               {
    //                                   .width = swapChainExtent.width,
    //                                   .height = swapChainExtent.height,
    //                                   .format = VK_FORMAT_R32G32B32A32_SFLOAT,
    //                                   .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    //                                   .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
    //                               }
    //                              );
    //
    //     armHnd =
    //         vuDevice->createImage(
    //                               {
    //                                   .width = swapChainExtent.width,
    //                                   .height = swapChainExtent.height,
    //                                   .format = VK_FORMAT_R32G32B32A32_SFLOAT,
    //                                   .usage = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    //                                   .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
    //                               }
    //                              );
    //
    //     depthStencilHnd =
    //         vuDevice->createImage(
    //                               {
    //                                   .width = swapChainExtent.width,
    //                                   .height = swapChainExtent.height,
    //                                   .format = VK_FORMAT_D32_SFLOAT,
    //                                   .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
    //                                   .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
    //                               }
    //                              );
    //
    //     VuImage* dsImage = depthStencilHnd.getResource();
    //
    //     gBufferPass.initAsGBufferPass(vuDevice->device,
    //                                   VK_FORMAT_R8G8B8A8_UNORM,
    //                                   VK_FORMAT_R32G32B32A32_SFLOAT,
    //                                   VK_FORMAT_R32G32B32A32_SFLOAT,
    //                                   dsImage->lastCreateInfo.format);
    //
    //     lightningPass.initAsLightningPass(vuDevice->device, swapChainImageFormat);
    //
    //     createFramebuffers();
    // }

    void VuSwapChain::uninit()
    {
        for (auto imageView : swapChainImageViews)
        {
            vkDestroyImageView(vuDevice->device, imageView, nullptr);
        }
        for (auto framebuffer : lightningFrameBuffers)
        {
            vkDestroyFramebuffer(vuDevice->device, framebuffer, nullptr);
        }

        gBufferPass.uninit();
        //depthStencilHnd.destroyHandle();
        vkDestroySwapchainKHR(vuDevice->device, swapChain, nullptr);
    }


    VkSurfaceFormatKHR VuSwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats)
    {
        ZoneScoped;
        for (const auto& availableFormat : availableFormats)
        {
            if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB
                && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            {
                return availableFormat;
            }
        }
        return availableFormats[0];
    }

    VkPresentModeKHR VuSwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes)
    {
        //TODO present mode
        // for (const auto& availablePresentMode: availablePresentModes) {
        //     if (availablePresentMode == VK_PRESENT_MODE_FIFO_KHR) {
        //         return availablePresentMode;
        //     }
        // }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    VkExtent2D VuSwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities)
    {
        if (capabilities.currentExtent.width != std::numeric_limits<u32>::max())
        {
            return capabilities.currentExtent;
        }
        int width, height;
        SDL_GetWindowSize(ctx::window, &width, &height);
        std::cout << "width: " << width << std::endl;
        VkExtent2D actualExtent = {
            static_cast<u32>(width),
            static_cast<u32>(height)
        };
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width,
                                        capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height,
                                         capabilities.maxImageExtent.height);
        return actualExtent;
    }

    QueueFamilyIndices VuSwapChain::findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
    {
        QueueFamilyIndices indices;
        u32                queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        int i = 0;

        for (const auto& queuefamily : queueFamilies)
        {
            if (queuefamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport)
            {
                indices.presentFamily = i;
            }
            if (indices.isComplete())
            {
                break;
            }
            i++;
        }
        return indices;
    }

    void VuSwapChain::resetSwapChain(VkSurfaceKHR surface)
    {
        for (auto imageView : swapChainImageViews)
        {
            vkDestroyImageView(vuDevice->device, imageView, nullptr);
        }
        for (auto framebuffer : lightningFrameBuffers)
        {
            vkDestroyFramebuffer(vuDevice->device, framebuffer, nullptr);
        }

        //depthStencilH.uninit();
        vkDestroySwapchainKHR(vuDevice->device, swapChain, nullptr);
        createSwapChain(surface);
        createImageViews(vuDevice->device);
        //TODO reset resource inplace
        //depthStencilH.init(depthStencilH.lastCreateInfo);
        createFramebuffers();
    }

    void VuSwapChain::beginGBufferPass(VkCommandBuffer commandBuffer, u32 frameIndex)
    {
        std::array<VkClearValue, 4> clearValues{};
        clearValues[0].color        = {{0, 0, 0, 1.0f}};
        clearValues[1].color        = {{0, 0, 0, 1.0f}};
        clearValues[3].color        = {{0, 0, 0, 1.0f}};
        clearValues[3].depthStencil = {1.0f, 0};

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass        = gBufferPass.renderPass;
        renderPassInfo.framebuffer       = gPassFrameBuffers[frameIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChainExtent;
        renderPassInfo.clearValueCount   = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues      = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VuSwapChain::beginLightningPass(VkCommandBuffer commandBuffer, u32 frameIndex)
    {
        std::array<VkClearValue, 1> clearValues = {
            {
                {.color = {{0.0f, 0.0f, 0.f, 1.0f}}}
            }
        };

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass        = lightningPass.renderPass;
        renderPassInfo.framebuffer       = lightningFrameBuffers[frameIndex];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = swapChainExtent;
        renderPassInfo.clearValueCount   = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues      = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    }

    void VuSwapChain::endRenderPass(VkCommandBuffer commandBuffer)
    {
        vkCmdEndRenderPass(commandBuffer);
    }

    void VuSwapChain::createSwapChain(VkSurfaceKHR surfaceKHR)
    {
        SwapChainSupportDetails swapChainSupport =
            SwapChainSupportDetails::querySwapChainSupport(vuDevice->physicalDevice, surfaceKHR);
        VkExtent2D         extend        = chooseSwapExtent(swapChainSupport.capabilities);
        VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
        VkPresentModeKHR   presentMode   = chooseSwapPresentMode(swapChainSupport.presentModes);
        u32                imageCount    = swapChainSupport.capabilities.minImageCount;


        VkSwapchainCreateInfoKHR createInfo{};
        createInfo.sType            = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        createInfo.surface          = surfaceKHR;
        createInfo.minImageCount    = imageCount;
        createInfo.imageFormat      = surfaceFormat.format;
        createInfo.imageColorSpace  = surfaceFormat.colorSpace;
        createInfo.imageExtent      = extend;
        createInfo.imageArrayLayers = 1;
        createInfo.imageUsage       = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

        QueueFamilyIndices indices              = findQueueFamilies(vuDevice->physicalDevice, surfaceKHR);
        u32                queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        if (indices.graphicsFamily != indices.presentFamily)
        {
            createInfo.imageSharingMode      = VK_SHARING_MODE_CONCURRENT;
            createInfo.queueFamilyIndexCount = 2;
            createInfo.pQueueFamilyIndices   = queueFamilyIndices;
        }
        else
        {
            createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        }

        createInfo.preTransform   = swapChainSupport.capabilities.currentTransform;
        createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        createInfo.presentMode    = presentMode;
        createInfo.clipped        = VK_TRUE;
        createInfo.oldSwapchain   = VK_NULL_HANDLE;
        //
        {
            VkCheck(vkCreateSwapchainKHR(vuDevice->device, &createInfo, nullptr, &swapChain));
            Utils::giveDebugName(vuDevice->device, VK_OBJECT_TYPE_SWAPCHAIN_KHR, swapChain, "Swapcahin");
        }

        //
        {
            vkGetSwapchainImagesKHR(vuDevice->device, swapChain, &imageCount, nullptr);
            swapChainImages.resize(imageCount);
            vkGetSwapchainImagesKHR(vuDevice->device, swapChain, &imageCount, swapChainImages.data());
        }

        swapChainImageFormat = surfaceFormat.format;
        swapChainExtent      = extend;
    }

    void VuSwapChain::createImageViews(VkDevice device)
    {
        swapChainImageViews.resize(swapChainImages.size());

        for (size_t i = 0; i < swapChainImages.size(); i++)
        {
            VkImageViewCreateInfo createInfo{};
            createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            createInfo.image = swapChainImages[i];

            createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
            createInfo.format   = swapChainImageFormat;


            createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

            createInfo.subresourceRange.aspectMask     = VK_IMAGE_ASPECT_COLOR_BIT;
            createInfo.subresourceRange.baseMipLevel   = 0;
            createInfo.subresourceRange.levelCount     = 1;
            createInfo.subresourceRange.baseArrayLayer = 0;
            createInfo.subresourceRange.layerCount     = 1;

            VkCheck(vkCreateImageView(device, &createInfo, nullptr, &swapChainImageViews[i]));
        }
    }

    void VuSwapChain::createFramebuffers()
    {
        gPassFrameBuffers.resize(swapChainImageViews.size());
        for (size_t i = 0; i < swapChainImageViews.size(); i++)
        {
            std::array<VkImageView, 4> attachments = {
                colorHnd.get()->imageView,
                normalHnd.get()->imageView,
                armHnd.get()->imageView,
                depthStencilHnd.get()->imageView,
            };
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass      = gBufferPass.renderPass;
            framebufferInfo.attachmentCount = attachments.size();
            framebufferInfo.pAttachments    = attachments.data();
            framebufferInfo.width           = swapChainExtent.width;
            framebufferInfo.height          = swapChainExtent.height;
            framebufferInfo.layers          = 1;
            VkCheck(vkCreateFramebuffer(vuDevice->device, &framebufferInfo, nullptr, &gPassFrameBuffers[i]));
            auto name = std::format("gpass framebuffer [{}]", i);
            Utils::giveDebugName(vuDevice->device, VK_OBJECT_TYPE_FRAMEBUFFER, gPassFrameBuffers[i], name.c_str());
        }


        lightningFrameBuffers.resize(swapChainImageViews.size());
        for (size_t i = 0; i < swapChainImageViews.size(); i++)
        {
            std::array<VkImageView, 1> attachments = {
                swapChainImageViews[i],
            };
            VkFramebufferCreateInfo framebufferInfo{};
            framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebufferInfo.renderPass      = lightningPass.renderPass;
            framebufferInfo.attachmentCount = attachments.size();
            framebufferInfo.pAttachments    = attachments.data();
            framebufferInfo.width           = swapChainExtent.width;
            framebufferInfo.height          = swapChainExtent.height;
            framebufferInfo.layers          = 1;
            VkCheck(vkCreateFramebuffer(vuDevice->device, &framebufferInfo, nullptr, &lightningFrameBuffers[i]));
            auto name = std::format("framebuffer [{}]", i);
            Utils::giveDebugName(vuDevice->device, VK_OBJECT_TYPE_FRAMEBUFFER, lightningFrameBuffers[i], name.c_str());
        }
    }
}
