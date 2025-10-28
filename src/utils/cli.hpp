#pragma once
#include <optional>
#include <string>

struct CliParams {
    // Girdi / çıktı
    std::string inputPath;               // .toml dosya yolu veya URL (zorunlu)
    std::string outSvg   = "data/output1.svg"; // Varsayılan yol kullanıldı

    // RANSAC / Geometri
    double epsilon       = 0.02;        // RANSAC mesafe eşiği
    int    minInliers    = 8;           // RANSAC asgari inlier
    int    maxIters      = 1000;        // RANSAC iter sayısı
    double angleThreshDeg= 60.0;        // Doğru çifti açı eşiği (deg)

    // SVG görünüm
    int svgWidth  = 1200;               // SVG genişliği (piksel)
    int svgHeight = 900;                // SVG yüksekliği (piksel)
    int svgMargin = 40;                 // SVG kenar boşluğu (piksel)
};

/// argv'den parametreleri okur. Hata/--help durumunda std::nullopt döner.
std::optional<CliParams> parse_cli(int argc, char* argv[]);

/// --help çıktısını üretir.
void print_cli_help(const char* exeName);