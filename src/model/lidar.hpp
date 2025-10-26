#pragma once
#include "model/types.hpp"

// Ham LidarScan verisini alır, filtreler ve
// Kartezyen koordinatlara (Point) dönüştürür.
std::vector<Point> filterAndConvertToPoints(const LidarScan& scan);