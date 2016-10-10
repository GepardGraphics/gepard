#! /bin/bash

mkdir -p thirdparty/vulkan
cd thirdparty/vulkan

wget https://raw.githubusercontent.com/KhronosGroup/Vulkan-Docs/1.0/src/vulkan/vulkan.h
wget https://raw.githubusercontent.com/KhronosGroup/Vulkan-Docs/1.0/src/vulkan/vk_platform.h

cd -
