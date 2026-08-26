add_library(utils STATIC
    reg_helpers.cc
    delay.cc
    dcache.cc
)

target_include_directories(utils PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# dcache.cc needs stm32h723xx.h (CMSIS SCB_*DCache* intrinsics) on-device;
# it's guarded out (#ifdef __arm__) for native/host builds, which don't
# build the st_h7_support/cmsis targets at all.
if (TARGET_DEVICE MATCHES "^(STM32H7)[0-9]+")
    target_link_libraries(utils PUBLIC st_h7_support)
endif()

