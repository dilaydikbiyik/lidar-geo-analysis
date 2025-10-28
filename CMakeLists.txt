# ================= Top-level CMakeLists.txt =================

# Gerekli minimum CMake sürümünü belirtir
cmake_minimum_required(VERSION 3.10)

# Projenin adını belirler
project(LidarGeoAnalysis CXX)

# === C++17 Standardını zorunlu kıl ===
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True) # ON yerine True kullanmak daha yaygındır


# === Ortak Kütüphane: Ana Mantık ===
# Projenin ana mantığını içeren tüm .cpp dosyaları burada listelenir.
# Bu, main.cpp ve console_view.cpp HARİÇ tüm dosyaları içerir.
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
        # View (SVG kısmı kütüphanede, Console kısmı executable'da)
        src/view/svg_writer.cpp
)

# === Header Yolu Ekleme ===
# lidar_core kütüphanesinin ve onu kullanan diğer kodların
# #include "model/types.hpp" gibi dosyaları bulabilmesi için.
target_include_directories(lidar_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)


# === Ana Çalıştırılabilir Dosya ===
# Sadece programın giriş noktası (main) ve konsol çıktısını içerir.
add_executable(proje_calistir
        src/main.cpp
        src/view/console_view.cpp # Konsol çıktısı burada
)

# === Kütüphaneyi Çalıştırılabilir Dosyaya Bağlama ===
# proje_calistir'in, lidar_core içindeki fonksiyonları
# (findLinesRANSAC, loadScanFromFile vb.) kullanabilmesi için.
target_link_libraries(proje_calistir PRIVATE lidar_core)


# === Testler Alt Dizini ===
# CMake'e testlerin olduğu "tests" adında bir alt klasör olduğunu söyle
# (Bu satır iki versiyonda da aynıydı)
add_subdirectory(tests)