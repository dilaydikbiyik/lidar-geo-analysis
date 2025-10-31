#pragma once

#include "model/types.hpp"
#include <string>
#include <optional>

std::optional<LidarScan> loadScanFromFile(const std::string& path);