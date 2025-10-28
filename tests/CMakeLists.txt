# ============== tests/CMakeLists.txt ==============

cmake_minimum_required(VERSION 3.10)

add_executable(unit_tests
        test_geometry.cpp
        test_toml.cpp
)

# Testleri ortak kütüphaneye bağla
target_link_libraries(unit_tests PRIVATE lidar_core)
target_include_directories(unit_tests PUBLIC ${CMAKE_SOURCE_DIR}/src)

enable_testing()
add_test(NAME unit_tests COMMAND unit_tests)