if(PLATFORM STREQUAL "STM32F0xx")
    set(LIBOPENCM3_PLATFORM STM32F0)
elseif(PLATFORM STREQUAL "STM32F1xx")
    set(LIBOPENCM3_PLATFORM STM32F1)
elseif(PLATFORM STREQUAL "STM32F4xx")
    set(LIBOPENCM3_PLATFORM STM32F4)
    set(COMMON_SOURCE_LIST
        "${CMAKE_CURRENT_SOURCE_DIR}/STM32/Platform/CdcAcm.cpp"
    )
elseif(PLATFORM STREQUAL "STM32F76x")
    set(LIBOPENCM3_PLATFORM STM32F7)
    set(COMMON_SOURCE_LIST
        "${CMAKE_CURRENT_SOURCE_DIR}/STM32/Platform/CdcAcm.cpp"
    )
endif()

if(PLATFORM MATCHES "STM32F0xx|STM32F1xx|STM32F4xx|STM32F76x")
    add_subdirectory(Cortex-M)
endif()
add_subdirectory("${PLATFORM}")

if(COMMON_SOURCE_LIST)
    target_sources(PlatformObjects PRIVATE ${COMMON_SOURCE_LIST})
endif()

target_include_directories(PlatformObjects PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/STM32")
target_compile_definitions(PlatformObjects PUBLIC
    $<TARGET_PROPERTY:CoreObjects,INTERFACE_COMPILE_DEFINITIONS>)
target_include_directories(PlatformObjects PUBLIC
    $<TARGET_PROPERTY:CoreObjects,INTERFACE_INCLUDE_DIRECTORIES>)
target_include_directories(PlatformObjects SYSTEM PUBLIC
    $<TARGET_PROPERTY:CoreObjects,INTERFACE_SYSTEM_INCLUDE_DIRECTORIES>)
