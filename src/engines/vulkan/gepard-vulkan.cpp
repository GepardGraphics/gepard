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

#include "gepard-vulkan.h"

#include "gepard-types.h"

#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#ifdef VK_USE_PLATFORM_XLIB_KHR
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#endif // VK_USE_PLATFORM_XLIB_KHR

namespace gepard {
namespace vulkan {

GepardVulkan::GepardVulkan(GepardContext& context)
    : _context(context)
    , _vk("libvulkan.so")
    , _allocator(nullptr)
    , _instance(0)
    , _imageFormat(VK_FORMAT_R8G8B8A8_UNORM)
    , _wsiSurface(0)
    , _wsiSwapChain(0)
{
    GD_LOG1("GepardVulkan");
    _vk.loadGlobalFunctions();
    GD_LOG2(" - Global functions are loaded");
    createDefaultInstance();
    _vk.loadInstanceFunctions(_instance);
    GD_LOG2(" - Instance functions are loaded");
    chooseDefaultDevice();
    _vk.loadDeviceFunctions(_device);
    GD_LOG2(" - Device functions are loaded");
    createCommandPool();
    allocatePrimaryCommandBuffer();
    GD_LOG2(" - Command buffer is allocated");
    createDefaultRenderPass();
    GD_LOG2(" - Default render pass is created");
    createSurfaceImage();
    GD_LOG2(" - Surface backing image is created");
    createDefaultFrameBuffer();
    GD_LOG2(" - Default frame buffer is created");
    if (_context.surface->getDisplay())
        createSwapChain();
}

GepardVulkan::~GepardVulkan()
{
    // TODO: it would be extremely usefull to have container classes for these
    if (_frameBuffer) {
        _vk.vkDestroyFramebuffer(_device, _frameBuffer, _allocator);
    }
    if (_frameBufferColorAttachmentImageView) {
        _vk.vkDestroyImageView(_device, _frameBufferColorAttachmentImageView, _allocator);
    }
    if (_surfaceImage) {
        _vk.vkDestroyImage(_device, _surfaceImage, _allocator);
    }
    if (_renderPass) {
        _vk.vkDestroyRenderPass(_device, _renderPass, _allocator);
    }
    if (_secondaryCommandBuffers.size()) {
        _vk.vkFreeCommandBuffers(_device, _commandPool, _secondaryCommandBuffers.size(), _secondaryCommandBuffers.data());
    }
    if (_primaryCommandBuffers.size()) {
        _vk.vkFreeCommandBuffers(_device, _commandPool, _primaryCommandBuffers.size(), _primaryCommandBuffers.data());
    }
    for (uint32_t i = 0; i < _memoryAllocations.size(); i++) {
        _vk.vkFreeMemory(_device, _memoryAllocations[i], _allocator);
    }
    if (_commandPool) {
        _vk.vkDestroyCommandPool(_device, _commandPool, _allocator);
    }
    if (_wsiSwapChain) {
        _vk.vkDestroySwapchainKHR(_device, _wsiSwapChain, _allocator);
    }
    if (_device) {
        _vk.vkDestroyDevice(_device, _allocator);
    }
    if (_wsiSurface) {
        _vk.vkDestroySurfaceKHR(_instance, _wsiSurface, _allocator);
    }
    if (_instance) {
        _vk.vkDestroyInstance(_instance, _allocator);
    }
}

void GepardVulkan::fillRect(Float x, Float y, Float w, Float h)
{
    // Vertex data setup
    const float r = _context.currentState().fillColor.r;
    const float g = _context.currentState().fillColor.g;
    const float b = _context.currentState().fillColor.b;
    const float a = _context.currentState().fillColor.a;

    const float left = (float)((2.0 * x / (float)_context.surface->width()) - 1.0);
    const float right = (float)((2.0 * (x + w) / (float)_context.surface->width()) - 1.0);
    const float top = (float)((2.0 * y / (float)_context.surface->height()) - 1.0);
    const float bottom = (float)((2.0 * (y + h) / (float)_context.surface->height()) - 1.0);

    const float vertexData[] = {
        left, top, 1.0, 1.0, r, g, b, a,
        right, top, 1.0, 1.0, r, g, b, a,
        left, bottom, 1.0, 1.0, r, g, b, a,
        right, bottom, 1.0, 1.0, r, g, b, a,
    };

    const uint32_t rectIndicies[] = {0, 1, 2, 2, 1, 3};

    const VkBufferCreateInfo vertexBufferInfo = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,   // VkStructureType        sType;
        nullptr,                                // const void*            pNext;
        0,                                      // VkBufferCreateFlags    flags;
        (VkDeviceSize)sizeof(vertexData),       // VkDeviceSize           size;
        VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,      // VkBufferUsageFlags     usage;
        VK_SHARING_MODE_EXCLUSIVE,              // VkSharingMode          sharingMode;
        1u,                                     // uint32_t               queueFamilyIndexCount;
        &_queueFamilyIndex,                     // const uint32_t*        pQueueFamilyIndices;
    };

    VkBuffer vertexBuffer;
    VkDeviceMemory vertexBufferMemory;
    VkDeviceSize vertexBufferOffset = 0;

    _vk.vkCreateBuffer(_device, &vertexBufferInfo, _allocator, &vertexBuffer);

    // Upload vertex data.
    {
        VkMemoryRequirements memoryRequirements;
        _vk.vkGetBufferMemoryRequirements(_device, vertexBuffer, &memoryRequirements);
        const VkMemoryAllocateInfo allocationInfo = {
            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,                                         // VkStructureType    sType;
            nullptr,                                                                        // const void*        pNext;
            memoryRequirements.size,                                                        // VkDeviceSize       allocationSize;
            getMemoryTypeIndex(memoryRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT),    // uint32_t           memoryTypeIndex;
        };

        _vk.vkAllocateMemory(_device, &allocationInfo, _allocator, &vertexBufferMemory);
        _vk.vkBindBufferMemory(_device, vertexBuffer, vertexBufferMemory, vertexBufferOffset);

        void* data;
        _vk.vkMapMemory(_device, vertexBufferMemory, 0, (VkDeviceSize)sizeof(vertexData), 0, &data);

        std::memcpy(data, vertexData, (VkDeviceSize)sizeof(vertexData));

        const VkMappedMemoryRange range = {
            VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,  // VkStructureType    sType;
            nullptr,                                // const void*        pNext;
            vertexBufferMemory,                     // VkDeviceMemory     memory;
            0,                                      // VkDeviceSize       offset;
            (VkDeviceSize)sizeof(vertexData),       // VkDeviceSize       size;
        };

        _vk.vkFlushMappedMemoryRanges(_device, 1, &range);
        _vk.vkUnmapMemory(_device, vertexBufferMemory);
    }

    const VkBufferCreateInfo indexBufferInfo = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,   // VkStructureType        sType;
        nullptr,                                // const void*            pNext;
        0,                                      // VkBufferCreateFlags    flags;
        (VkDeviceSize)sizeof(rectIndicies),     // VkDeviceSize           size;
        VK_BUFFER_USAGE_INDEX_BUFFER_BIT,       // VkBufferUsageFlags     usage;
        VK_SHARING_MODE_EXCLUSIVE,              // VkSharingMode          sharingMode;
        1u,                                     // uint32_t               queueFamilyIndexCount;
        &_queueFamilyIndex,                      // const uint32_t*        pQueueFamilyIndices;
    };

    VkBuffer indexBuffer;
    VkDeviceMemory indexBufferMemory;

    _vk.vkCreateBuffer(_device, &indexBufferInfo, _allocator, &indexBuffer);

