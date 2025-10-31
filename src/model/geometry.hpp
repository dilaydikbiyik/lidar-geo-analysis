#pragma once
#include "model/types.hpp"
#include <vector>
#include <optional>

std::optional<Point> getSegmentIntersection(const Line& segA, const Line& segB);

std::vector<Intersection> findPhysicalIntersections(
    const std::vector<Line>& segments,
    double minAngleDeg
);