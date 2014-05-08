CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

PROJECT(wddlib CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif()


INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/src)


SET(CMAKE_CXX_FLAGS "-Wall -Wextra -Werror -Wno-unused-parameter -Wno-deprecated -Wno-strict-aliasing -fno-omit-frame-pointer -g")

set(CMAKE_CXX_FLAGS_DEBUG "-O0 -DDEBUG")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

add_subdirectory(src/util)
if(CMAKE_BUILD_EXAMPLES)
  add_subdirectory(testcase)
endif()