    // Upload index data.
    {
        VkMemoryRequirements memoryRequirements;
        _vk.vkGetBufferMemoryRequirements(_device, indexBuffer, &memoryRequirements);
        const VkMemoryAllocateInfo allocationInfo = {
            VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,                                         // VkStructureType    sType;
            nullptr,                                                                        // const void*        pNext;
            memoryRequirements.size,                                                        // VkDeviceSize       allocationSize;
            getMemoryTypeIndex(memoryRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT),    // uint32_t           memoryTypeIndex;
        };

        _vk.vkAllocateMemory(_device, &allocationInfo, _allocator, &indexBufferMemory);
        _vk.vkBindBufferMemory(_device, indexBuffer, indexBufferMemory, 0);

        void* data;
        _vk.vkMapMemory(_device, indexBufferMemory, 0, (VkDeviceSize)sizeof(rectIndicies), 0, &data);

        std::memcpy(data, rectIndicies, (VkDeviceSize)sizeof(rectIndicies));

        const VkMappedMemoryRange range = {
            VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,  // VkStructureType    sType;
            nullptr,                                // const void*        pNext;
            indexBufferMemory,                      // VkDeviceMemory     memory;
            0,                                      // VkDeviceSize       offset;
            (VkDeviceSize)sizeof(rectIndicies),     // VkDeviceSize       size;
        };

        _vk.vkFlushMappedMemoryRanges(_device, 1, &range);
        _vk.vkUnmapMemory(_device, indexBufferMemory);
    }

    // Pipeline creation

    VkShaderModule vertex;
    VkShaderModule fragment;

    // TODO: better file handling is needed.
    {
        std::ifstream vertexInput( "src/engines/vulkan/shaders/vert.spv", std::ios::binary );
        std::vector<char> vertexCode((std::istreambuf_iterator<char>(vertexInput)), (std::istreambuf_iterator<char>()));

        std::ifstream fragmentInput( "src/engines/vulkan/shaders/frag.spv", std::ios::binary );
        std::vector<char> fragmentCode((std::istreambuf_iterator<char>(fragmentInput)), (std::istreambuf_iterator<char>()));

        const VkShaderModuleCreateInfo vertexModulInfo = {
            VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,            // VkStructureType              sType;
            nullptr,                                                // const void*                  pNext;
            0,                                                      // VkShaderModuleCreateFlags    flags;
            vertexCode.size(),                                      // size_t                       codeSize;
            reinterpret_cast<const uint32_t*>(vertexCode.data()),   // const uint32_t*              pCode;
        };
        _vk.vkCreateShaderModule(_device, &vertexModulInfo, _allocator, &vertex);

        const VkShaderModuleCreateInfo fragmentModulInfo = {
            VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,            // VkStructureType              sType;
            nullptr,                                                // const void*                  pNext;
            0,                                                      // VkShaderModuleCreateFlags    flags;
            fragmentCode.size(),                                    // size_t                       codeSize;
            reinterpret_cast<const uint32_t*>(fragmentCode.data()), // const uint32_t*              pCode;
        };
        _vk.vkCreateShaderModule(_device, &fragmentModulInfo, _allocator, &fragment);
    }

