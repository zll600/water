include_directories(${PROJECT_SOURCE_DIR})
link_directories(${PROJECT_SOURCE_DIR}/lib)

set(LIBS water)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin/test)

set(TESTS_FILES "")
aux_source_directory("${CMAKE_SOURCE_DIR}/tests" TESTS_FILES)

foreach(TESTS_FILE_PATH ${TESTS_FILES})
    string(REGEX REPLACE ".+[/\]([^/\.]+)\\.cc" "\\1" TESTS_FILE_NAME ${TESTS_FILE_PATH})
    message(STATUS "Generating test target: ${TESTS_FILE_NAME}")
    water_add_executable(${TESTS_FILE_NAME} ${TESTS_FILE_NAME}.cc water ${LIBS})
endforeach(TESTS_FILE_PATH)
