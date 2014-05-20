set(BAK_EXECUTABLE_OUTPUT_PATH ${EXECUTABLE_OUTPUT_PATH})
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin/testcase)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/testcase)

MACRO(SUBDIRLIST result curdir)
  FILE(GLOB children RELATIVE ${curdir} ${curdir}/*)
  SET(dirlist "")
  FOREACH(child ${children})
    IF(IS_DIRECTORY ${curdir}/${child} AND NOT ${child} MATCHES ".svn")
        SET(dirlist ${dirlist} ${child})
    ENDIF()
  ENDFOREACH()
  SET(${result} ${dirlist})
ENDMACRO()



############################################################
SUBDIRLIST(SUBDIRS  ${CMAKE_CURRENT_SOURCE_DIR}/util)
MESSAGE(STATUS "SUBDIRS:${SUBDIRS}")

FOREACH(subdir ${SUBDIRS})
    ADD_SUBDIRECTORY(util/${subdir})
ENDFOREACH()



set(EXECUTABLE_OUTPUT_PATH ${BAK_EXECUTABLE_OUTPUT_PATH})