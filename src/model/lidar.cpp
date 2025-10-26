#include "lidar.hpp"
#include <cmath> // cos() ve sin() fonksiyonları için

std::vector<Point> filterAndConvertToPoints(const LidarScan& scan) {
    std::vector<Point> points;

    for (size_t i = 0; i < scan.ranges.size(); ++i) {
        double range = scan.ranges[i];

        // === AŞAMA 1: Değer Filtresi (NaN) ===
        // Hocanın belirttiği geçersiz değerleri atla
        if (range == -1.0 || range == 999.0 || range == -999.0) {
            continue;
        }

        // === AŞAMA 2: Mesafe Filtresi (min/max) ===
        // Hocanın belirttiği minimum ve maksimum algılama mesafesi dışını atla
        if (range < scan.range_min || range > scan.range_max) {
            continue;
        }

        // === AŞAMA 3: Açı Filtresi (max açı) ===
        // Bu noktanın açısını hesapla
        double angle = scan.angle_min + (i * scan.angle_increment);

        // Hocanın belirttiği "açının dışında kalan verileri" atla
        if (angle > scan.angle_max) {
            continue;
        }

        // === DÖNÜŞÜM ===
        // Eğer nokta tüm filtreleri geçtiyse, Kartezyen koordinata çevir
        // Robotun konumu (0,0) olarak kabul edilir
        Point p;
        p.x = range * std::cos(angle);
        p.y = range * std::sin(angle);

        points.push_back(p);
    }

    return points;
}