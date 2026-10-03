# Host unit tests, one subdirectory per unit under test. Each uses the
# add_tests(<link_lib> <test_name>...) helper from cmake/build-util.cmake,
# which builds <test_name>.cc against GoogleTest and registers it with ctest.
add_subdirectory(modbus)
add_subdirectory(packet)
