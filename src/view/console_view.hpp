#pragma once

#include <string>
#include <vector>
#include "model/types.hpp" // Intersection'ı bilmesi için

// Tüm konsol çıktı fonksiyonlarını bir 'namespace' altında
namespace ConsoleView {

    void printControllerStart(const std::string& inputPath);
    void printAppRunning();
    void printUrlDownload();
    void printUrlDownloadFallback();
    void printUrlDownloadSuccess(const std::string& localPath);
    void printTomlResult(size_t rangeCount);
    void printFilterResult(size_t pointCount);
    void printRansacResult(size_t segmentCount);
    void printGeometryResult(size_t intersectionCount, double angleThresh);
    void printFinalReport(const std::vector<Intersection>& intersections);
    void printSvgSuccess(const std::string& outputPath);
    void printAppComplete();

} // namespace ConsoleView