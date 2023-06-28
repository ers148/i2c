# Download and unpack googletest at configure time
configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download )
if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
endif()

# Prevent overriding the parent project's compiler/linker settings on Windows
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

# Add googletest directly to our build. This defines the gtest and gtest_main targets
add_subdirectory(
        ${CMAKE_CURRENT_BINARY_DIR}/googletest-src
        ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
        EXCLUDE_FROM_ALL)

macro(subdirlist RESULT CURDIR)
    file(GLOB CHILDREN RELATIVE ${CURDIR} ${CURDIR}/*)
    set(DIRLIST "")
    foreach(CHILD ${CHILDREN})
        if(IS_DIRECTORY ${CURDIR}/${CHILD})
            list(APPEND DIRLIST ${CHILD})
        endif()
    endforeach()
    set(${RESULT} ${DIRLIST})
endmacro()

subdirlist(TESTS_LIST "${CMAKE_SOURCE_DIR}/Tests")

extract_valid_cxx_flags(TEST_FLAGS
        -pedantic
        -Wall
        -Wcast-align
        -Wcast-qual
        -Wconversion
        -Wextra
        -Wshadow
)
string(REPLACE " " ";" TEST_FLAGS ${TEST_FLAGS})

foreach(TEST_NAME ${TESTS_LIST})
    file(GLOB_RECURSE SOURCES_STATIC "${TEST_NAME}/*.cpp")
    add_executable("${TEST_NAME}Test" ${SOURCES_STATIC})
    target_link_libraries("${TEST_NAME}Test" pthread rt gtest_main DroneDevice)
    target_compile_options("${TEST_NAME}Test" PUBLIC ${TEST_FLAGS})
    target_compile_features("${TEST_NAME}Test" PRIVATE cxx_std_14 c_std_11)
    add_test("${TEST_NAME}Test" "${TEST_NAME}Test")
endforeach(TEST_NAME)
