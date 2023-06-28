# Make library
file(GLOB_RECURSE SOURCE_LIST "*.cpp")
add_library(CommonObjects OBJECT ${SOURCE_LIST})
target_include_directories(CommonObjects PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(CommonObjects PUBLIC DroneDevice)
