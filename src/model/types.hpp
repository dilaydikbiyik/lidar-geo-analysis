#pragma once

#include <vector>
#include <string>

// 2D Kartezyen nokta
struct Point {
    double x = 0.0; // Arkadaşından (varsayılan değer)
    double y = 0.0; // Arkadaşından (varsayılan değer)
}; // İkinizde de var [cite: 4, 13]

// Ax + By + C = 0 şeklinde bir doğru denklemi ve fiziksel segmenti
struct Line {
    // Sonsuz doğru denklemi
    double A = 0.0; // İkinizde de var [cite: 5, 14]
    double B = 0.0; // İkinizde de var [cite: 5, 14]
    double C = 0.0; // İkinizde de var

    // Bu doğruyu oluşturan RANSAC inlier noktaları
    std::vector<Point> inlierPoints; // İkinizde de var

    // Doğru parçasının (küçültülmüş) başlangıç ve bitiş noktaları
    Point startPoint; // İkinizde de var [cite: 6, 15]
    Point endPoint;   // İkinizde de var [cite: 6, 15]
};

// .toml dosyasından okunan ham LIDAR verilerini tutar
struct LidarScan {
    double angle_min = 0.0; // İkinizde de var [cite: 8, 17]
    double angle_max = 0.0; // İkinizde de var [cite: 8, 17]
    double angle_increment = 0.0; // İkinizde de var
    double range_min = 0.0; // İkinizde de var
    double range_max = 0.0; // İkinizde de var
    std::vector<double> ranges; // İkinizde de var [cite: 9, 18]
}; // İkinizde de var [cite: 7, 16]

// Geçerli bir kesişimin bilgilerini tutar
struct Intersection {
    Point position;               // Kesişim noktası (x, y) - İkinizde de var [cite: 10, 19]
    double angleDeg = 0.0;        // Doğrular arasındaki açı (derece) - Arkadaşından (varsayılan değer) [cite: 11]
    double distanceToRobot = 0.0; // Robota (0,0) olan mesafe (metre) - Arkadaşından (varsayılan değer) [cite: 11]
}; // İkinizde de var