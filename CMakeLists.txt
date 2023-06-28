cmake_minimum_required(VERSION 3.13)
project(i2c_test CXX)

option(DFU_LAYOUT "Use memory layout for bootloader instead of debug layout." OFF)
option(USE_LTO "Enable Link Time Optimization." OFF)
option(USE_WDT "Enable watchdog." OFF)
option(FACTORY "Factory test firmware." OFF)

set(FLAGS_COMMON
    "-pedantic"
    "-Wall"
    "-Wcast-align"
    "-Wcast-qual"
    "-Wconversion"
    "-Wextra"
    "-Wshadow"
    "-Wvla"
    "-Wuninitialized"
    "-Winit-self"
    "-Wdouble-promotion"
    "-Wstrict-aliasing"
    "-Wno-unused-local-typedefs"
    "-Wduplicated-cond"
    "-Wsuggest-override"
    "-Wno-psabi"
    "-Wold-style-cast"
    "-Woverloaded-virtual"
    "-Wno-stringop-truncation"
    "-fno-exceptions"
    "-fsingle-precision-constant"
    "-fno-rtti"
)
string(REPLACE ";" " " FLAGS_COMMON "${FLAGS_COMMON}")

if(${USE_LTO})
    set(FLAGS_COMMON "${FLAGS_COMMON} -flto -ffat-lto-objects")
endif()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${FLAGS_CPU} ${FLAGS_COMMON}")
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")
set(CMAKE_EXECUTABLE_SUFFIX ".elf")

# Board files should be processed first
add_subdirectory(Board)

# Add external libraries
add_subdirectory(DroneDevicePlatform)
add_subdirectory(DroneDevice)

# Add common files at the last step
add_subdirectory(Sources)

add_executable(${PROJECT_NAME} $<TARGET_OBJECTS:BoardObjects> $<TARGET_OBJECTS:CommonObjects>)

target_include_directories(BoardObjects PRIVATE $<TARGET_PROPERTY:CommonObjects,INTERFACE_INCLUDE_DIRECTORIES>)
target_link_libraries(${PROJECT_NAME} PRIVATE DroneDevicePlatform)

add_custom_target(
    ${PROJECT_NAME}.bin ALL
    COMMAND ${CMAKE_OBJCOPY} ${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX} -Obinary ${PROJECT_NAME}.bin
    DEPENDS ${PROJECT_NAME}
)

add_custom_target(
    ${PROJECT_NAME}.hex ALL
    COMMAND ${CMAKE_OBJCOPY} ${PROJECT_NAME}${CMAKE_EXECUTABLE_SUFFIX} -Oihex ${PROJECT_NAME}.hex
    DEPENDS ${PROJECT_NAME}
)

if(BUILD_DOCS)
    find_package(Sphinx REQUIRED)
    add_custom_target(sphinx ALL)

    if (SPHINX_FOUND)
        include(AddSphinxTarget)
        add_sphinx_target(html battery)
    endif()
endif()
