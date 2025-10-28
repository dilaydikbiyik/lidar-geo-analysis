#include "utils/cli.hpp"
#include <iostream>
#include <sstream>
#include <vector>
#include <cstring>   // std::strcmp, std::strncmp, std::strlen
#include <cctype>    // (Kullanılmıyor, kaldırılabilir)
#include <cstdlib>   // std::strtod, std::strtol
#include <algorithm> // (Kullanılmıyor, kaldırılabilir)
#include <string>    // std::string
#include <limits> // std::numeric_limits eklenmeli (eğer yoksa)

// === Arkadaşının Yardımcı Fonksiyonları ===
static bool starts_with(const std::string& s, const char* p) {
    return s.rfind(p, 0) == 0;
}

static bool parse_int(const std::string& s, int& out) {
    char* end = nullptr;
    long v = std::strtol(s.c_str(), &end, 10);
    // Uç işaretçi kontrolü VE Taşma (Overflow) kontrolü
    if (!end || *end != '\0' || v < std::numeric_limits<int>::min() || v > std::numeric_limits<int>::max()) {
        return false; // Geçersiz sayı veya aralık dışı
    }
    // DOĞRU ATAMA: long'dan int'e güvenli dönüşüm
    out = static_cast<int>(v);
    return true;
}

static bool parse_double(const std::string& s, double& out) {
    char* end = nullptr;
    double v = std::strtod(s.c_str(), &end);
    if (!end || *end != '\0') return false;
    out = v;
    return true;
}

// --svg-size "1200x900" parse (Arkadaşından)
static bool parse_size(const std::string& s, int& w, int& h) {
    auto x = s.find('x');
    if (x == std::string::npos) return false;
    std::string sw = s.substr(0, x);
    std::string sh = s.substr(x + 1);
    return parse_int(sw, w) && parse_int(sh, h);
}
// ==========================================

// Birleştirilmiş Yardım Mesajı
void print_cli_help(const char* exe) {
    std::cout
      << "Usage:\n  " << exe << " [--input <pathOrUrl>] [<pathOrUrl>] [options]\n\n" // Positional eklendi
      << "Required:\n"
      << "  -i, --input <pathOrUrl>      TOML dosya yolu veya URL\n"
      << "                               (Eger flag kullanilmazsa ilk arguman olarak da verilebilir)\n\n" // Açıklama eklendi
      << "RANSAC / Geometri:\n"
      << "      --epsilon <m>            RANSAC mesafe esigi (default: " << CliParams{}.epsilon << ")\n" // Varsayılan değerler gösterildi
      << "      --min-inliers <n>        RANSAC min inlier (default: " << CliParams{}.minInliers << ")\n"
      << "      --max-iters <n>          RANSAC iter sayisi (default: " << CliParams{}.maxIters << ")\n"
      << "      --angle-thresh <deg>     Dogru cifti aci esigi (default: " << CliParams{}.angleThreshDeg << ")\n\n"
      << "SVG Cikti:\n"
      << "      --out-svg <path>         SVG cikti yolu (default: " << CliParams{}.outSvg << ")\n" // Arkadaşının flag adı kullanıldı
      << "      --svg-size <WxH>         Or: 1200x900 (default: " << CliParams{}.svgWidth << "x" << CliParams{}.svgHeight << ")\n" // Arkadaşından
      << "      --svg-margin <px>        Kenar bosluk px (default: " << CliParams{}.svgMargin << ")\n\n" // Arkadaşından
      << "Other:\n"
      << "  -h, --help                   Bu yardimi goster\n"; // İkinizde de var
}

// Birleştirilmiş Argüman Okuyucu
std::optional<CliParams> parse_cli(int argc, char* argv[]) {
    if (argc <= 1) { print_cli_help(argv[0]); return std::nullopt; } // İkinizde de var

    CliParams p; // Varsayılan değerler cli.hpp'den gelir

    // === SENİN EKLEDİĞİN POSITIONAL ARGUMENT MANTIĞI ===
    // Flag olmayan ilk argümanı inputPath olarak ata
    bool first_positional_used = false;
    if (argc >= 2 && argv[1][0] != '-') {
        p.inputPath = argv[1];
        first_positional_used = true;
    }
    // ================================================

    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];

        // === SENİN EKLEDİĞİN POSITIONAL ARGUMENT KONTROLÜ ===
        // Eğer ilk argümanı zaten input olarak aldıysak ve bu da ilk argümansa, atla
        if (i == 1 && first_positional_used) {
            continue;
        }
        // ====================================================

        if (a == "-h" || a == "--help") { // İkinizde de var
            print_cli_help(argv[0]);
            return std::nullopt;
        }
        else if (a == "-i" || a == "--input") { // İkinizde de var
            if (i + 1 >= argc) { std::cerr << "[!] " << a << " deger bekliyor\n"; return std::nullopt; }
            p.inputPath = argv[++i];
        }
        else if (a == "--epsilon") { // İkinizde de var
            if (i + 1 >= argc || !parse_double(argv[i+1], p.epsilon)) {
                std::cerr << "[!] --epsilon <double>\n"; return std::nullopt;
            }
            ++i;
        }
        else if (a == "--min-inliers") { // İkinizde de var
            if (i + 1 >= argc || !parse_int(argv[i+1], p.minInliers)) {
                std::cerr << "[!] --min-inliers <int>\n"; return std::nullopt;
            }
            ++i;
        }
        else if (a == "--max-iters") { // İkinizde de var
            if (i + 1 >= argc || !parse_int(argv[i+1], p.maxIters)) {
                std::cerr << "[!] --max-iters <int>\n"; return std::nullopt;
            }
            ++i;
        }
        else if (a == "--angle-thresh") { // İkinizde de var
            if (i + 1 >= argc || !parse_double(argv[i+1], p.angleThreshDeg)) {
                std::cerr << "[!] --angle-thresh <deg>\n"; return std::nullopt;
            }
            ++i;
        }
        // --- Arkadaşının Eklediği Parametreler ---
        else if (a == "--out-svg") { // Arkadaşının flag adı
            if (i + 1 >= argc) { std::cerr << "[!] --out-svg <path>\n"; return std::nullopt; }
            p.outSvg = argv[++i];
        }
        else if (a == "--svg-size") { // Arkadaşından
            if (i + 1 >= argc || !parse_size(argv[i+1], p.svgWidth, p.svgHeight)) {
                std::cerr << "[!] --svg-size <W>x<H> (ornegin 1200x900)\n"; return std::nullopt;
            }
            ++i;
        }
        else if (a == "--svg-margin") { // Arkadaşından
            if (i + 1 >= argc || !parse_int(argv[i+1], p.svgMargin)) {
                std::cerr << "[!] --svg-margin <px>\n"; return std::nullopt;
            }
            ++i;
        }
        // -----------------------------------------
        else {
            // Bilinmeyen argüman (Arkadaşının hata verme mantığı korundu)
            std::cerr << "[!] Bilinmeyen veya hatali arguman: " << a << "\n\n"; // Daha net mesaj
            print_cli_help(argv[0]);
            return std::nullopt;
        }
    }

    // Input path kontrolü (İkinizde de var, arkadaşının mesajı daha net)
    if (p.inputPath.empty()) {
        std::cerr << "[!] Girdi dosyasi (--input) belirtilmedi.\n\n"; // Daha net mesaj
        print_cli_help(argv[0]);
        return std::nullopt;
    }
    return p; // Başarılı
}