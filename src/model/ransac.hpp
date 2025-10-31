#pragma once
#include "model/types.hpp"
#include <vector>

std::vector<Line> findLinesRANSAC(
    const std::vector<Point>& allPoints,
    int minInliers,
    double distanceThreshold,
    int maxIterations
);