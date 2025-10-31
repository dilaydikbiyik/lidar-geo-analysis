#include "lidar.hpp"
#include <cmath>

std::vector<Point> filterAndConvertToPoints(const LidarScan& scan) {
    std::vector<Point> points;

    for (size_t i = 0; i < scan.ranges.size(); ++i) {
        double range = scan.ranges[i];

        if (range == -1.0 || range == 999.0 || range == -999.0) {
            continue;
        }

        if (range < scan.range_min || range > scan.range_max) {
            continue;
        }

        double angle = scan.angle_min + (i * scan.angle_increment);

        if (angle > scan.angle_max) {
            continue;
        }

        Point p;
        p.x = range * std::cos(angle);
        p.y = range * std::sin(angle);

        points.push_back(p);
    }


    return points;
}