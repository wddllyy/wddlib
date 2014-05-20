ADD_CUSTOM_COMMAND(
OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/test.pb.h ${CMAKE_CURRENT_SOURCE_DIR}/test.pb.cc
COMMAND ${PB_TOOLS} --proto_path=${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/test.proto --cpp_out=${CMAKE_CURRENT_SOURCE_DIR}
DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test.proto
)
MESSAGE(STATUS "pwd:${CMAKE_CURRENT_SOURCE_DIR}")

ADD_CUSTOM_COMMAND(
OUTPUT ${EXECUTABLE_OUTPUT_PATH}/testapp.conf
COMMAND cp ${CMAKE_CURRENT_SOURCE_DIR}/testapp.conf ${EXECUTABLE_OUTPUT_PATH}/testapp.conf
DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/testapp.conf
)


FILE(GLOB_RECURSE SourceFiles "*.cpp")
add_executable(framework_test ${SourceFiles} ${CMAKE_CURRENT_SOURCE_DIR}/test.pb.cc ${EXECUTABLE_OUTPUT_PATH}/testapp.conf)
target_link_libraries(framework_test util protobuf_lib)