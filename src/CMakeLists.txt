file(GLOB_RECURSE SOURCES *.cpp)

add_library(gepard SHARED ${SOURCES})
#set_target_properties(gepard PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${LIB_OUTPUT_DIR})

file(GLOB_RECURSE header_list *.h)
set(dir_list "")
foreach(file_path ${header_list})
  get_filename_component(dir_path ${file_path} PATH)
  list(APPEND dir_list ${dir_path})
endforeach()
list(REMOVE_DUPLICATES dir_list)
include_directories(${dir_list})

file(COPY gepard.h gepard-xsurface.h gepard-surface.h DESTINATION ${INCLUDE_OUTPUT_DIR})
