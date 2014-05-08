FILE(GLOB_RECURSE SourceFiles "*.cpp")
add_executable(container_test ${SourceFiles})
target_link_libraries(container_test util)