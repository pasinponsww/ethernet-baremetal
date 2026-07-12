add_library(sma STATIC sma.cc)

target_include_directories(sma PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)