#include "geometry.hpp"
#include <cmath>
#include <algorithm> // std::min, std::max
#include <iostream>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

std::optional<Point> getSegmentIntersection(const Line& segA, const Line& segB) {
    Point p0 = segA.startPoint;
    Point p1 = segA.endPoint;
    Point p2 = segB.startPoint;
    Point p3 = segB.endPoint;

    double s1_x = p1.x - p0.x;
    double s1_y = p1.y - p0.y;
    double s2_x = p3.x - p2.x;
    double s2_y = p3.y - p2.y;

    double det = (-s2_x * s1_y + s1_x * s2_y);
    if (std::abs(det) < 1e-9) {
        return std::nullopt;
    }

    double s, t;
    s = (-s1_y * (p0.x - p2.x) + s1_x * (p0.y - p2.y)) / det;
    t = ( s2_x * (p0.y - p2.y) - s2_y * (p0.x - p2.x)) / det;

    if (s >= 0 && s <= 1 && t >= 0 && t <= 1) {
        Point intersection;
        intersection.x = p0.x + (t * s1_x);
        intersection.y = p0.y + (t * s1_y);
        return intersection;
    }

    return std::nullopt;
}

// İki Doğrunun Vektörleri Arasındaki Açı
static double getAngleBetweenLines(const Line& lineA, const Line& lineB) {
    double vA_x = lineA.B;
    double vA_y = -lineA.A;

    double vB_x = lineB.B;
    double vB_y = -lineB.A;

    double dotProduct = vA_x * vB_x + vA_y * vB_y;
    double magA = std::sqrt(vA_x * vA_x + vA_y * vA_y);
    double magB = std::sqrt(vB_x * vB_x + vB_y * vB_y);

    if (magA == 0 || magB == 0) return 0;

    double cosTheta = dotProduct / (magA * magB);

    cosTheta = std::max(-1.0, std::min(1.0, cosTheta));

    double angleRad = std::acos(cosTheta);

    double angleDeg = (angleRad * 180.0) / M_PI;

    if (angleDeg > 90.0) {
        angleDeg = 180.0 - angleDeg;
    }

    return angleDeg;
}

// Geometri Fonksiyonu
std::vector<Intersection> findPhysicalIntersections(
    const std::vector<Line>& segments,
    double minAngleDeg)
{
    std::vector<Intersection> validIntersections;

    for (size_t i = 0; i < segments.size(); ++i) {
        for (size_t j = i + 1; j < segments.size(); ++j) {
            const Line& segA = segments[i];
            const Line& segB = segments[j];

            std::optional<Point> intersectionPoint = getSegmentIntersection(segA, segB);

            if (intersectionPoint.has_value()) {
                Point p_intersect = intersectionPoint.value();

                double angle = getAngleBetweenLines(segA, segB);

                if (angle >= minAngleDeg) {

                    double dist =
                        std::sqrt(p_intersect.x * p_intersect.x + p_intersect.y * p_intersect.y);

                    validIntersections.push_back({p_intersect, angle, dist});
                }
            }
        }
    }


    return validIntersections;
}