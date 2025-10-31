cmake_minimum_required(VERSION 3.10)

project(LidarGeoAnalysis CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Ortak Kütüphane
add_library(lidar_core
        # Controller
        src/controller/app_controller.cpp
        # Model
        src/model/geometry.cpp
        src/model/lidar.cpp
        src/model/ransac.cpp
        src/model/toml_parser.cpp
        # Utils
        src/utils/cli.cpp
        # View
        src/view/svg_writer.cpp
        src/view/console_view.cpp
)

target_include_directories(lidar_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(proje_calistir
        src/main.cpp
)

target_link_libraries(proje_calistir PRIVATE lidar_core)

add_subdirectory(tests)