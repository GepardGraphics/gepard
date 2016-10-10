find_path(VULKAN_INCLUDE_DIR "vulkan/vulkan.h"
          PATH "thirdparty")

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(VULKAN
                                  REQUIRED_VARS VULKAN_INCLUDE_DIR)

include_directories(${VULKAN_INCLUDE_DIR})

