string(REPLACE " " ";" FLAGS_CPU "${FLAGS_CPU}")
file(GLOB SOURCE_FILES "libcanard/*.c")
add_library(libcanard_sources OBJECT ${SOURCE_FILES})
target_compile_options(libcanard_sources PUBLIC ${FLAGS_CPU})
