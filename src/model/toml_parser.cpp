#include "toml_parser.hpp"
#include <iostream>
#include <fstream>  // Dosya okumak için (ifstream)
#include <sstream>  // Satırları ve string'leri işlemek için
#include <string>
#include <algorithm> // Boşluk silmek için

// === YARDIMCI FONKSİYONLAR ===

// Bir string'in başındaki ve sonundaki boşlukları siler
static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

// "anahtar = deger" formatındaki bir satırdan 'deger'i double olarak alır
static std::optional<double> parseDoubleValue(std::string line) {
    size_t equalsPos = line.find('=');
    if (equalsPos == std::string::npos) return std::nullopt;

    std::string valueStr = trim(line.substr(equalsPos + 1));
    try {
        return std::stod(valueStr);
    } catch (...) {
        return std::nullopt;
    }
}

// === ANA PARSER FONKSİYONU ===

std::optional<LidarScan> loadScanFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Hata: TOML dosyasi acilamadi: " << path << std::endl;
        return std::nullopt;
    }

    LidarScan scan;
    std::string line;
    bool inScanSection = false; // [scan] bölümünde olup olmadığımızı tutan bayrak
    bool inRangesArray = false; // ranges = [...] dizisi içinde olup olmadığımızı tutan bayrak

    std::stringstream rangesStream; // ranges dizisindeki sayıları birleştirmek için

    while (std::getline(file, line)) {
        line = trim(line); // Her satırı boşluklardan arındır

        if (line.empty() || line[0] == '#') { // Boş satırları veya yorumları atla
            continue;
        }

        // --- Durum Değişimi ---
        if (line == "[scan]") {
            inScanSection = true;
            continue; // Bu satırda başka iş yok
        } else if (line[0] == '[') {
            // Başka bir bölüme girdik (örn: [header]), scan bölümü bitti
            inScanSection = false;
            inRangesArray = false; // Ranges dizisi de bitmiş olmalı
            continue;
        }

        if (inRangesArray) {
            // Eğer ranges dizisini okuyorsak...
            size_t endPos = line.find(']');
            if (endPos != std::string::npos) {
                // Dizinin sonuna ('_') geldik
                inRangesArray = false;
                rangesStream << line.substr(0, endPos); // ']' karakterine kadar olan kısmı al
            } else {
                // Dizinin ortasındayız, satırın tamamını ekle
                rangesStream << line << " ";
            }
            continue; // Diziyi okurken başka bir şey arama
        }

        // --- Veri Okuma ---
        if (inScanSection) {
            // Eğer [scan] bölümünün içindeysek...
            if (line.rfind("angle_min", 0) == 0) {
                scan.angle_min = parseDoubleValue(line).value_or(0.0);
            } else if (line.rfind("angle_max", 0) == 0) {
                scan.angle_max = parseDoubleValue(line).value_or(0.0);
            } else if (line.rfind("angle_increment", 0) == 0) {
                scan.angle_increment = parseDoubleValue(line).value_or(0.0);
            } else if (line.rfind("range_min", 0) == 0) {
                scan.range_min = parseDoubleValue(line).value_or(0.0);
            } else if (line.rfind("range_max", 0) == 0) {
                scan.range_max = parseDoubleValue(line).value_or(0.0);
            } else if (line.rfind("ranges", 0) == 0) {
                // ranges dizisinin başlangıcını bulduk
                inRangesArray = true;
                size_t startPos = line.find('[');
                size_t endPos = line.find(']');

                if (startPos != std::string::npos) {
                    std::string segment = line.substr(startPos + 1);
                    // Aynı satırda bitip bitmediğini kontrol et
                    if (endPos != std::string::npos && endPos > startPos) {
                        segment = segment.substr(0, endPos - startPos - 1);
                        inRangesArray = false; // Aynı satırda bitti
                    }
                    rangesStream << segment << " ";
                }
            }
        }
    }

    file.close();

    // --- Ranges Dizisini Ayrıştırma ---
    std::string numStr;
    double val;
    std::string allRanges = rangesStream.str();
    std::replace(allRanges.begin(), allRanges.end(), ',', ' '); // Virgülleri boşlukla değiştir

    std::stringstream finalStream(allRanges);
    while (finalStream >> val) {
        scan.ranges.push_back(val);
    }



    return scan;
}