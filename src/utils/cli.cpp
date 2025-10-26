#include "utils/cli.hpp"
#include <iostream>
#include <cstring>   // std::strcmp, std::strncmp
#include <cstdlib>   // std::strtod, std::strtol

static bool str_starts_with(const char* s, const char* prefix) {
    return std::strncmp(s, prefix, std::strlen(prefix)) == 0;
}

void print_cli_help(const char* exeName) {
    std::cout
      << "Usage:\n  " << exeName << " --input <file.toml> [options]\n\n"
      << "Required:\n"
      << "  --input <path>           LIDAR veri TOML dosyasi\n\n"
      << "Options:\n"
      << "  --out <file.svg>         Cikti SVG yolu (default: data/output1.svg)\n"
      << "  --epsilon <float>        RANSAC mesafe esigi (default: 0.02)\n"
      << "  --min-inliers <int>      Minimum inlier (default: 8)\n"
      << "  --max-iters <int>        RANSAC iter sayisi (default: 1000)\n"
      << "  --angle-thresh <deg>     Aci esigi derece (default: 60)\n"
      << "  -h, --help               Bu yardimi goster\n\n"
      << "Positional (kisa yol):\n"
      << "  Sadece TOML yolu verirseniz --input yerine gecer: \n"
      << "    " << exeName << " data/lidar1.toml\n";
}

std::optional<CliParams> parse_cli(int argc, char* argv[]) {
    CliParams p;

    if (argc <= 1) {
        print_cli_help(argv[0]);
        return std::nullopt;
    }

    // Positional: ilk arguman .toml ise input say
    if (argc >= 2 && !str_starts_with(argv[1], "-")) {
        p.inputPath = argv[1];
    }

    for (int i = 1; i < argc; ++i) {
        const char* a = argv[i];

        if ((std::strcmp(a, "-h") == 0) || (std::strcmp(a, "--help") == 0)) {
            print_cli_help(argv[0]);
            return std::nullopt;
        } else if (std::strcmp(a, "--input") == 0 && i + 1 < argc) {
            p.inputPath = argv[++i];
        } else if (std::strcmp(a, "--out") == 0 && i + 1 < argc) {
            p.outSvg = argv[++i];
        } else if (std::strcmp(a, "--epsilon") == 0 && i + 1 < argc) {
            p.epsilon = std::strtod(argv[++i], nullptr);
        } else if (std::strcmp(a, "--min-inliers") == 0 && i + 1 < argc) {
            p.minInliers = static_cast<int>(std::strtol(argv[++i], nullptr, 10));
        } else if (std::strcmp(a, "--max-iters") == 0 && i + 1 < argc) {
            p.maxIters = static_cast<int>(std::strtol(argv[++i], nullptr, 10));
        } else if (std::strcmp(a, "--angle-thresh") == 0 && i + 1 < argc) {
            p.angleThreshDeg = std::strtod(argv[++i], nullptr);
        } else if (!str_starts_with(a, "-") && p.inputPath.empty()) {
            // positional fallback (ilk non-flag görüleni input say)
            p.inputPath = a;
        }
        // bilinmeyen flag'leri şimdilik yok sayıyoruz
    }

    if (p.inputPath.empty()) {
        std::cerr << "Hata: --input <file.toml> zorunlu (veya ilk arguman olarak verin)\n\n";
        print_cli_help(argv[0]);
        return std::nullopt;
    }
    return p;
}