/*! \todo Add license */

#include "gepard-vulkan.h"

#include <dlfcn.h>
#include <iostream>

namespace gepard {

void* pVulkanLibrary = nullptr;
VkInstance vkInstance;

#define GD_VK_DEFINE_FUNCTION(fun) PFN_##fun fun = nullptr;

/*! Global level functions */
GD_VK_DEFINE_FUNCTION(vkGetInstanceProcAddr);
GD_VK_DEFINE_FUNCTION(vkCreateInstance);

/*! Instance level functions */
GD_VK_DEFINE_FUNCTION(vkDestroyInstance);

#undef GD_VK_DEFINE_FUNCTION

#define GD_VK_LOAD_FUNCTION(instance, fun)\
    if (!(fun = (PFN_##fun) vkGetInstanceProcAddr (instance, #fun))) { \
        std::cout << "Couldn't load " #fun "!\n"; \
        success &= false; \
    }

bool loadVulkanGlobalFunctions()
{
    bool success = true;

    pVulkanLibrary = dlopen("libvulkan.so", RTLD_NOW);
    if (!pVulkanLibrary) {
        std::cout << "Loading the Vulkan library was unsuccessfuly!\n";
        return false;
    }

    if (!(vkGetInstanceProcAddr = (PFN_vkGetInstanceProcAddr)dlsym(pVulkanLibrary, "vkGetInstanceProcAddr"))) {
        std::cout << "Couldn't load vkGetInstanceProcAddr!\n";
        return false;
    }

    GD_VK_LOAD_FUNCTION(nullptr, vkCreateInstance);

    return success;
}

bool loadVulkanIstanceFunctions()
{
    bool success = true;
    if (!vkInstance)
        return false;

    GD_VK_LOAD_FUNCTION(vkInstance, vkDestroyInstance);

    return true;
}

#undef GD_VK_LOAD_FUNCTION

bool createDefaultInstance()
{
    VkResult result;

    const VkInstanceCreateInfo instanceCreateInfo = {
        VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        nullptr,
        0u,
        nullptr,
        0u,
        nullptr,
        0u,
        nullptr,
    };

    result = vkCreateInstance(&instanceCreateInfo, nullptr, &vkInstance);

    return result == VK_SUCCESS;
}

void destroyVulkanInstance()
{
    if (vkInstance)
        vkDestroyInstance(vkInstance, nullptr);
}

void closeVulkanLibrary()
{
    if (pVulkanLibrary)
        dlclose(pVulkanLibrary);
}

} // namespace gepard
