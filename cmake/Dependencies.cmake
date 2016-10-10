SET(LIB_DEPENDENCIES "")

find_package(EGL REQUIRED)

if (${USE_GLES2})
  find_package(GLES2 REQUIRED)
endif ()

if (${USE_VULKAN})
    find_package(VULKAN REQUIRED)
endif ()
