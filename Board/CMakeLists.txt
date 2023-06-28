# Generate version and board descriptions
include(GenerateVersion)

# Gather board files
message("${CMAKE_CURRENT_SOURCE_DIR}/${BOARD}")
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${BOARD} ${CMAKE_MODULE_PATH})
include(Board)
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/Platform/${PLATFORM} ${CMAKE_MODULE_PATH})
include(Platform)

extract_git_version(VERSION_SW_MAJOR VERSION_SW_MINOR VERSION_SW_HASH VERSION_SW_REVISION)
extract_hw_version("${VERSION_HW}" VERSION_HW_MAJOR VERSION_HW_MINOR)

# Generate board and version files from templates
set(BOARD_HPP_FILE "${PROJECT_BINARY_DIR}/Generated/Board.hpp")
configure_file("${PROJECT_SOURCE_DIR}/Templates/BoardTemplate.hpp" ${BOARD_HPP_FILE})
set(VERSION_HPP_FILE "${PROJECT_BINARY_DIR}/Generated/Version.hpp")
configure_file("${PROJECT_SOURCE_DIR}/Templates/VersionTemplate.hpp" ${VERSION_HPP_FILE})
set(VERSION_CPP_FILE "${PROJECT_BINARY_DIR}/Generated/Version.cpp")
configure_file("${PROJECT_SOURCE_DIR}/Templates/VersionTemplate.cpp" ${VERSION_CPP_FILE})

# Make library
file(GLOB_RECURSE SOURCE_LIST "${BOARD}/*.cpp")
add_library(BoardObjects OBJECT ${SOURCE_LIST} ${VERSION_CPP_FILE})
target_include_directories(BoardObjects PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${PROJECT_BINARY_DIR}/Generated)
target_link_libraries(BoardObjects PUBLIC DroneDevicePlatform)


if (USE_WDT OR REQUIRE_WDT)
	target_compile_definitions(BoardObjects PRIVATE -DCONFIG_USE_WDT)
endif()

if (ARTERY)
	target_compile_definitions(BoardObjects PRIVATE -DCONFIG_ARTERY)
endif()

if (FACTORY)
	target_compile_definitions(BoardObjects PRIVATE -DCONFIG_FACTORY)	
endif()




