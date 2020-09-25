cmake_minimum_required(VERSION 3.0.0)
project(tcpp-chuffed)

set(CMAKE_BUILD_TYPE Release)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 11)

include_directories(include)
add_subdirectory(src)

set_target_properties(tcpp-chuffed PROPERTIES  
                      RUNTIME_OUTPUT_DIRECTORY_RELEASE 
                      ${CMAKE_SOURCE_DIR})
