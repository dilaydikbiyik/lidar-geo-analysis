# Gerekli minimum CMake sürümünü belirtir
cmake_minimum_required(VERSION 3.10)

# Projenin adını belirler
project(LidarGeoAnalysis CXX)

# src klasöründeki tüm .cpp ve .hpp dosyalarını bulur
# Not: main.cpp dosyanın src/main.cpp olduğundan emin ol!
file(GLOB_RECURSE PROJECT_SOURCES 
    "src/*.cpp"
    "src/*.hpp"
)

# Ana çalıştırılabilir dosyayı (proje_calistir) bu kaynaklardan oluştur
add_executable(proje_calistir ${PROJECT_SOURCES})

# CMake'e testlerin olduğu "tests" adında bir alt klasör olduğunu söyle
add_subdirectory(tests)