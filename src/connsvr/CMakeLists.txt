CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_CUSTOM_COMMAND(
OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/framework/connsvr.pb.h ${CMAKE_CURRENT_SOURCE_DIR}/framework/connsvr.pb.cc
COMMAND ${PB_TOOLS} --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/framework ${CMAKE_CURRENT_SOURCE_DIR}/framework/connsvr.proto --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/framework
DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/framework/connsvr.proto
)
MESSAGE(STATUS "pwd:${CMAKE_CURRENT_SOURCE_DIR}")


FILE(GLOB_RECURSE SourceFiles "*.cpp")
add_executable(connsvr ${SourceFiles} ${CMAKE_CURRENT_SOURCE_DIR}/framework/connsvr.pb.cc)
target_link_libraries(connsvr util protobuf_lib)