    const VkPipelineShaderStageCreateInfo stages[] = {
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,    // VkStructureType                     sType;
            nullptr,                                                // const void*                         pNext;
            0,                                                      // VkPipelineShaderStageCreateFlags    flags;
            VK_SHADER_STAGE_VERTEX_BIT,                             // VkShaderStageFlagBits               stage;
            vertex,                                                 // VkShaderModule                      module;
            "main",                                                 // const char*                         pName;
            nullptr,                                                // const VkSpecializationInfo*         pSpecializationInfo;
        },
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,    // VkStructureType                     sType;
            nullptr,                                                // const void*                         pNext;
            0,                                                      // VkPipelineShaderStageCreateFlags    flags;
            VK_SHADER_STAGE_FRAGMENT_BIT,                           // VkShaderStageFlagBits               stage;
            fragment,                                               // VkShaderModule                      module;
            "main",                                                 // const char*                         pName;
            nullptr,                                                // const VkSpecializationInfo*         pSpecializationInfo;
        }
    };

    const VkVertexInputBindingDescription bindingDescription = {
        0u,                             // uint32_t             binding;
        2 * (4 * sizeof(float)),        // uint32_t             stride;
        VK_VERTEX_INPUT_RATE_VERTEX,    // VkVertexInputRate    inputRate;
    };

    const VkVertexInputAttributeDescription vertexAttributeDescriptions[] = {
        {
            0u,                             // uint32_t location
            0u,                             // uint32_t binding
            VK_FORMAT_R32G32B32A32_SFLOAT,  // VkFormat format
            0u,                             // uint32_t offset
        },
        {
            1u,                             // uint32_t location
            0u,                             // uint32_t binding
            VK_FORMAT_R32G32B32A32_SFLOAT,  // VkFormat format
            sizeof(float) * 4,              // uint32_t offset
        },
    };

    const VkPipelineVertexInputStateCreateInfo vertexInputState = {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,  // VkStructureType                             sType;
        nullptr,                                                    // const void*                                 pNext;
        0,                                                          // VkPipelineVertexInputStateCreateFlags       flags;
        1u,                                                         // uint32_t                                    vertexBindingDescriptionCount;
        &bindingDescription,                                        // const VkVertexInputBindingDescription*      pVertexBindingDescriptions;
        2u,                                                         // uint32_t                                    vertexAttributeDescriptionCount;
        vertexAttributeDescriptions,                                // const VkVertexInputAttributeDescription*    pVertexAttributeDescriptions;
    };

    const VkPipelineInputAssemblyStateCreateInfo inputAssemblyState = {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,    // VkStructureType                          sType
        nullptr,                                                        // const void*                              pNext
        0,                                                              // VkPipelineInputAssemblyStateCreateFlags  flags
        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,                            // VkPrimitiveTopology                      topology
        VK_FALSE,                                                       // VkBool32                                 primitiveRestartEnable
    };

    const uint32_t width = _context.surface->width();
    const uint32_t height = _context.surface->height();
    const VkViewport viewports[] = {
        {
            0.0f,           // float    x;
            0.0f,           // float    y;
            (float)width,   // float    width;
            (float)height,  // float    height;
            0.0f,           // float    minDepth;
            1.0f,           // float    maxDepth;
        }
    };

    const VkRect2D scissors[] = {
        {
            {
                0,      // int32_t x
                0,      // int32_t y
            },  // VkOffset2D    offset;
            {
                width,  // uint32_t    width
                height, // uint32_t    height
            }, // VkExtent2D    extent;
        }
    };

    const VkPipelineViewportStateCreateInfo viewportState = {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,  // VkStructureType                       sType;
        nullptr,                                                // const void*                           pNext;
        0,                                                      // VkPipelineViewportStateCreateFlags    flags;
        1u,                                                     // uint32_t                              viewportCount;
        viewports,                                              // const VkViewport*                     pViewports;
        1u,                                                     // uint32_t                              scissorCount;
        scissors,                                               // const VkRect2D*                       pScissors;
    };

    const VkPipelineRasterizationStateCreateInfo rasterizationState = {
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, // VkStructureType                            sType;
        nullptr,                                                    // const void*                                pNext;
        0,                                                          // VkPipelineRasterizationStateCreateFlags    flags;
        VK_FALSE,                                                   // VkBool32                                   depthClampEnable;
        VK_FALSE,                                                   // VkBool32                                   rasterizerDiscardEnable;
        VK_POLYGON_MODE_FILL,                                       // VkPolygonMode                              polygonMode;
        VK_CULL_MODE_NONE,                                          // VkCullModeFlags                            cullMode;
        VK_FRONT_FACE_COUNTER_CLOCKWISE,                            // VkFrontFace                                frontFace;
        VK_FALSE,                                                   // VkBool32                                   depthBiasEnable;
        0.0f,                                                       // float                                      depthBiasConstantFactor;
        0.0f,                                                       // float                                      depthBiasClamp;
        0.0f,                                                       // float                                      depthBiasSlopeFactor;
        1.0f,                                                       // float                                      lineWidth;
    };

    const VkPipelineMultisampleStateCreateInfo multisampleState = {
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,   // VkStructureType                          sType;
        nullptr,                                                    // const void*                              pNext;
        0,                                                          // VkPipelineMultisampleStateCreateFlags    flags;
        VK_SAMPLE_COUNT_1_BIT,                                      // VkSampleCountFlagBits                    rasterizationSamples;
        VK_FALSE,                                                   // VkBool32                                 sampleShadingEnable;
        0.0,                                                        // float                                    minSampleShading;
        nullptr,                                                    // const VkSampleMask*                      pSampleMask;
        VK_FALSE,                                                   // VkBool32                                 alphaToCoverageEnable;
        VK_FALSE,                                                   // VkBool32                                 alphaToOneEnable;
    };

    const VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {
        VK_TRUE,                                                                                                    // VkBool32                 blendEnable;
        VK_BLEND_FACTOR_SRC_ALPHA,                                                                                  // VkBlendFactor            srcColorBlendFactor;
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,                                                                        // VkBlendFactor            dstColorBlendFactor;
        VK_BLEND_OP_ADD,                                                                                            // VkBlendOp                colorBlendOp;
        VK_BLEND_FACTOR_SRC_ALPHA,                                                                                  // VkBlendFactor            srcAlphaBlendFactor;
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,                                                                        // VkBlendFactor            dstAlphaBlendFactor;
        VK_BLEND_OP_ADD,                                                                                            // VkBlendOp                alphaBlendOp;
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT   // VkColorComponentFlags    colorWriteMask;
    };

    const VkPipelineColorBlendStateCreateInfo colorBlendState = {
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,   // VkStructureType                               sType;
        nullptr,                                                    // const void*                                   pNext;
        0,                                                          // VkPipelineColorBlendStateCreateFlags          flags;
        VK_FALSE,                                                   // VkBool32                                      logicOpEnable;
        VK_LOGIC_OP_COPY,                                           // VkLogicOp                                     logicOp;
        1u,                                                         // uint32_t                                      attachmentCount;
        &colorBlendAttachmentState,                                 // const VkPipelineColorBlendAttachmentState*    pAttachments;
        {
            0.0f,   // float R
            0.0f,   // float G
            0.0f,   // float B
            0.0f,   // float A
        },                                                          // float                                         blendConstants[4];
    };

    VkPipelineLayout layout;
    const VkPipelineLayoutCreateInfo layoutCreateInfo = {
          VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,  // VkStructureType                sType
          nullptr,                                        // const void                    *pNext
          0,                                              // VkPipelineLayoutCreateFlags    flags
          0,                                              // uint32_t                       setLayoutCount
          nullptr,                                        // const VkDescriptorSetLayout   *pSetLayouts
          0,                                              // uint32_t                       pushConstantRangeCount
          nullptr                                         // const VkPushConstantRange     *pPushConstantRanges
    };

    _vk.vkCreatePipelineLayout(_device, &layoutCreateInfo, _allocator, &layout);

    const VkGraphicsPipelineCreateInfo pipelineCreateInfo = {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,    // VkStructureType                                  sType;
        nullptr,                                            // const void*                                      pNext;
        0,                                                  // VkPipelineCreateFlags                            flags;
        2u,                                                 // uint32_t                                         stageCount;
        stages,                                             // const VkPipelineShaderStageCreateInfo*           pStages;
        &vertexInputState,                                  // const VkPipelineVertexInputStateCreateInfo*      pVertexInputState;
        &inputAssemblyState,                                // const VkPipelineInputAssemblyStateCreateInfo*    pInputAssemblyState;
        nullptr,                                            // const VkPipelineTessellationStateCreateInfo*     pTessellationState;
        &viewportState,                                     // const VkPipelineViewportStateCreateInfo*         pViewportState;
        &rasterizationState,                                // const VkPipelineRasterizationStateCreateInfo*    pRasterizationState;
        &multisampleState,                                  // const VkPipelineMultisampleStateCreateInfo*      pMultisampleState;
        nullptr,                                            // const VkPipelineDepthStencilStateCreateInfo*     pDepthStencilState;
        &colorBlendState,                                   // const VkPipelineColorBlendStateCreateInfo*       pColorBlendState;
        nullptr,                                            // const VkPipelineDynamicStateCreateInfo*          pDynamicState;
        layout,                                             // VkPipelineLayout                                 layout;
        _renderPass,                                        // VkRenderPass                                     renderPass;
        0u,                                                 // uint32_t                                         subpass;
        VK_NULL_HANDLE,                                     // VkPipeline                                       basePipelineHandle;
        0,                                                  // int32_t                                          basePipelineIndex;
    };

    VkPipeline pipeline;

    _vk.vkCreateGraphicsPipelines(_device, VK_NULL_HANDLE, 1, &pipelineCreateInfo, _allocator, &pipeline);

    // Drawing

    const VkCommandBuffer commandBuffer = _primaryCommandBuffers[0];

    const VkCommandBufferBeginInfo commandBufferBeginInfo = {
       VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, // VkStructureType                          sType;
       nullptr,                                     // const void*                              pNext;
       VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, // VkCommandBufferUsageFlags                flags;
       nullptr,                                     // const VkCommandBufferInheritanceInfo*    pInheritanceInfo;
    };

    _vk.vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

    const VkClearValue clearValue = { 0.0, 0.0, 0.0, 0.0 };

    const VkRenderPassBeginInfo renderPassInfo {
        VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,   // VkStructureType        sType;
        nullptr,                                    // const void*            pNext;
        _renderPass,                                // VkRenderPass           renderPass;
        _frameBuffer,                               // VkFramebuffer          framebuffer;
        scissors[0],                                // VkRect2D               renderArea;
        1,                                          // uint32_t               clearValueCount;
        &clearValue,                                // const VkClearValue*    pClearValues;
    };

    _vk.vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
    _vk.vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

    _vk.vkCmdBindVertexBuffers(commandBuffer, 0, 1, &vertexBuffer, &vertexBufferOffset);
    _vk.vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);

    const uint32_t indexCount = sizeof(rectIndicies) / sizeof(uint32_t);
    _vk.vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);

    _vk.vkCmdEndRenderPass(commandBuffer);

    _vk.vkEndCommandBuffer(commandBuffer);

    const VkSubmitInfo submitInfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,  // VkStructureType                sType;
        nullptr,                        // const void*                    pNext;
        0,                              // uint32_t                       waitSemaphoreCount;
        nullptr,                        // const VkSemaphore*             pWaitSemaphores;
        nullptr,                        // const VkPipelineStageFlags*    pWaitDstStageMask;
        1,                              // uint32_t                       commandBufferCount;
        &commandBuffer,                 // const VkCommandBuffer*         pCommandBuffers;
        0,                              // uint32_t                       signalSemaphoreCount;
        nullptr,                        // const VkSemaphore*             pSignalSemaphores;
    };

    VkQueue queue;
    _vk.vkGetDeviceQueue(_device, _queueFamilyIndex, 0, &queue);

    const VkFenceCreateInfo fenceInfo = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,    // VkStructureType       sType;
        nullptr,                                // const void*           pNext;
        0,                                      // VkFenceCreateFlags    flags;
    };

    VkFence fence;
    _vk.vkCreateFence(_device, &fenceInfo, _allocator, &fence);

    _vk.vkQueueSubmit(queue, 1, &submitInfo, fence);

    const uint64_t oneMiliSec = 1000000;
    const uint64_t timeout = (uint64_t)16 * oneMiliSec; // 16 ms
    _vk.vkWaitForFences(_device, 1, &fence, VK_TRUE, timeout);

    if(_context.surface->getDisplay()) {
        presentImage();
    } else if(_context.surface->getBuffer()) {
        readImage();
    }

    // Clean up
    _vk.vkDestroyFence(_device, fence, _allocator);

    _vk.vkDestroyPipeline(_device, pipeline, _allocator);
    _vk.vkDestroyPipelineLayout(_device, layout, _allocator);

    _vk.vkDestroyShaderModule(_device, vertex, _allocator);
    _vk.vkDestroyShaderModule(_device, fragment, _allocator);

    _vk.vkDestroyBuffer(_device, vertexBuffer, _allocator);
    _vk.vkDestroyBuffer(_device, indexBuffer, _allocator);

    _vk.vkFreeMemory(_device, vertexBufferMemory, _allocator);
    _vk.vkFreeMemory(_device, indexBufferMemory, _allocator);
}

