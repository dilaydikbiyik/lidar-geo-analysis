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

    // Bu doğruyu hangi noktaların oluşturduğunu saklamak için
    std::vector<Point> inlierPoints;

    // Görev A.1 için: Doğru parçasının başlangıç ve bitiş noktaları
    Point startPoint;
    Point endPoint;
};

// .toml dosyasından okunan ham LIDAR verilerini tutar
struct LidarScan {
    double angle_min = 0.0;
    double angle_max = 0.0;
    double angle_increment = 0.0;
    double range_min = 0.0;
    double range_max = 0.0;
    std::vector<double> ranges; // Tüm mesafe ölçümleri
};

// Geçerli bir kesişimin bilgilerini tutar
struct Intersection {
    Point position; // Kesişim noktası (x, y)
    double angleDeg; // Kesişim açısı (derece)
    double distanceToRobot; // Robota (0,0) olan mesafe
};