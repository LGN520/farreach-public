cmake_minimum_required (VERSION 3.5)
project (XIndex)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_COMPILER /usr/bin/g++)
#set(JEMALLOC_DIR "/usr/lib/x86_64-linux-gnu")
set(JEMALLOC_DIR "/usr/local/lib")
set(MKL_LINK_DIRECTORY "/opt/intel/mkl/lib/intel64")
set(MKL_INCLUDE_DIRECTORY "/opt/intel/mkl/include")

# Set a default build type if none was specified
# https://blog.kitware.com/cmake-and-the-default-build-type/
#set(default_build_type "Release")
#if(EXISTS "${CMAKE_SOURCE_DIR}/.git")
#  set(default_build_type "Debug")
#endif()

#if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
#  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
#  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE
#      STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
#  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
#    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
#endif()

link_directories(${JEMALLOC_DIR})
link_directories(${MKL_LINK_DIRECTORY})
link_libraries(jemalloc)

include_directories(${MKL_INCLUDE_DIRECTORY})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

if(NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE "Debug")
endif()
string(TOLOWER ${CMAKE_BUILD_TYPE} _type)
if (_type STREQUAL release)
	add_compile_options(-Wall -fmax-errors=5 -faligned-new -march=native -mtune=native)
else()
	add_compile_options(-g -Wall -fmax-errors=5 -faligned-new -march=native -mtune=native)
endif()
message(STATUS "Setting build type to '${CMAKE_BUILD_TYPE}' ")

# microbench
#add_executable(microbench
#    ${CMAKE_CURRENT_SOURCE_DIR}/microbench.cpp
#)
#target_link_libraries(microbench
#    PRIVATE
#        mkl_rt
#        -lpthread
#)

# prepare
add_executable(prepare ${CMAKE_CURRENT_SOURCE_DIR}/prepare.cpp)

# client
add_executable(client ${CMAKE_CURRENT_SOURCE_DIR}/client.cpp)
target_link_libraries(client PRIVATE -lpthread)

# server
add_executable(server ${CMAKE_CURRENT_SOURCE_DIR}/server.cpp)
target_link_libraries(server PRIVATE -lpthread mkl_rt)
target_compile_definitions(server PRIVATE NDEBUGGING)
