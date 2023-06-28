cmake_minimum_required(VERSION 3.7)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/modules ${CMAKE_MODULE_PATH})
project(DroneDevice C CXX)

enable_testing()

option(USE_LTO "Enable Link Time Optimization." OFF)
option(DRONEDDEVICE_CODE_COVERAGE "Enable coverage reporting" OFF)

# Code Coverage Configuration
add_library(coverage_config INTERFACE)
if(DRONEDDEVICE_CODE_COVERAGE AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  # Add required flags (GCC & LLVM/Clang)
  target_compile_options(coverage_config INTERFACE
    -O0        # no optimization
    -g         # generate debug info
    --coverage # sets all required flags
  )
  if(CMAKE_VERSION VERSION_GREATER_EQUAL 3.13)
    target_link_options(coverage_config INTERFACE --coverage)
  else()
    target_link_libraries(coverage_config INTERFACE --coverage)
  endif()
endif()

include(ExtractValidFlags)
extract_valid_cxx_flags(PUBLIC_FLAGS
        -mfp16-format=ieee
)

if(${USE_LTO})
    set(FLAGS_COMMON "${FLAGS_COMMON} -flto -ffat-lto-objects")
endif()

string(REPLACE " " ";" PUBLIC_FLAGS "${PUBLIC_FLAGS}")

add_subdirectory(Libs)
add_library(${PROJECT_NAME} $<TARGET_OBJECTS:libcanard_sources>)
target_compile_options(${PROJECT_NAME} PRIVATE ${FLAGS_COMMON})
target_compile_options(${PROJECT_NAME} PUBLIC ${PUBLIC_FLAGS})
target_include_directories(${PROJECT_NAME} PUBLIC "${PROJECT_SOURCE_DIR}/Libs")
target_include_directories(${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Include>)

if(${CMAKE_SYSTEM_NAME} STREQUAL "Nuttx")
    install(TARGETS ${PROJECT_NAME}
            ARCHIVE DESTINATION lib)
endif()

if((${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86") AND ${PROJECT_NAME} STREQUAL ${CMAKE_PROJECT_NAME})
    target_link_libraries(${PROJECT_NAME} PUBLIC coverage_config)
    add_subdirectory(Tests)
endif()
