#pragma once
#include <string>
#include <vector>
#include "model/types.hpp"  // Point, Line, (Intersection burada olacak)

struct SvgParams {
    int width  = 1200;
    int height = 900;
    int margin = 40;
};

void saveToSVG(
    const std::string& outputPath,
    const std::vector<Point>& allPoints,
    const std::vector<Line>& segments,
    const std::vector<Intersection>& intersections,
    const SvgParams& params
);
