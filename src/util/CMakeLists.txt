
ADD_CUSTOM_COMMAND(
OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/pbparser/UtilProtocol.pb.h ${CMAKE_CURRENT_SOURCE_DIR}/pbparser/UtilProtocol.pb.cc
COMMAND ${PB_TOOLS} --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/../ --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/../../dep/protobuf/include/  ${CMAKE_CURRENT_SOURCE_DIR}/../util/pbparser/UtilProtocol.proto --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}/../
DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/pbparser/UtilProtocol.proto
)


FILE(GLOB_RECURSE SourceFiles "*.cpp")
MESSAGE(STATUS "SourceFiles:${SourceFiles}") 
ADD_LIBRARY(util STATIC ${SourceFiles} ${CMAKE_CURRENT_SOURCE_DIR}/pbparser/UtilProtocol.pb.cc coroutine/co_swapcontext.o)
TARGET_LINK_LIBRARIES(util)
