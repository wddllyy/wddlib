FILE(GLOB_RECURSE SourceFiles "*.cpp")
add_executable(log_test ${SourceFiles})
target_link_libraries(log_test util)