cmake_minimum_required(VERSION 3.28)
project(sorting-playground VERSION 1.0 LANGUAGES CXX)

# set default to Release if no configuration is specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose Debug or Release" FORCE)
endif()

# C++ settings
set(CMAKE_C_COMPILER gcc)
set(CMAKE_CXX_COMPILER g++)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# optimization flags for Release
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# add your subprojects
add_subdirectory(src/lib)
add_subdirectory(src/bin)
