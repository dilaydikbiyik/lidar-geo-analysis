// Gerekli header'lar...
#include "app_controller.hpp"
#include "model/lidar.hpp"
#include "model/toml_parser.hpp"
#include "model/ransac.hpp"
#include "model/geometry.hpp"
// ...diğer model header'ları...
#include "utils/cli.hpp"
#include "view/svg_writer.hpp"   // SVG View
#include "view/console_view.hpp" // YENİ EKLENDİ: Console View
#include <stdexcept> // std::runtime_error
#include <cstdlib>   // std::system
// #include <iostream> // ARTIK GEREKLİ DEĞİL!

// Constructor (Yapıcı)
AppController::AppController(const CliParams& params)
    : m_params(params)
{
    // Sadece View'ı çağırır
    ConsoleView::printControllerStart(m_params.inputPath);
}

// Ana uygulama akışı
void AppController::run() {
    ConsoleView::printAppRunning();

    // 1. Girdi yolunu al ve URL olup olmadığını kontrol et
    std::string filePath = m_params.inputPath;
    std::string localPath = "data/downloaded_scan.toml";

    if (filePath.rfind("http", 0) == 0) {
        ConsoleView::printUrlDownload(); // View'ı çağır

        std::string command;
        int result_code = 1;

#ifdef _WIN32
        command = "powershell -Command \"Invoke-WebRequest -UseBasicParsing -Uri '" + filePath + "' -OutFile '" + localPath + "'\"";
        result_code = std::system(command.c_str());

        if (result_code != 0) {
            ConsoleView::printUrlDownloadFallback(); // View'ı çağır
            command = "curl -L -s -o \"" + localPath + "\" \"" + filePath + "\"";
            result_code = std::system(command.c_str());
        }
#else
        command = "curl -L -s -o '" + localPath + "' '" + filePath + "'";
        result_code = std::system(command.c_str());
#endif

        // 3. Hata Kontrolü (Hata fırlatma Controller'ın işidir)
        if (result_code != 0) {
             throw std::runtime_error("[HATA] Dosya indirilemedi: " + filePath +
                                      " | Lutfen URL'yi veya internet baglantinizi kontrol edin.");
        }

        filePath = localPath;
        ConsoleView::printUrlDownloadSuccess(localPath); // View'ı çağır
    }

    // 4. TOML dosyasını oku ve verileri işle
    std::optional<LidarScan> scanData = loadScanFromFile(filePath);
    if (!scanData) {
        throw std::runtime_error("TOML dosyasi okunamadi veya islenemedi: " + filePath);
    }
    ConsoleView::printTomlResult(scanData->ranges.size()); // View'ı çağır

    // 5. Filtrele ve Kartezyen Koordinatlara Dönüştür
    std::vector<Point> allPoints = filterAndConvertToPoints(*scanData);
    ConsoleView::printFilterResult(allPoints.size()); // View'ı çağır

    // 6. Doğru Parçalarını Bul
    std::vector<Line> segments = findLinesRANSAC(
        allPoints, m_params.minInliers, m_params.epsilon, m_params.maxIters
    );
    ConsoleView::printRansacResult(segments.size()); // View'ı çağır

    // 7. Geometrik Analiz
    std::vector<Intersection> intersections = findPhysicalIntersections(
        segments, m_params.angleThreshDeg
    );
    ConsoleView::printGeometryResult(intersections.size(), m_params.angleThreshDeg); // View'ı çağır

    // 8. Raporlama
    // Adım 8a: Sonuçları konsola metin olarak yazdır
    ConsoleView::printFinalReport(intersections); // View'ı çağır

    // Adım 8b: Sonuçları SVG dosyasına grafiksel olarak yazdır
    SvgParams sp{ m_params.svgWidth, m_params.svgHeight, m_params.svgMargin };
    saveToSVG(m_params.outSvg, allPoints, segments, intersections, sp); // Diğer View'ı (SVG) çağır

    ConsoleView::printSvgSuccess(m_params.outSvg); // View'ı çağır
    ConsoleView::printAppComplete(); // View'ı çağır
}