void GepardVulkan::createDefaultInstance()
{
    GD_ASSERT(!_instance);

    const std::vector<const char*> enabledExtensions = {
        VK_KHR_SURFACE_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_XLIB_KHR
        VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
#endif // VK_USE_PLATFORM_XLIB_KHR
    };

    // todo: find better default arguments
    const VkInstanceCreateInfo instanceCreateInfo = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO, // VkStructureType             sType;
        nullptr,                                // const void*                 pNext;
        0u,                                     // VkInstanceCreateFlags       flags;
        nullptr,                                // const VkApplicationInfo*    pApplicationInfo;
        0u,                                     // uint32_t                    enabledLayerCount;
        nullptr,                                // const char* const*          ppEnabledLayerNames;
        (uint32_t)enabledExtensions.size(),     // uint32_t                    enabledExtensionCount;
        enabledExtensions.data(),               // const char* const*          ppEnabledExtensionNames;
    };

    _vk.vkCreateInstance(&instanceCreateInfo, _allocator, &_instance);
    GD_ASSERT(_instance);
}

void GepardVulkan::chooseDefaultPhysicalDevice()
{
    uint32_t deviceCount;
    _vk.vkEnumeratePhysicalDevices(_instance, &deviceCount, nullptr);
    GD_ASSERT(deviceCount && "Couldn't find any device!");
    GD_LOG3("Physical devices found: " << deviceCount);
    std::vector<VkPhysicalDevice> physicalDevices(deviceCount);

    std::vector<VkPhysicalDevice> integratedDevices;
    std::vector<VkPhysicalDevice> discreteDevices;
    std::vector<VkPhysicalDevice> otherDevices;
    _vk.vkEnumeratePhysicalDevices(_instance, &deviceCount, physicalDevices.data());
    for (uint32_t i = 0; i < deviceCount; i++) {
           VkPhysicalDeviceProperties deviceProperties;
           _vk.vkGetPhysicalDeviceProperties(physicalDevices[i], &deviceProperties);
           GD_LOG3("device name " << deviceProperties.deviceName);
           GD_LOG3("api version " << (deviceProperties.apiVersion >> 22) << "."
               << ((deviceProperties.apiVersion >> 12) & 0x1ff) << "."  << (deviceProperties.apiVersion & 0x7ff));
           if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU) {
               integratedDevices.push_back(physicalDevices[i]);
               continue;
           } else if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
               discreteDevices.push_back(physicalDevices[i]);
               continue;
           }
           otherDevices.push_back(physicalDevices[i]);
    }

    if (findGraphicsQueue(discreteDevices))
        return;

    if (findGraphicsQueue(integratedDevices))
        return;

    if (findGraphicsQueue(otherDevices))
        return;

    GD_CRASH("Couldn't find any feasible vulkan device!");
}

bool GepardVulkan::findGraphicsQueue(std::vector<VkPhysicalDevice> devices)
{
    for (uint32_t i = 0; i < devices.size(); i++) {
        uint32_t queueCount;
        _vk.vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queueCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilyProperties(queueCount);
        _vk.vkGetPhysicalDeviceQueueFamilyProperties(devices[i], &queueCount, queueFamilyProperties.data());
        for (uint32_t j = 0; j < queueCount; j++) {
            if (queueFamilyProperties[j].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                _physicalDevice = devices[i];
                _queueFamilyIndex = j;
                return true;
            }
        }
    }

    return false;
}

void GepardVulkan::chooseDefaultDevice()
{
    chooseDefaultPhysicalDevice();
    _vk.vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &_physicalDeviceMemoryProperties);
    _vk.vkGetPhysicalDeviceFeatures(_physicalDevice, &_physicalDeviceFeatures);

    VkResult vkResult;
    const float queuePriorities[] = { 1.0f };

    const VkDeviceQueueCreateInfo deviceQueueCreateInfo = {
        VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO, // VkStructureType             sType;
        nullptr,                                    // const void*                 pNext;
        0u,                                         // VkDeviceQueueCreateFlags    flags;
        _queueFamilyIndex,                          // uint32_t                    queueFamilyIndex;
        1u,                                         // uint32_t                    queueCount;
        queuePriorities                             // const float*                pQueuePriorities;
    };

    std::vector<const char*> enabledInstanceLayers;
    std::vector<const char*> enabledInstanceExtensions;

    enabledInstanceLayers.push_back("VK_LAYER_LUNARG_standard_validation");

    if (_context.surface->getDisplay()) {
        enabledInstanceExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
    }

    const VkDeviceCreateInfo deviceCreateInfo = {
        VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,       // VkStructureType                  sType;
        nullptr,                                    // const void*                      pNext;
        0u,                                         // VkDeviceCreateFlags              flags;
        1u,                                         // uint32_t                         queueCreateInfoCount;
        &deviceQueueCreateInfo,                     // const VkDeviceQueueCreateInfo*   pQueueCreateInfos;
        (uint32_t)enabledInstanceLayers.size(),     // uint32_t                         enabledLayerCount;
        enabledInstanceLayers.data(),               // const char* const*               ppEnabledLayerNames;
        (uint32_t)enabledInstanceExtensions.size(), // uint32_t                         enabledExtensionCount;
        enabledInstanceExtensions.data(),           // const char* const*               ppEnabledExtensionNames;
        &_physicalDeviceFeatures,                   // const VkPhysicalDeviceFeatures*  pEnabledFeatures;
    };

    vkResult = _vk.vkCreateDevice(_physicalDevice, &deviceCreateInfo, _allocator, &_device);
    GD_ASSERT(vkResult == VK_SUCCESS && "Logical device creation is failed!");
}

void GepardVulkan::createCommandPool()
{
    const VkCommandPoolCreateInfo commandPoolCreateInfo = {
        VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,         // VkStructureType          sType;
        nullptr,                                            // const void*              pNext;
        VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,    // VkCommandPoolCreateFlags flags;
        _queueFamilyIndex,                                  // uint32_t                 queueFamilyIndex;
    };

    VkResult vkResult;
    vkResult = _vk.vkCreateCommandPool(_device, &commandPoolCreateInfo, _allocator, &_commandPool);

    GD_ASSERT(vkResult == VK_SUCCESS && "Command pool creation is failed!");
}

void GepardVulkan::allocatePrimaryCommandBuffer()
{
    const VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
        VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO, // VkStructureType         sType;
        nullptr,                                        // const void*             pNext;
        _commandPool,                                   // VkCommandPool           commandPool;
        VK_COMMAND_BUFFER_LEVEL_PRIMARY,                // VkCommandBufferLevel    level;
        1,                                              // uint32_t                commandBufferCount;
    };

    VkResult vkResult;
    VkCommandBuffer commandBuffer;
    vkResult = _vk.vkAllocateCommandBuffers(_device, &commandBufferAllocateInfo, &commandBuffer);

    GD_ASSERT(vkResult == VK_SUCCESS && "Command buffer allocation is failed!");
    _primaryCommandBuffers.push_back(commandBuffer);
}

