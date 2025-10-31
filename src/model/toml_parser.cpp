#include "toml_parser.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <algorithm>

// YARDIMCI FONKSİYONLAR
static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (std::string::npos == first) {
        return str;
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

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

// ANA PARSER FONKSİYONU
std::optional<LidarScan> loadScanFromFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Hata: TOML dosyasi acilamadi: " << path << std::endl;
        return std::nullopt;
    }

    LidarScan scan;
    std::string line;
    bool inScanSection = false;
    bool inRangesArray = false;

    std::stringstream rangesStream;

    while (std::getline(file, line)) {
        line = trim(line);

        if (line.empty() || line[0] == '#') {
            continue;
        }

        if (line == "[scan]") {
            inScanSection = true;
            continue;
        } else if (line[0] == '[') {
            inScanSection = false;
            inRangesArray = false;
            continue;
        }

        if (inRangesArray) {
            size_t endPos = line.find(']');
            if (endPos != std::string::npos) {
                inRangesArray = false;
                rangesStream << line.substr(0, endPos);
            } else {
                rangesStream << line << " ";
            }
            continue;
        }

        if (inScanSection) {
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
                inRangesArray = true;
                size_t startPos = line.find('[');
                size_t endPos = line.find(']');

                if (startPos != std::string::npos) {
                    std::string segment = line.substr(startPos + 1);

                    if (endPos != std::string::npos && endPos > startPos) {
                        segment = segment.substr(0, endPos - startPos - 1);
                        inRangesArray = false;
                    }
                    rangesStream << segment << " ";
                }
            }
        }
    }

    file.close();

    std::string numStr;
    double val;
    std::string allRanges = rangesStream.str();
    std::replace(allRanges.begin(), allRanges.end(), ',', ' ');

    std::stringstream finalStream(allRanges);
    while (finalStream >> val) {
        scan.ranges.push_back(val);
    }


    return scan;
}