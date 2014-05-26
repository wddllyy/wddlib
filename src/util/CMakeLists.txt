FILE(GLOB_RECURSE SourceFiles "*.cpp")
MESSAGE(STATUS "SourceFiles:${SourceFiles}") 
ADD_LIBRARY(util STATIC ${SourceFiles} coroutine/co_swapcontext.o)
TARGET_LINK_LIBRARIES(util)