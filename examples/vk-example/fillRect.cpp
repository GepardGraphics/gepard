/*! \todo Add license */

#include "fillRect.h"

#include "gepard-vulkan.h"

#include <iostream>

namespace gepard {

bool initVulkan()
{
    if (!loadVulkanGlobalFunctions())
        return false;
    if (!createDefaultInstance())
        return false;
    if (!loadVulkanIstanceFunctions())
        return false;

    return true;
}

void deinitVulkan()
{
    destroyVulkanInstance();
    closeVulkanLibrary();
}

void fillRect(float x, float y, float width, float height)
{
    std::cout << "fillRect " << x << "," << y << "," << width << "," << height << "\n";
}

}
