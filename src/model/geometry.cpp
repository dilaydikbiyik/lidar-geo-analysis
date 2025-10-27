#include "geometry.hpp"
#include <cmath>
#include <algorithm> // std::min, std::max
#include <iostream>

// Pi sayısını tanımla (eğer <cmath> sağlamazsa)
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// === Yardımcı Fonksiyon: İki Doğru Parçası Kesişimi ===
// Bu fonksiyon, iki doğru parçasının fiziksel olarak kesişip
// kesişmediğini kontrol eder.
std::optional<Point> getSegmentIntersection(const Line& segA, const Line& segB) {
    Point p0 = segA.startPoint;
    Point p1 = segA.endPoint;
    Point p2 = segB.startPoint;
    Point p3 = segB.endPoint;

    // Matematiksel formüller (çizgisel cebir)
    double s1_x = p1.x - p0.x;
    double s1_y = p1.y - p0.y;
    double s2_x = p3.x - p2.x;
    double s2_y = p3.y - p2.y;

    double det = (-s2_x * s1_y + s1_x * s2_y);
    if (std::abs(det) < 1e-9) { // Neredeyse paralelse (0'a çok yakınsa)
        return std::nullopt; // Paralel doğrular, kesişmez
    }

    double s, t;
    s = (-s1_y * (p0.x - p2.x) + s1_x * (p0.y - p2.y)) / det;
    t = ( s2_x * (p0.y - p2.y) - s2_y * (p0.x - p2.x)) / det;

    // Kesişim noktasının her iki doğru PARÇASI üzerinde olup olmadığını kontrol et
    // s ve t değerleri [0, 1] aralığında olmalıdır.
    if (s >= 0 && s <= 1 && t >= 0 && t <= 1) {
        // Kesişim var
        Point intersection;
        intersection.x = p0.x + (t * s1_x);
        intersection.y = p0.y + (t * s1_y);
        return intersection;
    }

    return std::nullopt; // Kesişim, doğru parçalarının "dışında" kaldı
}

// === Yardımcı Fonksiyon: İki Doğrunun Vektörleri Arasındaki Açı ===
static double getAngleBetweenLines(const Line& lineA, const Line& lineB) {
    // Doğrunun denklemi Ax + By + C = 0 ise,
    // (B, -A) bir yön vektörüdür.
    double vA_x = lineA.B;
    double vA_y = -lineA.A;

    double vB_x = lineB.B;
    double vB_y = -lineB.A;

    // Vektörel çarpım (Dot product): vA . vB = |vA| * |vB| * cos(theta)
    double dotProduct = vA_x * vB_x + vA_y * vB_y;
    double magA = std::sqrt(vA_x * vA_x + vA_y * vA_y);
    double magB = std::sqrt(vB_x * vB_x + vB_y * vB_y);

    if (magA == 0 || magB == 0) return 0; // Geçersiz doğru

    double cosTheta = dotProduct / (magA * magB);

    // cosTheta'nın -1 ile 1 arasında olduğundan emin ol (hassasiyet hataları için)
    cosTheta = std::max(-1.0, std::min(1.0, cosTheta));

    // Açı (Radyan)
    double angleRad = std::acos(cosTheta);

    // Dereceye çevir
    double angleDeg = (angleRad * 180.0) / M_PI;

    // Biz her zaman 0-90 derece arasındaki dar açıyı istiyoruz
    if (angleDeg > 90.0) {
        angleDeg = 180.0 - angleDeg;
    }

    return angleDeg;
}


// === ANA GEOMETRİ FONKSİYONU ===

std::vector<Intersection> findPhysicalIntersections(
    const std::vector<Line>& segments,
    double minAngleDeg)
{
    std::vector<Intersection> validIntersections;

    // 1. Tüm segment çiftlerini kontrol et (iç içe döngü)
    for (size_t i = 0; i < segments.size(); ++i) {
        for (size_t j = i + 1; j < segments.size(); ++j) {
            const Line& segA = segments[i];
            const Line& segB = segments[j];

            // 2. İki doğru parçasının fiziksel olarak kesişip kesişmediğini kontrol et
            std::optional<Point> intersectionPoint = getSegmentIntersection(segA, segB);

            // 3. Eğer fiziksel bir kesişim varsa:
            if (intersectionPoint.has_value()) {
                Point p_intersect = intersectionPoint.value();

                // 4. Açı Hesapla (İster 4)
                double angle = getAngleBetweenLines(segA, segB);

                // 5. Açı 60 dereceden büyük veya eşitse:
                if (angle >= minAngleDeg) {

                    // 6. Mesafe Hesapla (İster 5)
                    // Robota (0,0) olan Öklid mesafesi
                    double dist = std::sqrt(p_intersect.x * p_intersect.x + p_intersect.y * p_intersect.y);

                    // 7. Geçerli kesişimi listeye ekle
                    validIntersections.push_back({p_intersect, angle, dist});
                }
            }
        }
    }

    // Bu log satırını controller'a taşıdık, o yüzden burada artık olmasına gerek yok.
    // std::cout << "Geometri Analizi: Toplam " << ...

    return validIntersections;
}