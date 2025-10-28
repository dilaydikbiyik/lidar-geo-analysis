#include "app_controller.hpp"    // Kendi header'ı
#include "model/lidar.hpp"       // Filtreleme
#include "model/toml_parser.hpp" // TOML Okuyucu
#include "model/types.hpp"       // Tüm struct'lar (Point, Line, LidarScan, Intersection, SvgParams)
#include "model/ransac.hpp"      // RANSAC
#include "model/geometry.hpp"    // Geometri
#include "utils/cli.hpp"         // Komut Satırı Parametreleri
#include "view/svg_writer.hpp"   // SVG Yazıcı
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
    std::cout << "Uygulama calisiyor...\n";

    // 1. Girdi yolunu al ve URL olup olmadığını kontrol et
    std::string filePath = m_params.inputPath;
    std::string localPath = "data/downloaded_scan.toml"; // İndirilen dosyanın kaydedileceği yerel yol

    // Girdi 'http' ile başlıyorsa, bu bir URL'dir
    if (filePath.rfind("http", 0) == 0) {
        std::cout << "[i] URL tespit edildi, indiriliyor...\n";

        std::string command;
        int result_code = 1; // Komutun başarı durumunu tutar (0 = başarılı)

// 2. Platforma özel indirme komutunu çalıştır (Windows vs. macOS/Linux)
#ifdef _WIN32
        // Windows: Önce PowerShell komutunu (Invoke-WebRequest) dene
        command = "powershell -Command \"Invoke-WebRequest -UseBasicParsing -Uri '" + filePath + "' -OutFile '" + localPath + "'\"";
        result_code = std::system(command.c_str());

        if (result_code != 0) {
            // PowerShell başarısız olursa (örn. yüklü değilse), 'curl' komutunu dene
            std::cout << "[i] PowerShell basarisiz oldu, 'curl' ile deneniyor..." << std::endl;
            command = "curl -L -s -o \"" + localPath + "\" \"" + filePath + "\"";
            result_code = std::system(command.c_str());
        }
#else
        // macOS / Linux: Doğrudan 'curl' komutunu kullan
        command = "curl -L -s -o '" + localPath + "' '" + filePath + "'";
        result_code = std::system(command.c_str());
#endif

        // 3. Hata Kontrolü: İndirme işlemi başarılı oldu mu?
        if (result_code != 0) {
             // 'result_code' 0 değilse, komut başarısız olmuştur (404, ağ hatası vb.)
             // Programı hemen durdur ve bir hata fırlat.
             throw std::runtime_error("[HATA] Dosya indirilemedi: " + filePath +
                                      " | Lutfen URL'yi veya internet baglantinizi kontrol edin.");
        }

        // İndirme başarılıysa, TOML parser'a bu yeni yerel yolu ver
        filePath = localPath;
        std::cout << "[i] Dosya '" << localPath << "' olarak basariyla indirildi.\n";
    }

    // --- (URL indirme kod bloğu buradan önce gelir) ---

    // 4. TOML dosyasını oku ve verileri işle
    // (Bu noktada 'filePath' ya orijinal yoldur ya da indirilen dosyanın yoludur)
    std::optional<LidarScan> scanData = loadScanFromFile(filePath);

    // std::optional kullanarak C++'a daha uygun bir hata kontrolü
    if (!scanData) {
        throw std::runtime_error("TOML dosyasi okunamadi veya islenemedi: " + filePath);
    }
    std::cout << "TOML Parser: " << scanData->ranges.size() << " adet 'range' degeri okundu." << std::endl;

    // 5. Filtrele ve Kartezyen Koordinatlara Dönüştür (İster 1)
    // (NaN, min/max dışı değerler filtrelenir ve kutupsaldan kartezyene geçilir)
    std::vector<Point> allPoints = filterAndConvertToPoints(*scanData);
    std::cout << "Lidar Filtre: " << allPoints.size() << " adet gecerli nokta bulundu." << std::endl;

    // 6. Doğru Parçalarını Bul (İster 2)
    // (Filtrelenmiş nokta bulutu üzerinde RANSAC algoritmasını çalıştır)
    std::vector<Line> segments = findLinesRANSAC(
        allPoints,
        m_params.minInliers,
        m_params.epsilon,
        m_params.maxIters
    );
    std::cout << "RANSAC (v2) tamamlandi. Toplam " << segments.size() << " adet dogru parcasi bulundu." << std::endl;

    // 7. Geometrik Analiz
    // (Tespit edilen doğru parçaları arasında fiziksel kesişimleri ve açıları bul)
    std::vector<Intersection> intersections = findPhysicalIntersections(
        segments,
        m_params.angleThreshDeg // CLI'dan gelen minimum açı eşiği (örn: 60 derece)
    );
    std::cout << "Geometri Analizi: Toplam " << intersections.size()
              << " adet gecerli ('" << m_params.angleThreshDeg << " derece ustu') kesisim bulundu." << std::endl;

    // 8. Raporlama
    // Adım 8a: Sonuçları konsola metin olarak yazdır
    std::cout << "--- Kesisim Raporu ---\n";
    for (size_t i = 0; i < intersections.size(); ++i) {
        const auto& k = intersections[i]; // Okunabilirlik için kısa değişken adı

        // Raporu formatlı bir şekilde yazdır
        std::cout << "#" << (i + 1) << " -> (" << k.position.x << ", " << k.position.y
                  << ")  angle=" << k.angleDeg
                  << " deg  dist=" << k.distanceToRobot << " m\n";
    }

    // Adım 8b: Sonuçları SVG dosyasına grafiksel olarak yazdır
    // (SVG parametrelerini CLI'dan alarak ayarla)
    SvgParams sp{ m_params.svgWidth, m_params.svgHeight, m_params.svgMargin };
    saveToSVG(m_params.outSvg, allPoints, segments, intersections, sp);

    std::cout << "[i] SVG ciktisi su dosyaya kaydedildi: " << m_params.outSvg << "\n";

    std::cout << "Uygulama tamamlandi.\n";
}