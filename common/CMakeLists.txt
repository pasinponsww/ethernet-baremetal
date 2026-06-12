add_library(common INTERFACE)

# Expose the common directory to any target that links against 'common'
target_include_directories(common INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

# Uncomment and change to a STATIC library when you add .c/.cpp files here:
# add_library(common STATIC
#     # my_common_file.c
# )
# target_include_directories(common PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