void GepardVulkan::createDefaultRenderPass()
{
    VkResult vkResult;

    const VkAttachmentDescription attachmentDescription = {
        0u,                                         // VkAttachmentDescriptionFlags flags;
        _imageFormat,                               // VkFormat                     format;
        VK_SAMPLE_COUNT_1_BIT,                      // VkSampleCountFlagBits        samples;
        VK_ATTACHMENT_LOAD_OP_LOAD,                 // VkAttachmentLoadOp           loadOp; TODO: change it back.
        VK_ATTACHMENT_STORE_OP_STORE,               // VkAttachmentStoreOp          storeOp;
        VK_ATTACHMENT_LOAD_OP_DONT_CARE,            // VkAttachmentLoadOp           stencilLoadOp;
        VK_ATTACHMENT_STORE_OP_DONT_CARE,           // VkAttachmentStoreOp          stencilStoreOp;
        VK_IMAGE_LAYOUT_UNDEFINED,                  // VkImageLayout                initialLayout;
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,   // VkImageLayout                finalLayout;
    };

    const VkAttachmentReference colorAttachment = {
        0u,                                         // uint32_t         attachment;
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,   // VkImageLayout    layout;
    };

    const VkSubpassDescription subpassDescription = {
        0u,                                 // VkSubpassDescriptionFlags       flags;
        VK_PIPELINE_BIND_POINT_GRAPHICS,    // VkPipelineBindPoint             pipelineBindPoint;
        0u,                                 // uint32_t                        inputAttachmentCount;
        nullptr,                            // const VkAttachmentReference*    pInputAttachments;
        1u,                                 // uint32_t                        colorAttachmentCount;
        &colorAttachment,                   // const VkAttachmentReference*    pColorAttachments;
        nullptr,                            // const VkAttachmentReference*    pResolveAttachments;
        nullptr,                            // const VkAttachmentReference*    pDepthStencilAttachment;
        0u,                                 // uint32_t                        preserveAttachmentCount;
        nullptr,                            // const uint32_t*                 pPreserveAttachments;
    };

    const VkRenderPassCreateInfo renderPassCreateInfo = {
        VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,  // VkStructureType                   sType;
        nullptr,                                    // const void*                       pNext;
        0u,                                         // VkRenderPassCreateFlags           flags;
        1u,                                         // uint32_t                          attachmentCount;
        &attachmentDescription,                     // const VkAttachmentDescription*    pAttachments;
        1u,                                         // uint32_t                          subpassCount;
        &subpassDescription,                        // const VkSubpassDescription*       pSubpasses;
        0u,                                         // uint32_t                          dependencyCount;
        nullptr,                                    // const VkSubpassDependency*        pDependencies;
    };

    vkResult = _vk.vkCreateRenderPass(_device, &renderPassCreateInfo, _allocator, &_renderPass);
    GD_ASSERT(vkResult == VK_SUCCESS && "Creating the default render pass is failed!");
}

void GepardVulkan::createSurfaceImage()
{
    VkResult vkResult;

    const VkExtent3D imageSize = {
        _context.surface->width(),  // uint32_t    width;
        _context.surface->height(), // uint32_t    height;
        1,                          // uint32_t    depth;
    };

    const VkImageCreateInfo imageCreateInfo = {
        VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,        // VkStructureType          sType;
        nullptr,                                    // const void*              pNext;
        0u,                                         // VkImageCreateFlags       flags;
        VK_IMAGE_TYPE_2D,                           // VkImageType              imageType;
        _imageFormat,                               // VkFormat                 format;
        imageSize,                                  // VkExtent3D               extent;
        1u,                                         // uint32_t                 mipLevels;
        1u,                                         // uint32_t                 arrayLayers;
        VK_SAMPLE_COUNT_1_BIT,                      // VkSampleCountFlagBits    samples;
        VK_IMAGE_TILING_OPTIMAL,                    // VkImageTiling            tiling;
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT
            | VK_IMAGE_USAGE_TRANSFER_DST_BIT
            | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,  // VkImageUsageFlags        usage;
        VK_SHARING_MODE_EXCLUSIVE,                  // VkSharingMode            sharingMode;
        1u,                                         // uint32_t                 queueFamilyIndexCount;
        &_queueFamilyIndex,                         // const uint32_t*          pQueueFamilyIndices;
        VK_IMAGE_LAYOUT_UNDEFINED,                  // VkImageLayout            initialLayout;
    };

    vkResult = _vk.vkCreateImage(_device, &imageCreateInfo, _allocator, &_surfaceImage);
    GD_ASSERT(vkResult == VK_SUCCESS && "Creating the surface backing image is failed!");

    VkMemoryRequirements memoryRequirements;
    _vk.vkGetImageMemoryRequirements(_device, _surfaceImage, &memoryRequirements);

    const VkMemoryAllocateInfo memoryAllocateInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,                                         // VkStructureType    sType;
        nullptr,                                                                        // const void*        pNext;
        memoryRequirements.size,                                                        // VkDeviceSize       allocationSize;
        getMemoryTypeIndex(memoryRequirements, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT),    // uint32_t           memoryTypeIndex;
    };

    VkDeviceMemory deviceMemory;
    vkResult = _vk.vkAllocateMemory(_device, &memoryAllocateInfo, _allocator, &deviceMemory);
    GD_ASSERT(vkResult == VK_SUCCESS && "Memory allocation is failed!");

    _memoryAllocations.push_back(deviceMemory);

    vkResult = _vk.vkBindImageMemory(_device, _surfaceImage, deviceMemory, static_cast<VkDeviceSize>(0u));
    GD_ASSERT(vkResult == VK_SUCCESS && "Memory bind is failed!");

    // Clear the surface image

    const VkCommandBuffer commandBuffer = _primaryCommandBuffers[0];

    const VkCommandBufferBeginInfo commandBufferBeginInfo = {
       VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, // VkStructureType                          sType;
       nullptr,                                     // const void*                              pNext;
       VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, // VkCommandBufferUsageFlags                flags;
       nullptr,                                     // const VkCommandBufferInheritanceInfo*    pInheritanceInfo;
    };

    const VkClearColorValue clearColor = {
        0.0, 0.0, 0.0, 0.0
    };

    const VkImageSubresourceRange range = {
        VK_IMAGE_ASPECT_COLOR_BIT,  // VkImageAspectFlags    aspectMask;
        0u,                         // uint32_t              baseMipLevel;
        1u,                         // uint32_t              levelCount;
        0u,                         // uint32_t              baseArrayLayer;
        1u,                         // uint32_t              layerCount;
    };

    _vk.vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

    _vk.vkCmdClearColorImage(commandBuffer, _surfaceImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clearColor, 1, &range);

    _vk.vkEndCommandBuffer(commandBuffer);

    const VkSubmitInfo submitInfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,  // VkStructureType                sType;
        nullptr,                        // const void*                    pNext;
        0,                              // uint32_t                       waitSemaphoreCount;
        nullptr,                        // const VkSemaphore*             pWaitSemaphores;
        nullptr,                        // const VkPipelineStageFlags*    pWaitDstStageMask;
        1,                              // uint32_t                       commandBufferCount;
        &commandBuffer,                 // const VkCommandBuffer*         pCommandBuffers;
        0,                              // uint32_t                       signalSemaphoreCount;
        nullptr,                        // const VkSemaphore*             pSignalSemaphores;
    };

    VkQueue queue;
    _vk.vkGetDeviceQueue(_device, _queueFamilyIndex, 0, &queue);

    const VkFenceCreateInfo fenceInfo = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,    // VkStructureType       sType;
        nullptr,                                // const void*           pNext;
        0,                                      // VkFenceCreateFlags    flags;
    };

    VkFence fence;
    _vk.vkCreateFence(_device, &fenceInfo, _allocator, &fence);

    _vk.vkQueueSubmit(queue, 1, &submitInfo, fence);

    const uint64_t oneMiliSec = 1000000;
    const uint64_t timeout = (uint64_t)16 * oneMiliSec; // 16 ms
    _vk.vkWaitForFences(_device, 1, &fence, VK_TRUE, timeout);
    _vk.vkDestroyFence(_device, fence, _allocator);
}

