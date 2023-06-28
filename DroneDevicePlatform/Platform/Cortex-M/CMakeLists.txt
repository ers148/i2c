file(GLOB_RECURSE SOURCE_LIST "Platform/*.cpp")

add_library(CoreObjects OBJECT ${SOURCE_LIST})
target_compile_definitions(CoreObjects PUBLIC ${LIBOPENCM3_PLATFORM})
target_include_directories(CoreObjects PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(CoreObjects SYSTEM PUBLIC ${PROJECT_BINARY_DIR}/libopencm3/include)
add_dependencies(CoreObjects libopencm3)
