CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

ADD_CUSTOM_COMMAND(
OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/framework/testconnsvr.pb.h ${CMAKE_CURRENT_SOURCE_DIR}/framework/testconnsvr.pb.cc
COMMAND ${PB_TOOLS} --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/framework ${CMAKE_CURRENT_SOURCE_DIR}/framework/testconnsvr.proto --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/framework
DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/framework/testconnsvr.proto
)


ADD_CUSTOM_COMMAND(
OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/protocol/protocol_connsvr.pb.h ${CMAKE_CURRENT_SOURCE_DIR}/protocol/protocol_connsvr.pb.cc
COMMAND ${PB_TOOLS} --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/../connsvr/protocol ${CMAKE_CURRENT_SOURCE_DIR}/../connsvr/protocol/protocol_connsvr.proto --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/protocol
DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../connsvr/protocol/protocol_connsvr.proto
)

MESSAGE(STATUS "pwd:${CMAKE_CURRENT_SOURCE_DIR}")


ADD_CUSTOM_COMMAND(
OUTPUT ${EXECUTABLE_OUTPUT_PATH}/testconnsvr.conf
COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/framework/testconnsvr.conf ${EXECUTABLE_OUTPUT_PATH}/testconnsvr.conf
DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/framework/testconnsvr.conf
)


FILE(GLOB_RECURSE SourceFiles "*.cpp")
add_executable(connsvrtest ${SourceFiles} ${CMAKE_CURRENT_SOURCE_DIR}/framework/testconnsvr.pb.cc ${CMAKE_CURRENT_SOURCE_DIR}/protocol/protocol_connsvr.pb.cc ${EXECUTABLE_OUTPUT_PATH}/testconnsvr.conf )
target_link_libraries(connsvrtest util protobuf_lib)
