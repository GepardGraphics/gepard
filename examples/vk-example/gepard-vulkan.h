/*! \todo Add license */

#ifndef GEPARD_VULKAN_H
#define GEPARD_VULKAN_H

#include <vulkan/vulkan.h>

namespace gepard {

extern void* pVulkanLibrary;
extern VkInstance vkInstance;

#define GD_VK_DECLARE_FUNCTION(fun) extern PFN_##fun fun;

/*! Global level functions */
GD_VK_DECLARE_FUNCTION(vkGetInstanceProcAddr);
GD_VK_DECLARE_FUNCTION(vkCreateInstance);

/*! \todo create class for the Instance */
/*! Instance level functions */
GD_VK_DECLARE_FUNCTION(vkDestroyInstance);

#undef GD_VK_DECLARE_FUNCTION

bool loadVulkanGlobalFunctions();
bool loadVulkanIstanceFunctions();
bool createDefaultInstance();
void destroyVulkanInstance();
void closeVulkanLibrary();

} // namespace gepard

#endif
