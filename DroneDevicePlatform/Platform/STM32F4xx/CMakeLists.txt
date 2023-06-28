file(GLOB_RECURSE SOURCE_LIST "Platform/*.cpp")

add_library(PlatformObjects OBJECT ${SOURCE_LIST})
target_include_directories(PlatformObjects PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(PlatformObjects PUBLIC ${DRONEDEVICE_DIR}/Include)
target_link_libraries(PlatformObjects PUBLIC opencm3_stm32f4)
add_dependencies(PlatformObjects libopencm3)
