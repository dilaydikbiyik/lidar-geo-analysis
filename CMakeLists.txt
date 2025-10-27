# Gerekli minimum CMake sürümünü belirtir
cmake_minimum_required(VERSION 3.10)

# Projenin adını belirler
project(LidarGeoAnalysis CXX)

# === HATA 2 ÇÖZÜMÜ: C++17 Standardını zorunlu kıl ===
# Bu, "std::optional" hatasını çözecek
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)


# === Manuel Kaynak Dosya Listesi ===
# Bu, "_main" hatasını (ve gelecekteki hataları) önler
set(PROJECT_SOURCES
        src/main.cpp

        src/controller/app_controller.cpp

        # BU SATIRIN LİSTEDE OLMASI GEREKİYOR:
        src/model/geometry.cpp

        src/model/lidar.cpp
        src/model/ransac.cpp
        src/model/toml_parser.cpp

        src/utils/cli.cpp

        src/view/console_view.cpp
        src/view/svg_writer.cpp
)


# Ana çalıştırılabilir dosyayı bu kaynaklardan oluştur
add_executable(proje_calistir ${PROJECT_SOURCES})

# === HATA 1 ÇÖZÜMÜ: Header Yolu Ekleme ===
# Bu, "utils/cli.hpp file not found" hatasını çözecek
target_include_directories(proje_calistir PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

# CMake'e testlerin olduğu "tests" adında bir alt klasör olduğunu söyle
add_subdirectory(tests)