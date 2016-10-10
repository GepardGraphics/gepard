/* Copyright (C) 2016, Gepard Graphics
 * Copyright (C) 2016, Kristof Kosztyo <kkristof@inf.u-szeged.hu>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifdef GD_USE_VULKAN

#ifndef GEPARD_VULKAN_H
#define GEPARD_VULKAN_H

#include "gepard-defs.h"

#include "gepard.h"
#include "gepard-image.h"
#include "gepard-types.h"

#include "gepard-vulkan-interface.h"

#include <vector>

namespace gepard {

class Image;
class Surface;

namespace vulkan {

class GepardVulkan {
public:
    explicit GepardVulkan(GepardContext&);
    ~GepardVulkan();

    void fillRect(Float x, Float y, Float w, Float h);

private:
    GepardContext& _context;
    GepardVulkanInterface _vk;
    VkAllocationCallbacks* _allocator;
    VkInstance _instance;
    VkPhysicalDevice _physicalDevice;
    VkDevice _device;
    uint32_t _queueFamilyIndex;
    VkCommandPool _commandPool;
    std::vector<VkCommandBuffer> _primaryCommandBuffers;
    std::vector<VkCommandBuffer> _secondaryCommandBuffers;
    std::vector<VkDeviceMemory> _memoryAllocations;
    VkRenderPass _renderPass;
    VkFormat _imageFormat;
    VkImage _surfaceImage;
    VkImageView _frameBufferColorAttachmentImageView;
    VkFramebuffer _frameBuffer;
    VkPhysicalDeviceMemoryProperties _physicalDeviceMemoryProperties;
    VkPhysicalDeviceFeatures _physicalDeviceFeatures;
    VkSurfaceKHR _wsiSurface;
    VkSwapchainKHR _wsiSwapChain;
    std::vector<VkImage> _wsiSwapChainImages;

    void createDefaultInstance();
    void chooseDefaultPhysicalDevice();
    void chooseDefaultDevice();
    bool findGraphicsQueue(std::vector<VkPhysicalDevice> devices);
    void createCommandPool();
    void allocatePrimaryCommandBuffer();
    void createDefaultRenderPass();
    void createSurfaceImage();
    void createDefaultFrameBuffer();
    uint32_t getMemoryTypeIndex(VkMemoryRequirements memoryRequirements, VkMemoryPropertyFlags properties);
    void createSwapChain();
    void presentImage();
    void readImage();
};

} // namespace vulkan

typedef vulkan::GepardVulkan GepardEngineBackend;

} // namespace gepard

#endif // GEPARD_VULKAN_H

#endif // GD_USE_VULKAN
