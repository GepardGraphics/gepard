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

#ifndef GEPARD_VULKAN_INTERFACE_H
#define GEPARD_VULKAN_INTERFACE_H

#include "gepard-defs.h"

// TODO: move this define to the build system
#define VK_USE_PLATFORM_XLIB_KHR

#include <vulkan/vulkan.h>

namespace gepard {
namespace vulkan {

class GepardVulkanInterface {
public:
    GepardVulkanInterface(const char* libraryName);
    ~GepardVulkanInterface();

    void loadGlobalFunctions();
    void loadInstanceFunctions(const VkInstance instance);
    void loadDeviceFunctions(const VkDevice device);

#define GD_VK_DECLARE_FUNCTION(fun) PFN_##fun fun

    // Global level vulkan functions
    GD_VK_DECLARE_FUNCTION(vkGetInstanceProcAddr);
    GD_VK_DECLARE_FUNCTION(vkCreateInstance);
    GD_VK_DECLARE_FUNCTION(vkEnumerateInstanceExtensionProperties);

    // Instance level vulkan functions
    GD_VK_DECLARE_FUNCTION(vkDestroyInstance);
    GD_VK_DECLARE_FUNCTION(vkEnumeratePhysicalDevices);
    GD_VK_DECLARE_FUNCTION(vkGetPhysicalDeviceProperties);
    GD_VK_DECLARE_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties);
    GD_VK_DECLARE_FUNCTION(vkCreateDevice);
    GD_VK_DECLARE_FUNCTION(vkGetDeviceProcAddr);
    GD_VK_DECLARE_FUNCTION(vkGetPhysicalDeviceMemoryProperties);
    GD_VK_DECLARE_FUNCTION(vkGetPhysicalDeviceFeatures);
    // WSI functions
    GD_VK_DECLARE_FUNCTION(vkDestroySurfaceKHR);
#ifdef VK_USE_PLATFORM_XLIB_KHR
    GD_VK_DECLARE_FUNCTION(vkCreateXlibSurfaceKHR);
#endif // VK_USE_PLATFORM_XLIB_KHR
    GD_VK_DECLARE_FUNCTION(vkGetPhysicalDeviceSurfaceFormatsKHR);
    GD_VK_DECLARE_FUNCTION(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    GD_VK_DECLARE_FUNCTION(vkGetPhysicalDeviceSurfacePresentModesKHR);

    // Device level vulkan functions
    GD_VK_DECLARE_FUNCTION(vkDestroyDevice);
    GD_VK_DECLARE_FUNCTION(vkGetDeviceQueue);
    GD_VK_DECLARE_FUNCTION(vkCreateCommandPool);
    GD_VK_DECLARE_FUNCTION(vkResetCommandPool);
    GD_VK_DECLARE_FUNCTION(vkDestroyCommandPool);
    GD_VK_DECLARE_FUNCTION(vkAllocateCommandBuffers);
    GD_VK_DECLARE_FUNCTION(vkFreeCommandBuffers);
    GD_VK_DECLARE_FUNCTION(vkBeginCommandBuffer);
    GD_VK_DECLARE_FUNCTION(vkEndCommandBuffer);
    GD_VK_DECLARE_FUNCTION(vkQueueSubmit);
    GD_VK_DECLARE_FUNCTION(vkCreateRenderPass);
    GD_VK_DECLARE_FUNCTION(vkDestroyRenderPass);
    GD_VK_DECLARE_FUNCTION(vkCreateFramebuffer);
    GD_VK_DECLARE_FUNCTION(vkDestroyFramebuffer);
    GD_VK_DECLARE_FUNCTION(vkCreateImage);
    GD_VK_DECLARE_FUNCTION(vkDestroyImage);
    GD_VK_DECLARE_FUNCTION(vkCreateImageView);
    GD_VK_DECLARE_FUNCTION(vkDestroyImageView);
    GD_VK_DECLARE_FUNCTION(vkGetBufferMemoryRequirements);
    GD_VK_DECLARE_FUNCTION(vkGetImageMemoryRequirements);
    GD_VK_DECLARE_FUNCTION(vkBindBufferMemory);
    GD_VK_DECLARE_FUNCTION(vkBindImageMemory);
    GD_VK_DECLARE_FUNCTION(vkAllocateMemory);
    GD_VK_DECLARE_FUNCTION(vkFreeMemory);
    GD_VK_DECLARE_FUNCTION(vkCreateGraphicsPipelines);
    GD_VK_DECLARE_FUNCTION(vkDestroyPipeline);
    GD_VK_DECLARE_FUNCTION(vkCreateShaderModule);
    GD_VK_DECLARE_FUNCTION(vkDestroyShaderModule);
    GD_VK_DECLARE_FUNCTION(vkCreatePipelineLayout);
    GD_VK_DECLARE_FUNCTION(vkDestroyPipelineLayout);
    GD_VK_DECLARE_FUNCTION(vkCmdBeginRenderPass);
    GD_VK_DECLARE_FUNCTION(vkCmdEndRenderPass);
    GD_VK_DECLARE_FUNCTION(vkCreateBuffer);
    GD_VK_DECLARE_FUNCTION(vkDestroyBuffer);
    GD_VK_DECLARE_FUNCTION(vkMapMemory);
    GD_VK_DECLARE_FUNCTION(vkFlushMappedMemoryRanges);
    GD_VK_DECLARE_FUNCTION(vkInvalidateMappedMemoryRanges);
    GD_VK_DECLARE_FUNCTION(vkUnmapMemory);
    GD_VK_DECLARE_FUNCTION(vkCmdBindPipeline);
    GD_VK_DECLARE_FUNCTION(vkCmdDraw);
    GD_VK_DECLARE_FUNCTION(vkCmdDrawIndexed);
    GD_VK_DECLARE_FUNCTION(vkCmdDrawIndirect);
    GD_VK_DECLARE_FUNCTION(vkCmdDrawIndexedIndirect);
    GD_VK_DECLARE_FUNCTION(vkCreateFence);
    GD_VK_DECLARE_FUNCTION(vkDestroyFence);
    GD_VK_DECLARE_FUNCTION(vkWaitForFences);
    GD_VK_DECLARE_FUNCTION(vkCmdBindVertexBuffers);
    GD_VK_DECLARE_FUNCTION(vkCmdBindIndexBuffer);
    GD_VK_DECLARE_FUNCTION(vkCmdPipelineBarrier);
    GD_VK_DECLARE_FUNCTION(vkCmdCopyBuffer);
    GD_VK_DECLARE_FUNCTION(vkCmdCopyBufferToImage);
    GD_VK_DECLARE_FUNCTION(vkCmdCopyImageToBuffer);
    GD_VK_DECLARE_FUNCTION(vkCmdCopyImage);
    GD_VK_DECLARE_FUNCTION(vkCmdBlitImage);
    GD_VK_DECLARE_FUNCTION(vkCmdClearColorImage);
    GD_VK_DECLARE_FUNCTION(vkCmdClearAttachments);
    // WSI functions
    GD_VK_DECLARE_FUNCTION(vkCreateSwapchainKHR);
    GD_VK_DECLARE_FUNCTION(vkDestroySwapchainKHR);
    GD_VK_DECLARE_FUNCTION(vkGetSwapchainImagesKHR);
    GD_VK_DECLARE_FUNCTION(vkAcquireNextImageKHR);
    GD_VK_DECLARE_FUNCTION(vkQueuePresentKHR);

#undef GD_VK_DECLARE_FUNCTION

private:
    void* _vulkanLibrary;
};

} // namespace vulkan
} // namespace gepard

#endif // GEPARD_VULKAN_INTERFACE_H

#endif // USE_VULKAN
