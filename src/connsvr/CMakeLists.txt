CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_CUSTOM_COMMAND(
OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/framework/connsvr.pb.h ${CMAKE_CURRENT_SOURCE_DIR}/framework/connsvr.pb.cc
COMMAND ${PB_TOOLS} --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/framework ${CMAKE_CURRENT_SOURCE_DIR}/framework/connsvr.proto --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/framework
DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/framework/connsvr.proto
)


ADD_CUSTOM_COMMAND(
OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/protocol/protocol_connsvr.pb.h ${CMAKE_CURRENT_SOURCE_DIR}/protocol/protocol_connsvr.pb.cc
COMMAND ${PB_TOOLS} --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/protocol ${CMAKE_CURRENT_SOURCE_DIR}/protocol/protocol_connsvr.proto --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/protocol
DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/protocol/protocol_connsvr.proto
)

MESSAGE(STATUS "pwd:${CMAKE_CURRENT_SOURCE_DIR}")

ADD_CUSTOM_COMMAND(
OUTPUT ${EXECUTABLE_OUTPUT_PATH}/connsvr.conf
COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/framework/connsvr.conf ${EXECUTABLE_OUTPUT_PATH}/connsvr.conf
DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/framework/connsvr.conf
)


FILE(GLOB_RECURSE SourceFiles "*.cpp")
add_executable(connsvr ${SourceFiles} ${CMAKE_CURRENT_SOURCE_DIR}/framework/connsvr.pb.cc ${CMAKE_CURRENT_SOURCE_DIR}/protocol/protocol_connsvr.pb.cc ${EXECUTABLE_OUTPUT_PATH}/connsvr.conf )
target_link_libraries(connsvr util protobuf_lib)
