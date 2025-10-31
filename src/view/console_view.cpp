#include "console_view.hpp"
#include <iostream>
#include <iomanip>

namespace ConsoleView {

    void printControllerStart(const std::string& inputPath) {
        std::cout << "Controller baslatildi.\n";
        std::cout << "Okunacak dosya: " << inputPath << std::endl;
    }

    void printAppRunning() {
        std::cout << "Uygulama calisiyor...\n";
    }

    void printUrlDownload() {
        std::cout << "[i] URL tespit edildi, indiriliyor...\n";
    }

    void printUrlDownloadFallback() {
        std::cout << "[i] PowerShell basarisiz oldu, 'curl' ile deneniyor..." << std::endl;
    }

    void printUrlDownloadSuccess(const std::string& localPath) {
        std::cout << "[i] Dosya '" << localPath << "' olarak basariyla indirildi.\n";
    }

    void printTomlResult(size_t rangeCount) {
        std::cout << "TOML Parser: " << rangeCount << " adet 'range' degeri okundu." << std::endl;
    }

    void printFilterResult(size_t pointCount) {
        std::cout << "Lidar Filtre: " << pointCount << " adet gecerli nokta bulundu." << std::endl;
    }

    void printRansacResult(size_t segmentCount) {
        std::cout << "RANSAC (v2) tamamlandi. Toplam " << segmentCount << " adet dogru parcasi bulundu." << std::endl;
    }

    void printGeometryResult(size_t intersectionCount, double angleThresh) {
        std::cout << "Geometri Analizi: Toplam " << intersectionCount
                  << " adet gecerli ('" << angleThresh << " derece ustu') kesisim bulundu." << std::endl;
    }

    void printFinalReport(const std::vector<Intersection>& intersections) {
        std::cout << "--- Kesisim Raporu ---\n";
        // Raporu yazdır
        for (size_t i = 0; i < intersections.size(); ++i) {
            const auto& k = intersections[i];
            std::cout << "#" << (i + 1) << " -> (" << k.position.x << ", " << k.position.y
                      << ")  angle=" << std::fixed << std::setprecision(4) << k.angleDeg
                      << " deg  dist=" << std::fixed << std::setprecision(4) << k.distanceToRobot << " m\n";
        }
    }

    void printSvgSuccess(const std::string& outputPath) {
        std::cout << "[i] SVG ciktisi su dosyaya kaydedildi: " << outputPath << "\n";
    }

    void printAppComplete() {
        std::cout << "Uygulama tamamlandi.\n";
    }

} // namespace ConsoleView