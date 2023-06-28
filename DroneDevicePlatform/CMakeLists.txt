cmake_minimum_required(VERSION 3.6)
project(DroneDevicePlatform CXX)
include(ExternalProject)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose build type: Debug Release." FORCE)
endif()
if(NOT PLATFORM)
    message(FATAL_ERROR "PLATFORM is not configured")
endif()

if(NOT DEFINED DRONEDEVICE_DIR)
    set(DRONEDEVICE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../DroneDevice")
endif()

option(USE_LTO "Enable Link Time Optimization." OFF)
#option(DRONEDDEVICE_CODE_COVERAGE "Enable coverage reporting" OFF)

# Code Coverage Configuration
#add_library(coverage_config INTERFACE)
#if(DRONEDDEVICE_CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
#    # Add required flags (GCC & LLVM/Clang)
#    target_compile_options(coverage_config INTERFACE
#        -O0        # no optimization
#        -g         # generate debug info
#        --coverage # sets all required flags
#    )
#    if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
#        target_link_options(coverage_config INTERFACE --coverage)
#    else()
#        target_link_libraries(coverage_config INTERFACE --coverage)
#    endif()
#endif()

set(FLAGS_COMMON "-pedantic -Wall -Wcast-align -Wcast-qual -Wconversion -Wextra -Wshadow")
if(${USE_LTO})
    set(FLAGS_COMMON "${FLAGS_COMMON} -flto -ffat-lto-objects")
endif()

set(FLAGS_COMMON_CXX "${FLAGS_COMMON} -Wold-style-cast -Woverloaded-virtual -fno-exceptions -fno-rtti")

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS "OFF")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS_CPU} ${FLAGS_COMMON_CXX}")

#add_subdirectory(Generic)
add_subdirectory(Platform)

# Configure external library libopencm3
ExternalProject_Add(libopencm3
        GIT_REPOSITORY git@gitlab.corp.geoscan.aero:internal/libopencm3.git
        GIT_TAG origin/device-bootloader
        GIT_SHALLOW 1
        SOURCE_DIR ${PROJECT_BINARY_DIR}/libopencm3
        CONFIGURE_COMMAND ""
        BINARY_DIR ${PROJECT_BINARY_DIR}/libopencm3
        BUILD_COMMAND ${CMAKE_MAKE_PROGRAM} -C ${PROJECT_BINARY_DIR}/libopencm3
        INSTALL_COMMAND ""
        BUILD_ALWAYS 1
)

add_library(${PROJECT_NAME})
target_link_directories(${PROJECT_NAME} PUBLIC ${PROJECT_BINARY_DIR}/libopencm3/lib)
target_link_libraries(${PROJECT_NAME} PlatformObjects CoreObjects DroneDevice)

#if(${CMAKE_SYSTEM_NAME} STREQUAL "Nuttx")
#    install(TARGETS ${PROJECT_NAME}
#            ARCHIVE DESTINATION lib)
#endif()

#if((${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86") AND ${PROJECT_NAME} STREQUAL ${CMAKE_PROJECT_NAME})
#    target_link_libraries(${PROJECT_NAME} PUBLIC coverage_config)
#    add_subdirectory(Tests)
#endif()
