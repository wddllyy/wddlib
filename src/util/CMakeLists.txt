FILE(GLOB_RECURSE SourceFiles "*.cpp")
MESSAGE(STATUS "SourceFiles:${SourceFiles}")
#coroutine/co_swapcontext.o 找不到这玩意，暂时先去掉了，不然编不过
ADD_LIBRARY(util STATIC ${SourceFiles})
TARGET_LINK_LIBRARIES(util)