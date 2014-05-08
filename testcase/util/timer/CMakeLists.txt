FILE(GLOB_RECURSE SourceFiles "*.cpp")
add_executable(timer_test ${SourceFiles})
target_link_libraries(timer_test util)