void GepardVulkan::createDefaultFrameBuffer()
{
    VkResult vkResult;

    const VkImageViewCreateInfo imageViewCreateInfo = {
        VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,   // VkStructureType            sType;
        nullptr,                                    // const void*                pNext;
        0,                                          // VkImageViewCreateFlags     flags;
        _surfaceImage,                              // VkImage                    image;
        VK_IMAGE_VIEW_TYPE_2D,                      // VkImageViewType            viewType;
        _imageFormat,                               // VkFormat                   format;
        {
            VK_COMPONENT_SWIZZLE_IDENTITY, // swizzle r
            VK_COMPONENT_SWIZZLE_IDENTITY, // swizzle g
            VK_COMPONENT_SWIZZLE_IDENTITY, // swizzle b
            VK_COMPONENT_SWIZZLE_IDENTITY, // swizzle a
        },                                          // VkComponentMapping         components;
        {
            VK_IMAGE_ASPECT_COLOR_BIT,  // VkImageAspectFlags    aspectMask;
            0u,                         // uint32_t              baseMipLevel;
            1u,                         // uint32_t              levelCount;
            0u,                         // uint32_t              baseArrayLayer;
            1u,                         // uint32_t              layerCount;
        },                                          // VkImageSubresourceRange    subresourceRange;
    };

    vkResult = _vk.vkCreateImageView(_device, &imageViewCreateInfo, _allocator, &_frameBufferColorAttachmentImageView);
    GD_ASSERT(vkResult == VK_SUCCESS && "Creating the default frame buffer is failed!");

    std::vector<VkImageView> attachments;
    attachments.push_back(_frameBufferColorAttachmentImageView);

    const VkFramebufferCreateInfo frameBufferCreateInfo = {
        VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,  // VkStructureType             sType;
        nullptr,                                    // const void*                 pNext;
        0u,                                         // VkFramebufferCreateFlags    flags;
        _renderPass,                                // VkRenderPass                renderPass;
        static_cast<uint32_t>(attachments.size()),  // uint32_t                    attachmentCount;
        attachments.data(),                         // const VkImageView*          pAttachments;
        _context.surface->width(),                  // uint32_t                    width;
        _context.surface->height(),                 // uint32_t                    height;
        1u,                                         // uint32_t                    layers;
    };

    vkResult = _vk.vkCreateFramebuffer(_device, &frameBufferCreateInfo, _allocator, &_frameBuffer);
    GD_ASSERT(vkResult == VK_SUCCESS && "Creating the default frame buffer is failed!");
}

uint32_t GepardVulkan::getMemoryTypeIndex(VkMemoryRequirements memoryRequirements, VkMemoryPropertyFlags properties)
{
    /* Algorithm copied from Vulkan specification */
    for (uint32_t i = 0; i < _physicalDeviceMemoryProperties.memoryTypeCount; ++i)
    {
        if ((memoryRequirements.memoryTypeBits & (1 << i)) &&
            ((_physicalDeviceMemoryProperties.memoryTypes[i].propertyFlags & properties) == properties))
            return i;
    }

    GD_ASSERT(false && "No feasible memory type index!");
    return -1;
}

void GepardVulkan::createSwapChain()
{
    GD_ASSERT(_context.surface->getDisplay());
    GD_ASSERT(!_wsiSurface);
#ifdef VK_USE_PLATFORM_XLIB_KHR
    // Todo: check if this could be moved in to gepard-xsurface
    XSync((Display*)_context.surface->getDisplay(), false);

    const VkXlibSurfaceCreateInfoKHR surfaceCreateInfo = {
        VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR, // VkStructureType                sType;
        nullptr,                                        // const void*                    pNext;
        0,                                              // VkXlibSurfaceCreateFlagsKHR    flags;
        (Display*)_context.surface->getDisplay(),               // Display*                       dpy;
        (Window)_context.surface->getWindow(),                  // Window                         window;
    };

    _vk.vkCreateXlibSurfaceKHR(_instance, &surfaceCreateInfo, _allocator, &_wsiSurface);
#endif // VK_USE_PLATFORM_XLIB_KHR
    GD_ASSERT(_wsiSurface);

    uint32_t surfaceFormatCount;
    _vk.vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _wsiSurface, &surfaceFormatCount, nullptr);

    std::vector<VkSurfaceFormatKHR> surfaceFormats(surfaceFormatCount);
    _vk.vkGetPhysicalDeviceSurfaceFormatsKHR(_physicalDevice, _wsiSurface, &surfaceFormatCount, surfaceFormats.data());

    VkFormat swapchainFormat;
    if ((surfaceFormatCount == 1) && (surfaceFormats[0].format == VK_FORMAT_UNDEFINED)) {
        swapchainFormat = VK_FORMAT_R8G8B8_UNORM;
    } else {
        GD_ASSERT(surfaceFormatCount >= 1);
        swapchainFormat = surfaceFormats[0].format;
    }
    const VkColorSpaceKHR swapchainColorSpace = surfaceFormats[0].colorSpace;

    const VkExtent2D imageSize = {
        _context.surface->width(),  // uint32_t    width;
        _context.surface->height(), // uint32_t    height;
    };

    VkSurfaceCapabilitiesKHR surfaceCapabilities;
    _vk.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physicalDevice, _wsiSurface, &surfaceCapabilities);

    // Application desires to simultaneously acquire 2 images (which is one
    // more than "surfCapabilities.minImageCount").
    uint32_t swapchainImagesCount = surfaceCapabilities.minImageCount + 1;

    uint32_t presentModeCount;
    _vk.vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _wsiSurface, &presentModeCount, NULL);
    std::vector<VkPresentModeKHR> presentModes(presentModeCount);
    _vk.vkGetPhysicalDeviceSurfacePresentModesKHR(_physicalDevice, _wsiSurface, &presentModeCount, presentModes.data());

    // If mailbox mode is available, use it, as it is the lowest-latency non-
    // tearing mode.  If not, fall back to FIFO which is always available.
    VkPresentModeKHR swapchainPresentMode = VK_PRESENT_MODE_FIFO_KHR;
    for (size_t i = 0; i < presentModes.size(); i++)
    {
        if (presentModes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            swapchainPresentMode = VK_PRESENT_MODE_MAILBOX_KHR;
            break;
        }
    }

    const VkSwapchainCreateInfoKHR swapChainCreateInfo = {
        VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,    // VkStructureType                  sType;
        nullptr,                                        // const void*                      pNext;
        0,                                              // VkSwapchainCreateFlagsKHR        flags;
        _wsiSurface,                                    // VkSurfaceKHR                     surface;
        swapchainImagesCount,                           // uint32_t                         minImageCount;
        swapchainFormat,                                // VkFormat                         imageFormat;
        swapchainColorSpace,                            // VkColorSpaceKHR                  imageColorSpace;
        imageSize,                                      // VkExtent2D                       imageExtent;
        1u,                                             // uint32_t                         imageArrayLayers;
        VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,            // VkImageUsageFlags                imageUsage;
        VK_SHARING_MODE_EXCLUSIVE,                      // VkSharingMode                    imageSharingMode;
        0u,                                             // uint32_t                         queueFamilyIndexCount;
        nullptr,                                        // const uint32_t*                  pQueueFamilyIndices;
        surfaceCapabilities.currentTransform,           // VkSurfaceTransformFlagBitsKHR    preTransform;
        VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR,             // VkCompositeAlphaFlagBitsKHR      compositeAlpha;
        swapchainPresentMode,                           // VkPresentModeKHR                 presentMode;
        VK_TRUE,                                        // VkBool32                         clipped;
        VK_NULL_HANDLE,                                 // VkSwapchainKHR                   oldSwapchain;
    };

    _vk.vkCreateSwapchainKHR(_device, &swapChainCreateInfo, _allocator, &_wsiSwapChain);
    GD_ASSERT(_wsiSwapChain);

    _vk.vkGetSwapchainImagesKHR(_device, _wsiSwapChain, &swapchainImagesCount, nullptr);
    _wsiSwapChainImages.resize(swapchainImagesCount);
    _vk.vkGetSwapchainImagesKHR(_device, _wsiSwapChain, &swapchainImagesCount, _wsiSwapChainImages.data());
}

