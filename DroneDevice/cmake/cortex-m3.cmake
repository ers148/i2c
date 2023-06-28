set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR Cortex-M3)

# The toolchain prefix for all toolchain executables
set(CROSS_COMPILE arm-none-eabi-)

set(CMAKE_C_COMPILER "${CROSS_COMPILE}gcc")
set(CMAKE_CXX_COMPILER "${CROSS_COMPILE}g++")

set(FLAGS_CPU
        "-nostartfiles"
        "-ffunction-sections"
        "-fdata-sections"
        "-mthumb"
        "-mcpu=cortex-m3"
)
string(REPLACE ";" " " FLAGS_CPU "${FLAGS_CPU}")

set(CMAKE_C_FLAGS_INIT "${FLAGS_CPU}")
set(CMAKE_CXX_FLAGS_INIT "${FLAGS_CPU}")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)
