#pragma once

#include "model/types.hpp" // LidarScan struct'ını kullanmak için
#include <string>
#include <optional> // Hata durumları için

// Verilen yoldaki .toml dosyasını okur ve bir LidarScan nesnesi döndürür.
// Hata olursa std::nullopt döndürür.
std::optional<LidarScan> loadScanFromFile(const std::string& path);