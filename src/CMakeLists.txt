set(COMMON_SOURCES
    engines/gepard-path.cpp
    gepard.cpp
    gepard-engine.cpp
    utils/gepard-types.cpp
    utils/gepard-defs.cpp
)

set(GLES2_SOURCES
    engines/gles2/gepard-gles2.cpp
    engines/gles2/gepard-gles2-shader-factory.cpp
)

set(VULKAN_SOURCES
    engines/vulkan/gepard-vulkan-interface.cpp
    engines/vulkan/gepard-vulkan.cpp
)

set(SOFTWARE_SOURCES
    engines/software/gepard-software.cpp
)

set(COMMON_INCLUDE_DIRS
    .
    engines
    surfaces
    utils
)

set(GLES2_INCLUDE_DIRS
    engines/gles2
)

set(VULKAN_INCLUDE_DIRS
    engines/vulkan
)

set(SOFTWARE_INCLUDE_DIRS
    engines/software
)

set(SOURCES ${COMMON_SOURCES})
if (${USE_GLES2})
list(APPEND SOURCES ${GLES2_SOURCES})
elseif (${USE_VULKAN})
list(APPEND SOURCES ${VULKAN_SOURCES})
elseif (${USE_SOFTWARE})
list(APPEND SOURCES ${SOFTWARE_SOURCES})
endif()

include_directories(${COMMON_INCLUDE_DIRS})
if (${USE_GLES2})
include_directories(${GLES2_INCLUDE_DIRS})
elseif (${USE_VULKAN})
include_directories(${VULKAN_INCLUDE_DIRS})
elseif (${USE_SOFTWARE})
include_directories(${SOFTWARE_INCLUDE_DIRS})
endif()

add_library(gepard SHARED ${SOURCES})

file(COPY gepard.h DESTINATION ${INCLUDE_OUTPUT_DIR})
file(COPY surfaces DESTINATION ${INCLUDE_OUTPUT_DIR})
