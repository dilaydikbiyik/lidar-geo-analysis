#pragma once
#include <string>
#include <optional>

struct CliParams {
    std::string inputPath;          // .toml dosya yolu (zorunlu)
    std::string outSvg = "data/output1.svg";
    double epsilon = 0.02;          // RANSAC mesafe eşiği
    int    minInliers = 8;          // asgari inlier
    int    maxIters   = 1000;       // RANSAC iter sayısı
    double angleThreshDeg = 60.0;   // doğrular arası açı eşiği (derece)
};

/// argv'den parametreleri okur. Hata/--help durumunda std::nullopt döner.
std::optional<CliParams> parse_cli(int argc, char* argv[]);

/// --help çıktısını üretir.
void print_cli_help(const char* exeName);