void GepardVulkan::presentImage()
{
    const VkFenceCreateInfo fenceInfo = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,    // VkStructureType       sType;
        nullptr,                                // const void*           pNext;
        0,                                      // VkFenceCreateFlags    flags;
    };
    VkFence fence;
    _vk.vkCreateFence(_device, &fenceInfo, _allocator, &fence);

    const uint64_t oneMiliSec = 1000000;
    const uint64_t timeout = (uint64_t)16 * oneMiliSec; // 16 ms

    uint32_t imageIndex;
    _vk.vkAcquireNextImageKHR(_device, _wsiSwapChain, timeout, VK_NULL_HANDLE, fence, &imageIndex);

    VkQueue queue;
    _vk.vkGetDeviceQueue(_device, _queueFamilyIndex, 0, &queue);

    const VkCommandBuffer commandBuffer = _primaryCommandBuffers[0];

    const VkCommandBufferBeginInfo commandBufferBeginInfo = {
       VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, // VkStructureType                          sType;
       nullptr,                                     // const void*                              pNext;
       VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, // VkCommandBufferUsageFlags                flags;
       nullptr,                                     // const VkCommandBufferInheritanceInfo*    pInheritanceInfo;
    };

    const VkImageSubresourceLayers subresource = {
        VK_IMAGE_ASPECT_COLOR_BIT,  // VkImageAspectFlags    aspectMask;
        0u,                         // uint32_t              mipLevel;
        0u,                         // uint32_t              baseArrayLayer;
        1u,                         // uint32_t              layerCount;
    };

    const VkOffset3D offset_1 = { 0, 0, 0 };
    const VkOffset3D offset_2 = {
        (int32_t)_context.surface->width(),
        (int32_t)_context.surface->height(),
        1,
    };

    const VkImageBlit imageCopy = {
        subresource,    // VkImageSubresourceLayers    srcSubresource;
        {
            offset_1,
            offset_2,
        },              // VkOffset3D                  srcOffsets[2];
        subresource,    // VkImageSubresourceLayers    dstSubresource;
        {
            offset_1,
            offset_2,
        },              // VkOffset3D                  dstOffsets[2];
    };

    const VkImageSubresourceRange subresourceRange = {
        VK_IMAGE_ASPECT_COLOR_BIT,  // VkImageAspectFlags    aspectMask;
        0u,                         // uint32_t              baseMipLevel;
        1u,                         // uint32_t              levelCount;
        0u,                         // uint32_t              baseArrayLayer;
        1u,                         // uint32_t              layerCount;
    };

    const VkImageMemoryBarrier preCopyBarrierFBO = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     // VkStructureType            sType;
        nullptr,                                    // const void*                pNext;
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,       // VkAccessFlags              srcAccessMask;
        VK_ACCESS_TRANSFER_READ_BIT,                // VkAccessFlags              dstAccessMask;
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,   // VkImageLayout              oldLayout;
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,       // VkImageLayout              newLayout;
        _queueFamilyIndex,                          // uint32_t                   srcQueueFamilyIndex;
        _queueFamilyIndex,                          // uint32_t                   dstQueueFamilyIndex;
        _surfaceImage,                              // VkImage                    image;
        subresourceRange,                           // VkImageSubresourceRange    subresourceRange;
    };

    const VkImageMemoryBarrier postCopyBarrierFBO = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     // VkStructureType            sType;
        nullptr,                                    // const void*                pNext;
        VK_ACCESS_TRANSFER_READ_BIT,                // VkAccessFlags              srcAccessMask;
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,       // VkAccessFlags              dstAccessMask;
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,       // VkImageLayout              oldLayout;
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,   // VkImageLayout              newLayout;
        _queueFamilyIndex,                          // uint32_t                   srcQueueFamilyIndex;
        _queueFamilyIndex,                          // uint32_t                   dstQueueFamilyIndex;
        _surfaceImage,                              // VkImage                    image;
        subresourceRange,                           // VkImageSubresourceRange    subresourceRange;
    };

    const VkImageMemoryBarrier preCopyBarrierWSI = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, // VkStructureType            sType;
        nullptr,                                // const void*                pNext;
        VK_ACCESS_MEMORY_READ_BIT,              // VkAccessFlags              srcAccessMask;
        VK_ACCESS_TRANSFER_WRITE_BIT,           // VkAccessFlags              dstAccessMask;
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,        // VkImageLayout              oldLayout;
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,   // VkImageLayout              newLayout;
        _queueFamilyIndex,                      // uint32_t                   srcQueueFamilyIndex;
        _queueFamilyIndex,                      // uint32_t                   dstQueueFamilyIndex;
        _surfaceImage,                          // VkImage                    image;
        subresourceRange,                       // VkImageSubresourceRange    subresourceRange;
    };

    const VkImageMemoryBarrier postCopyBarrierWSI = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER, // VkStructureType            sType;
        nullptr,                                // const void*                pNext;
        VK_ACCESS_TRANSFER_WRITE_BIT,           // VkAccessFlags              srcAccessMask;
        VK_ACCESS_MEMORY_READ_BIT,              // VkAccessFlags              dstAccessMask;
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,   // VkImageLayout              oldLayout;
        VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,        // VkImageLayout              newLayout;
        _queueFamilyIndex,                      // uint32_t                   srcQueueFamilyIndex;
        _queueFamilyIndex,                      // uint32_t                   dstQueueFamilyIndex;
        _wsiSwapChainImages[imageIndex],        // VkImage                    image;
        subresourceRange,                       // VkImageSubresourceRange    subresourceRange;
    };

    const VkImageMemoryBarrier preCopyBarriers[] = {
        preCopyBarrierFBO,
        preCopyBarrierWSI,
    };

    const VkImageMemoryBarrier postCopyBarriers[] = {
        postCopyBarrierFBO,
        postCopyBarrierWSI,
    };

    _vk.vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
    _vk.vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, (VkDependencyFlags)0, 0, (const VkMemoryBarrier*)nullptr, 0, (const VkBufferMemoryBarrier*)nullptr, 2, preCopyBarriers);
    _vk.vkCmdBlitImage(commandBuffer, _surfaceImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, _wsiSwapChainImages[imageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &imageCopy, VK_FILTER_NEAREST);
    _vk.vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_HOST_BIT, (VkDependencyFlags)0, 0, (const VkMemoryBarrier*)nullptr, 0, (const VkBufferMemoryBarrier*)nullptr, 2, postCopyBarriers);
    _vk.vkEndCommandBuffer(commandBuffer);

    const VkSubmitInfo submitInfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,  // VkStructureType                sType;
        nullptr,                        // const void*                    pNext;
        0,                              // uint32_t                       waitSemaphoreCount;
        nullptr,                        // const VkSemaphore*             pWaitSemaphores;
        nullptr,                        // const VkPipelineStageFlags*    pWaitDstStageMask;
        1,                              // uint32_t                       commandBufferCount;
        &commandBuffer,                 // const VkCommandBuffer*         pCommandBuffers;
        0,                              // uint32_t                       signalSemaphoreCount;
        nullptr,                        // const VkSemaphore*             pSignalSemaphores;
    };

    _vk.vkQueueSubmit(queue, 1, &submitInfo, fence);
    _vk.vkWaitForFences(_device, 1, &fence, VK_TRUE, timeout);

    const VkPresentInfoKHR presentInfo = {
        VK_STRUCTURE_TYPE_PRESENT_INFO_KHR, // VkStructureType          sType;
        nullptr,                            // const void*              pNext;
        0u,                                 // uint32_t                 waitSemaphoreCount;
        VK_NULL_HANDLE,                     // const VkSemaphore*       pWaitSemaphores;
        1u,                                 // uint32_t                 swapchainCount;
        &_wsiSwapChain,                     // const VkSwapchainKHR*    pSwapchains;
        &imageIndex,                        // const uint32_t*          pImageIndices;
        nullptr,                            // VkResult*                pResults;
    };

    _vk.vkQueuePresentKHR(queue, &presentInfo);

    // Clean up
    _vk.vkDestroyFence(_device, fence, _allocator);
}

