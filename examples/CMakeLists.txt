function(build_example directory)
  file(GLOB SOURCES ${directory}/*.cpp)
  add_executable("${directory}" ${SOURCES})
  target_link_libraries("${directory}" gepard X11 ${LIB_DEPENDENCIES})
endfunction(build_example)

include_directories(${CMAKE_BINARY_DIR}/include/)

build_example("fill-rect" "")
