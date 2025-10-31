#pragma once
#include <optional>
#include <string>

struct CliParams {
    // Girdi / çıktı
    std::string inputPath;
    std::string outSvg   = "data/output1.svg";

    // RANSAC / Geometri
    double epsilon       = 0.02;
    int    minInliers    = 8;
    int    maxIters      = 2000;
    double angleThreshDeg= 60.0;

    // SVG görünüm
    int svgWidth  = 1200;
    int svgHeight = 900;
    int svgMargin = 40;
};

std::optional<CliParams> parse_cli(int argc, char* argv[]);

void print_cli_help(const char* exeName);