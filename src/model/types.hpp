#pragma once

#include <vector>
#include <string>

// 2D Kartezyen nokta
struct Point {
    double x = 0.0;
    double y = 0.0;
};

// Ax + By + C = 0 şeklinde bir doğru denklemi
struct Line {
    double A = 0.0;
    double B = 0.0;
    double C = 0.0;

    // Bu doğruyu oluşturan RANSAC inlier noktaları
    std::vector<Point> inlierPoints;

    // Doğru parçasının (küçültülmüş) başlangıç ve bitiş noktaları
    Point startPoint;
    Point endPoint;
};

struct LidarScan {
    double angle_min = 0.0;
    double angle_max = 0.0;
    double angle_increment = 0.0;
    double range_min = 0.0;
    double range_max = 0.0;
    std::vector<double> ranges;
};

struct Intersection {
    Point position;
    double angleDeg = 0.0;
    double distanceToRobot = 0.0;
};