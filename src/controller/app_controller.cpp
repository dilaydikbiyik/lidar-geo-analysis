#include "app_controller.hpp"
#include "model/lidar.hpp"
#include "model/toml_parser.hpp"
#include "model/ransac.hpp"
#include "model/geometry.hpp"
#include "utils/cli.hpp"
#include "view/svg_writer.hpp"
#include "view/console_view.hpp"
#include <stdexcept>
#include <cstdlib>

AppController::AppController(const CliParams& params)
    : m_params(params)
{
    ConsoleView::printControllerStart(m_params.inputPath);
}

// Ana uygulama akışı
void AppController::run() {
    ConsoleView::printAppRunning();

    std::string filePath = m_params.inputPath;
    std::string localPath = "data/downloaded_scan.toml";

    if (filePath.rfind("http", 0) == 0) {
        ConsoleView::printUrlDownload();

        std::string command;
        int result_code = 1;

#ifdef _WIN32
        command = "powershell -Command \"Invoke-WebRequest -UseBasicParsing -Uri '" + filePath + "' -OutFile '" + localPath + "'\"";
        result_code = std::system(command.c_str());

        if (result_code != 0) {
            ConsoleView::printUrlDownloadFallback();
            command = "curl -L -s -o \"" + localPath + "\" \"" + filePath + "\"";
            result_code = std::system(command.c_str());
        }
#else
        command = "curl -L -s -o '" + localPath + "' '" + filePath + "'";
        result_code = std::system(command.c_str());
#endif

        // Hata Kontrolü
        if (result_code != 0) {
             throw std::runtime_error("[HATA] Dosya indirilemedi: " + filePath +
                                      " | Lutfen URL'yi veya internet baglantinizi kontrol edin.");
        }

        filePath = localPath;
        ConsoleView::printUrlDownloadSuccess(localPath);
    }

    std::optional<LidarScan> scanData = loadScanFromFile(filePath);
    if (!scanData) {
        throw std::runtime_error("TOML dosyasi okunamadi veya islenemedi: " + filePath);
    }
    ConsoleView::printTomlResult(scanData->ranges.size());

    std::vector<Point> allPoints = filterAndConvertToPoints(*scanData);
    ConsoleView::printFilterResult(allPoints.size());

    std::vector<Line> segments = findLinesRANSAC(
        allPoints, m_params.minInliers, m_params.epsilon, m_params.maxIters
    );
    ConsoleView::printRansacResult(segments.size());

    // Geometrik Analiz
    std::vector<Intersection> intersections = findPhysicalIntersections(
        segments, m_params.angleThreshDeg
    );
    ConsoleView::printGeometryResult(intersections.size(), m_params.angleThreshDeg);

    ConsoleView::printFinalReport(intersections);

    SvgParams sp{ m_params.svgWidth, m_params.svgHeight, m_params.svgMargin };
    saveToSVG(m_params.outSvg, allPoints, segments, intersections, sp);

    ConsoleView::printSvgSuccess(m_params.outSvg);
    ConsoleView::printAppComplete();
}