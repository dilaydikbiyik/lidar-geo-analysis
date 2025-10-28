#include "app_controller.hpp"    // Kendi header'ı
#include "model/lidar.hpp"       // Filtreleme
#include "model/toml_parser.hpp" // TOML Okuyucu
#include "model/types.hpp"       // Tüm struct'lar (Point, Line, LidarScan, Intersection, SvgParams)
#include "model/ransac.hpp"      // RANSAC
#include "model/geometry.hpp"    // Geometri
#include "utils/cli.hpp"         // Komut Satırı Parametreleri
#include "view/svg_writer.hpp"   // SVG Yazıcı (Arkadaşının görevi)

#include <iostream>  // std::cout, std::cerr
#include <stdexcept> // std::runtime_error
#include <cstdlib>   // std::system (curl çağırmak için)
#include <optional>  // std::optional
#include <vector>    // std::vector
#include <string>    // std::string

// Constructor (Yapıcı): CLI parametrelerini alır
AppController::AppController(const CliParams& params)
    : m_params(params) // m_params değişkenini ilk değer atamasıyla doldur
{
    // Constructor'ın içi
    std::cout << "Controller baslatildi.\n";
    std::cout << "Okunacak dosya: " << m_params.inputPath << std::endl; // Bu satır ikinizde de vardı
}

// Ana uygulama akışı: 'run' fonksiyonu
void AppController::run() {
    std::cout << "Uygulama calisiyor...\n"; // Bu satır ikinizde de vardı

    // --- Görev B.2: URL ise indir ---
    std::string filePath = m_params.inputPath;
    if (filePath.rfind("http", 0) == 0) { // URL kontrolü
        std::cout << "[i] URL tespit edildi, indiriliyor...\n"; // Arkadaşından
        std::string local = "data/downloaded_scan.toml"; // Geçici dosya adı (Arkadaşından)
        std::string cmd; // Arkadaşından
// Platforma özel indirme komutu (Arkadaşından)
#ifdef _WIN32
        cmd = "powershell -Command \"Invoke-WebRequest -UseBasicParsing -Uri '" + filePath + "' -OutFile '" + local + "'\""; // Arkadaşından
        int rc = std::system(cmd.c_str()); // Arkadaşından
        if (rc != 0) { cmd = "curl -L -s -o \"" + local + "\" \"" + filePath + "\""; // Fallback curl (Arkadaşından)
            rc = std::system(cmd.c_str()); } // Arkadaşından
#else
        // macOS/Linux için curl komutu
        cmd = "curl -L -s -o '" + local + "' '" + filePath + "'"; // Arkadaşından
        int rc = std::system(cmd.c_str()); // Arkadaşından
#endif
        if (rc != 0) { // İndirme başarısız olursa hata ver (Arkadaşından)
             throw std::runtime_error("Dosya indirilemedi: " + filePath); // Arkadaşından
        }
        filePath = local; // Artık indirilen lokal dosyayı kullan (Arkadaşından)
        std::cout << "[i] Dosya '" << local << "' olarak indirildi.\n"; // Arkadaşından
    }

    // --- Görev 1: TOML oku ---
    // (Artık 'filePath' ya orijinal yol ya da indirilen dosyanın yolu)
    std::optional<LidarScan> scanData = loadScanFromFile(filePath); // İkinizde de var
    if (!scanData) { // Daha C++ tarzı kontrol (Arkadaşından)
        throw std::runtime_error("TOML dosyasi okunamadi veya islenemedi: " + filePath); // Hata mesajı güncellendi (Arkadaşından)
    }
    std::cout << "TOML Parser: " << scanData->ranges.size() << " adet 'range' degeri okundu." << std::endl; // İkinizde de var

    // --- Görev 2: Filtrele ve Dönüştür ---
    std::vector<Point> allPoints = filterAndConvertToPoints(*scanData); // *scanData kullanımı da geçerli (Arkadaşından)
    std::cout << "Lidar Filtre: " << allPoints.size() << " adet gecerli nokta bulundu." << std::endl; // İkinizde de var

    // --- Görev 3: Doğru Parçalarını Bul (Senin Görevin A.1) ---
    std::vector<Line> segments = findLinesRANSAC(
        allPoints,
        m_params.minInliers,
        m_params.epsilon,
        m_params.maxIters
    ); // İkinizde de var
    std::cout << "RANSAC (v2) tamamlandi. Toplam " << segments.size() << " adet dogru parcasi bulundu." << std::endl; // Log mesajı güncellendi (Senin kodundan)

    // --- Görev 4: Kesişimleri ve Açıları Hesapla (Senin Görevin A.2) ---
    std::vector<Intersection> intersections = findPhysicalIntersections(
        segments,
        m_params.angleThreshDeg // minAngleDeg (default: 60)
    ); // İkinizde de var
    std::cout << "Geometri Analizi: Toplam " << intersections.size()
              << " adet gecerli ('" << m_params.angleThreshDeg << " derece ustu') kesisim bulundu." << std::endl; // İkinizde de var

    // --- Görev 5a: Sonuçları Konsola Yazdır ---
    std::cout << "--- Kesisim Raporu ---\n"; // Başlık arkadaşından
    for (size_t i = 0; i < intersections.size(); ++i) { //
        const auto& k = intersections[i]; // Kısa değişken adı arkadaşından
        // Arkadaşının formatı biraz daha okunaklı
        std::cout << "#" << (i + 1) << " -> (" << k.position.x << ", " << k.position.y
                  << ")  angle=" << k.angleDeg
                  << " deg  dist=" << k.distanceToRobot << " m\n"; // Arkadaşından
    }

    // --- Görev 5b: Sonuçları SVG Dosyasına Yazdır (Arkadaşının Görevi B.1) ---
    // NOT: Bu kod, types.hpp veya cli.hpp'nin SvgParams/SVG argümanlarını
    // içerecek şekilde güncellenmesini gerektirir.
    SvgParams sp{ m_params.svgWidth, m_params.svgHeight, m_params.svgMargin }; // Arkadaşından
    saveToSVG(m_params.outSvg, allPoints, segments, intersections, sp); // Arkadaşından
    std::cout << "[i] SVG ciktisi su dosyaya kaydedildi: " << m_params.outSvg << "\n"; // Arkadaşından

    std::cout << "Uygulama tamamlandi.\n"; // İkinizde de var
}
// Senin kodundaki fazladan '}' parantezi kaldırıldı.