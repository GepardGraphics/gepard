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

#include "gepard-vulkan-interface.h"

#include <dlfcn.h>

namespace gepard {

namespace vulkan {

GepardVulkanInterface::GepardVulkanInterface(const char *libraryName)
{
    _vulkanLibrary = dlopen(libraryName, RTLD_LAZY);
}

GepardVulkanInterface::~GepardVulkanInterface()
{
    dlclose(_vulkanLibrary);
}

void GepardVulkanInterface::loadGlobalFunctions()
{
#define GD_VK_LOAD_FUNCTION(fun)\
    fun = (PFN_##fun) vkGetInstanceProcAddr (0, #fun); \
    GD_ASSERT(fun && "Couldn't load " #fun "!");

    if (!_vulkanLibrary) {
            GD_CRASH("Loading the Vulkan library was unsuccessfuly!\n");
            return;
    }

    vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(_vulkanLibrary, "vkGetInstanceProcAddr");
    GD_ASSERT(vkGetInstanceProcAddr && "Couldn't load the vkGetInstanceProcAddr function!");

    GD_VK_LOAD_FUNCTION(vkCreateInstance);
    GD_VK_LOAD_FUNCTION(vkEnumerateInstanceExtensionProperties);

#undef GD_VK_LOAD_FUNCTION
}

void GepardVulkanInterface::loadInstanceFunctions(const VkInstance instance)
{
#define GD_VK_LOAD_FUNCTION(fun)\
    fun = (PFN_##fun) vkGetInstanceProcAddr (instance, #fun); \
    GD_ASSERT(fun && "Couldn't load " #fun "!");

    GD_VK_LOAD_FUNCTION(vkDestroyInstance);
    GD_VK_LOAD_FUNCTION(vkEnumeratePhysicalDevices);
    GD_VK_LOAD_FUNCTION(vkGetPhysicalDeviceProperties);
    GD_VK_LOAD_FUNCTION(vkGetPhysicalDeviceQueueFamilyProperties);
    GD_VK_LOAD_FUNCTION(vkCreateDevice);
    GD_VK_LOAD_FUNCTION(vkGetDeviceProcAddr);
    GD_VK_LOAD_FUNCTION(vkGetPhysicalDeviceMemoryProperties);
    GD_VK_LOAD_FUNCTION(vkGetPhysicalDeviceFeatures);

    // WSI functions
    GD_VK_LOAD_FUNCTION(vkDestroySurfaceKHR);
#ifdef VK_USE_PLATFORM_XLIB_KHR
    GD_VK_LOAD_FUNCTION(vkCreateXlibSurfaceKHR);
#endif // VK_USE_PLATFORM_XLIB_KHR
    GD_VK_LOAD_FUNCTION(vkGetPhysicalDeviceSurfaceFormatsKHR);
    GD_VK_LOAD_FUNCTION(vkGetPhysicalDeviceSurfaceCapabilitiesKHR);
    GD_VK_LOAD_FUNCTION(vkGetPhysicalDeviceSurfacePresentModesKHR);

#undef GD_VK_LOAD_FUNCTION
}

void GepardVulkanInterface::loadDeviceFunctions(const VkDevice device)
{
#define GD_VK_LOAD_FUNCTION(fun)\
    fun = (PFN_##fun) vkGetDeviceProcAddr (device, #fun); \
    GD_ASSERT(fun && "Couldn't load " #fun "!");

    GD_VK_LOAD_FUNCTION(vkDestroyDevice);
    GD_VK_LOAD_FUNCTION(vkGetDeviceQueue);
    GD_VK_LOAD_FUNCTION(vkCreateCommandPool);
    GD_VK_LOAD_FUNCTION(vkResetCommandPool);
    GD_VK_LOAD_FUNCTION(vkDestroyCommandPool);
    GD_VK_LOAD_FUNCTION(vkAllocateCommandBuffers);
    GD_VK_LOAD_FUNCTION(vkFreeCommandBuffers);
    GD_VK_LOAD_FUNCTION(vkBeginCommandBuffer);
    GD_VK_LOAD_FUNCTION(vkEndCommandBuffer);
    GD_VK_LOAD_FUNCTION(vkQueueSubmit);
    GD_VK_LOAD_FUNCTION(vkCreateRenderPass);
    GD_VK_LOAD_FUNCTION(vkDestroyRenderPass);
    GD_VK_LOAD_FUNCTION(vkCreateFramebuffer);
    GD_VK_LOAD_FUNCTION(vkDestroyFramebuffer);
    GD_VK_LOAD_FUNCTION(vkCreateImage);
    GD_VK_LOAD_FUNCTION(vkDestroyImage);
    GD_VK_LOAD_FUNCTION(vkCreateImageView);
    GD_VK_LOAD_FUNCTION(vkDestroyImageView);
    GD_VK_LOAD_FUNCTION(vkGetBufferMemoryRequirements);
    GD_VK_LOAD_FUNCTION(vkGetImageMemoryRequirements);
    GD_VK_LOAD_FUNCTION(vkBindBufferMemory);
    GD_VK_LOAD_FUNCTION(vkBindImageMemory);
    GD_VK_LOAD_FUNCTION(vkAllocateMemory);
    GD_VK_LOAD_FUNCTION(vkFreeMemory);
    GD_VK_LOAD_FUNCTION(vkCreateGraphicsPipelines);
    GD_VK_LOAD_FUNCTION(vkDestroyPipeline);
    GD_VK_LOAD_FUNCTION(vkCreateShaderModule);
    GD_VK_LOAD_FUNCTION(vkDestroyShaderModule);
    GD_VK_LOAD_FUNCTION(vkCreatePipelineLayout);
    GD_VK_LOAD_FUNCTION(vkDestroyPipelineLayout);
    GD_VK_LOAD_FUNCTION(vkCmdBeginRenderPass);
    GD_VK_LOAD_FUNCTION(vkCmdEndRenderPass);
    GD_VK_LOAD_FUNCTION(vkCreateBuffer);
    GD_VK_LOAD_FUNCTION(vkDestroyBuffer);
    GD_VK_LOAD_FUNCTION(vkMapMemory);
    GD_VK_LOAD_FUNCTION(vkFlushMappedMemoryRanges);
    GD_VK_LOAD_FUNCTION(vkInvalidateMappedMemoryRanges);
    GD_VK_LOAD_FUNCTION(vkUnmapMemory);
    GD_VK_LOAD_FUNCTION(vkCmdBindPipeline);
    GD_VK_LOAD_FUNCTION(vkCmdDraw);
    GD_VK_LOAD_FUNCTION(vkCmdDrawIndexed);
    GD_VK_LOAD_FUNCTION(vkCmdDrawIndirect);
    GD_VK_LOAD_FUNCTION(vkCmdDrawIndexedIndirect);
    GD_VK_LOAD_FUNCTION(vkCreateFence);
    GD_VK_LOAD_FUNCTION(vkDestroyFence);
    GD_VK_LOAD_FUNCTION(vkWaitForFences);
    GD_VK_LOAD_FUNCTION(vkCmdBindVertexBuffers);
    GD_VK_LOAD_FUNCTION(vkCmdBindIndexBuffer);
    GD_VK_LOAD_FUNCTION(vkCmdPipelineBarrier);
    GD_VK_LOAD_FUNCTION(vkCmdCopyBuffer);
    GD_VK_LOAD_FUNCTION(vkCmdCopyBufferToImage);
    GD_VK_LOAD_FUNCTION(vkCmdCopyImageToBuffer);
    GD_VK_LOAD_FUNCTION(vkCmdCopyImage);
    GD_VK_LOAD_FUNCTION(vkCmdBlitImage);
    GD_VK_LOAD_FUNCTION(vkCmdClearColorImage);
    GD_VK_LOAD_FUNCTION(vkCmdClearAttachments);

    // WSI functions
    GD_VK_LOAD_FUNCTION(vkCreateSwapchainKHR);
    GD_VK_LOAD_FUNCTION(vkDestroySwapchainKHR);
    GD_VK_LOAD_FUNCTION(vkGetSwapchainImagesKHR);
    GD_VK_LOAD_FUNCTION(vkAcquireNextImageKHR);
    GD_VK_LOAD_FUNCTION(vkQueuePresentKHR);

#undef GD_VK_LOAD_FUNCTION
}

} // namespace vulkan

} // namespace gepard

#endif // USE_VULKAN
