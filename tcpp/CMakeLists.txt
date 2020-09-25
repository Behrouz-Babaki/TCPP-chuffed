cmake_minimum_required(VERSION 3.0.0)
project(tcpp)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(include)
add_subdirectory(src)

set_target_properties(tcpp PROPERTIES  
                      RUNTIME_OUTPUT_DIRECTORY_RELEASE 
                      ${CMAKE_SOURCE_DIR})
