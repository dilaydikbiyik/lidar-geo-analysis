#include "app_controller.hpp" // Kendi header dosyasını
#include "model/lidar.hpp"    // Filtreleme modülü
#include "model/toml_parser.hpp" // TOML okuyucu
#include "model/types.hpp"    // LidarScan, Point, Line, Intersection
#include "model/ransac.hpp"   // RANSAC fonksiyonu
#include "model/geometry.hpp" // Geometri fonksiyonu
#include <iostream>
#include <stdexcept> // Hata fırlatmak için (runtime_error)
#include <vector>    // std::vector için

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

    // --- YOL HARİTASI ---

    // 1. TOML dosyasını oku
    std::optional<LidarScan> scanData = loadScanFromFile(m_params.inputPath);
    if (!scanData.has_value()) {
        throw std::runtime_error("TOML dosyasi okunamadi.");
    }
    std::cout << "TOML Parser: " << scanData->ranges.size() << " adet 'range' degeri okundu." << std::endl;

    // 2. Noktaları filtrele ve dönüştür
    std::vector<Point> allPoints = filterAndConvertToPoints(scanData.value());
    std::cout << "Lidar Filtre: " << allPoints.size()
              << " adet gecerli nokta bulundu." << std::endl;

    // 3. Doğru parçalarını bul (Görev A.1)
    // 'segments' DEĞİŞKENİ BURADA TANIMLANIYOR:
    std::vector<Line> segments = findLinesRANSAC(
        allPoints,
        m_params.minInliers,
        m_params.epsilon,
        m_params.maxIters
    );
    // Bu log satırı 'segments'i kullanır
    std::cout << "RANSAC (v2) tamamlandi. Toplam " << segments.size() << " adet dogru parcasi bulundu." << std::endl;


    // 4. Kesişimleri ve açıları hespla (Görev A.2)
    // Bu satır da 'segments'i kullanır
    std::vector<Intersection> intersections = findPhysicalIntersections(
        segments,
        m_params.angleThreshDeg // minAngleDeg (default: 60)
    );
    std::cout << "Geometri Analizi: Toplam " << intersections.size()
              << " adet gecerli ('" << m_params.angleThreshDeg << " derece ustu') kesisim bulundu." << std::endl;

    // 5. Sonuçları yazdır
    std::cout << "--- Kesisim Raporu ---" << std::endl;
    for (size_t i = 0; i < intersections.size(); ++i) {
        std::cout << "Kesisim " << i+1 << ": ("
                  << intersections[i].position.x << ", " << intersections[i].position.y
                  << ") | Aci: " << (int)intersections[i].angleDeg
                  << " | Mesafe: " << intersections[i].distanceToRobot << "m" << std::endl;
    }

    std::cout << "Uygulama tamamlandi.\n";
}