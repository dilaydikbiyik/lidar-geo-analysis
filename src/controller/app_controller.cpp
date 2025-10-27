#include "app_controller.hpp" // Kendi header dosyasını
#include "model/lidar.hpp"    // Yeni filtreleme modülünü
#include "model/toml_parser.hpp" // TOML okuyucuyu
#include "model/types.hpp"    // LidarScan, Point vb.
#include <iostream>
#include <stdexcept>// Hata fırlatmak için (runtime_error)
#include "model/ransac.hpp"

// Constructor (Yapıcı): CLI parametrelerini alır
AppController::AppController(const CliParams& params)
    : m_params(params) // m_params değişkenini ilk değer atamasıyla doldur
{
    // Constructor'ın içi
    std::cout << "Controller baslatildi.\n";
    std::cout << "Okunacak dosya: " << m_params.inputPath << std::endl;
}

// Ana uygulama akışı: 'run' fonksiyonu
void AppController::run() {
    std::cout << "Uygulama calisiyor...\n";

    // 1. TOML dosyasını oku
    std::optional<LidarScan> scanData = loadScanFromFile(m_params.inputPath);
    if (!scanData.has_value()) {
        throw std::runtime_error("TOML dosyasi okunamadi.");
    }
    std::cout << "TOML Parser: " << scanData->ranges.size() << " adet 'range' degeri okundu." << std::endl;

    // 2. Noktaları filtrele ve dönüştür
    std::vector<Point> allPoints = filterAndConvertToPoints(scanData.value());
    std::cout << "Lidar Filtre: " << allPoints.size() << " adet gecerli nokta bulundu." << std::endl;

    // 3. Doğruları bul (YENİ GÜNCELLENDİ)
    // RANSAC parametrelerini 'cli.hpp'den (m_params) alıyoruz
    std::vector<Line> lines = findLinesRANSAC(
        allPoints,
        m_params.minInliers,      // minInliers (default: 8)
        m_params.epsilon,         // distanceThreshold (default: 0.02)
        m_params.maxIters         // maxIterations (default: 1000)
    );

    // 4. Kesişimleri ve açıları hespla (model/geometry)
    // (SIRADAKİ ADIM BURASI)

    // ... (diğer adımlar) ...

    std::cout << "Uygulama tamamlandi.\n";
}
// Bu, dosyanın sonu olmalı.