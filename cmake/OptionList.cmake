include(OptionMacros)

ADD_CHOICE (BACKEND "Backend to use" "GLES2 SOFTWARE VULKAN" GLES2)
ADD_OPTION (LOG_LEVEL "Print log messages during execution" 0)
ADD_OPTION (DISABLE_LOG_COLORS "Do not color log messages" OFF)
ADD_OPTION (ENABLE_MINITRACE "Enable tracing" OFF)

if(ENABLE_MINITRACE)
    add_definitions(-DMTR_ENABLED)
else()
    add_definitions(-DMTR_DISABLE)
endif()