void GepardVulkan::readImage()
{
    VkBuffer buffer;
    VkDeviceMemory bufferAlloc;
    const VkCommandBuffer commandBuffer = _primaryCommandBuffers[0];
    const uint32_t width = _context.surface->width();
    const uint32_t height = _context.surface->width();
    const VkDeviceSize dataSize = width * height * 4; // r8g8b8a8 format

    // Create destination buffer
    const VkBufferCreateInfo bufferInfo = {
        VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,   // VkStructureType        sType;
        nullptr,                                // const void*            pNext;
        0u,                                     // VkBufferCreateFlags    flags;
        dataSize,                               // VkDeviceSize           size;
        VK_BUFFER_USAGE_TRANSFER_DST_BIT,       // VkBufferUsageFlags     usage;
        VK_SHARING_MODE_EXCLUSIVE,              // VkSharingMode          sharingMode;
        1u,                                     // uint32_t               queueFamilyIndexCount;
        &_queueFamilyIndex,                     // const uint32_t*        pQueueFamilyIndices;
    };

    _vk.vkCreateBuffer(_device, &bufferInfo, _allocator, &buffer);

    VkMemoryRequirements memoryRequirements;
    _vk.vkGetBufferMemoryRequirements(_device, buffer, &memoryRequirements);
    const VkMemoryAllocateInfo allocationInfo = {
        VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,                                         // VkStructureType    sType;
        nullptr,                                                                        // const void*        pNext;
        memoryRequirements.size,                                                        // VkDeviceSize       allocationSize;
        getMemoryTypeIndex(memoryRequirements, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT),    // uint32_t           memoryTypeIndex;
    };

    _vk.vkAllocateMemory(_device, &allocationInfo, _allocator, &bufferAlloc);
    _vk.vkBindBufferMemory(_device, buffer, bufferAlloc, 0);

    const VkImageSubresourceRange subresourceRange = {
        VK_IMAGE_ASPECT_COLOR_BIT,  // VkImageAspectFlags    aspectMask;
        0u,                         // uint32_t              baseMipLevel;
        1u,                         // uint32_t              levelCount;
        0u,                         // uint32_t              baseArrayLayer;
        1u,                         // uint32_t              layerCount;
    };

    const VkImageMemoryBarrier preImageBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     // VkStructureType            sType;
        nullptr,                                    // const void*                pNext;
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,       // VkAccessFlags              srcAccessMask;
        VK_ACCESS_TRANSFER_READ_BIT,                // VkAccessFlags              dstAccessMask;
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,   // VkImageLayout              oldLayout;
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,       // VkImageLayout              newLayout;
        VK_QUEUE_FAMILY_IGNORED,                    // uint32_t                   srcQueueFamilyIndex;
        VK_QUEUE_FAMILY_IGNORED,                    // uint32_t                   dstQueueFamilyIndex;
        _surfaceImage,                              // VkImage                    image;
        subresourceRange,                           // VkImageSubresourceRange    subresourceRange;
    };

    const VkImageMemoryBarrier postImageBarrier = {
        VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     // VkStructureType            sType;
        nullptr,                                    // const void*                pNext;
        VK_ACCESS_TRANSFER_READ_BIT,                // VkAccessFlags              srcAccessMask;
        VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,       // VkAccessFlags              dstAccessMask;
        VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,       // VkImageLayout              oldLayout;
        VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,   // VkImageLayout              newLayout;
        _queueFamilyIndex,                          // uint32_t                   srcQueueFamilyIndex;
        _queueFamilyIndex,                          // uint32_t                   dstQueueFamilyIndex;
        _surfaceImage,                              // VkImage                    image;
        subresourceRange,                           // VkImageSubresourceRange    subresourceRange;
    };

    const VkBufferMemoryBarrier bufferBarrier = {
        VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,    // VkStructureType    sType;
        nullptr,                                    // const void*        pNext;
        VK_ACCESS_TRANSFER_WRITE_BIT,               // VkAccessFlags      srcAccessMask;
        VK_ACCESS_HOST_READ_BIT,                    // VkAccessFlags      dstAccessMask;
        VK_QUEUE_FAMILY_IGNORED,                    // uint32_t           srcQueueFamilyIndex;
        VK_QUEUE_FAMILY_IGNORED,                    // uint32_t           dstQueueFamilyIndex;
        buffer,                                     // VkBuffer           buffer;
        0u,                                         // VkDeviceSize       offset;
        dataSize,                                   // VkDeviceSize       size;
    };

    const VkImageSubresourceLayers imageSubresource = {
        VK_IMAGE_ASPECT_COLOR_BIT,  // VkImageAspectFlags    aspectMask;
        0u,                         // uint32_t              mipLevel;
        0u,                         // uint32_t              baseArrayLayer;
        1u,                         // uint32_t              baseArrayLayer;
    };

    const VkBufferImageCopy copyRegion = {
        0u,                 // VkDeviceSize                bufferOffset;
        width,              // uint32_t                    bufferRowLength;
        height,             // uint32_t                    bufferImageHeight;
        imageSubresource,   // VkImageSubresourceLayers    imageSubresource;
        { 0, 0, 0 },        // VkOffset3D                  imageOffset;
        {
            width,
            height,
            1u,
        },                  // VkExtent3D                  imageExtent;
    };

    const VkCommandBufferBeginInfo commandBufferBeginInfo = {
       VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO, // VkStructureType                          sType;
       nullptr,                                     // const void*                              pNext;
       VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, // VkCommandBufferUsageFlags                flags;
       nullptr,                                     // const VkCommandBufferInheritanceInfo*    pInheritanceInfo;
    };

    _vk.vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

    _vk.vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, (VkDependencyFlags)0, 0, (const VkMemoryBarrier*)nullptr, 0, (const VkBufferMemoryBarrier*)nullptr, 1, &preImageBarrier);
    _vk.vkCmdCopyImageToBuffer(commandBuffer, _surfaceImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, buffer, 1, &copyRegion);
    _vk.vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, (VkDependencyFlags)0, 0, (const VkMemoryBarrier*)nullptr, 1, &bufferBarrier, 1, &postImageBarrier);
    _vk.vkEndCommandBuffer(commandBuffer);

    const VkSubmitInfo submitInfo = {
        VK_STRUCTURE_TYPE_SUBMIT_INFO,  // VkStructureType                sType;
        nullptr,                        // const void*                    pNext;
        0,                              // uint32_t                       waitSemaphoreCount;
        nullptr,                        // const VkSemaphore*             pWaitSemaphores;
        nullptr,                        // const VkPipelineStageFlags*    pWaitDstStageMask;
        1,                              // uint32_t                       commandBufferCount;
        &commandBuffer,                 // const VkCommandBuffer*         pCommandBuffers;
        0,                              // uint32_t                       signalSemaphoreCount;
        nullptr,                        // const VkSemaphore*             pSignalSemaphores;
    };

    VkQueue queque;
    _vk.vkGetDeviceQueue(_device, _queueFamilyIndex, 0, &queque);

    const VkFenceCreateInfo fenceInfo = {
        VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,    // VkStructureType       sType;
        nullptr,                                // const void*           pNext;
        0,                                      // VkFenceCreateFlags    flags;
    };

    VkFence fence;
    _vk.vkCreateFence(_device, &fenceInfo, _allocator, &fence);

    _vk.vkQueueSubmit(queque, 1, &submitInfo, fence);

    const uint64_t oneMiliSec = 1000000;
    const uint64_t timeout = (uint64_t)16 * oneMiliSec; // 16 ms
    _vk.vkWaitForFences(_device, 1, &fence, VK_TRUE, timeout);

    void* data;
    _vk.vkMapMemory(_device, bufferAlloc, 0, dataSize, 0, &data);

    const VkMappedMemoryRange range = {
        VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,  // VkStructureType    sType;
        nullptr,                                // const void*        pNext;
        bufferAlloc,                            // VkDeviceMemory     memory;
        0,                                      // VkDeviceSize       offset;
        dataSize,                               // VkDeviceSize       size;
    };

    _vk.vkInvalidateMappedMemoryRanges(_device, 1, &range);
    std::memcpy(_context.surface->getBuffer(), data, dataSize);

    // Clean up
    _vk.vkUnmapMemory(_device, bufferAlloc);

    _vk.vkFreeMemory(_device, bufferAlloc, _allocator);
    _vk.vkDestroyFence(_device, fence, _allocator);
    _vk.vkDestroyBuffer(_device, buffer, _allocator);
}

} // namespace vulkan
} // namespace gepard

#endif // GD_USE_VULKAN
