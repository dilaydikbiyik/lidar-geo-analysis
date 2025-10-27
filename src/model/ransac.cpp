#include "ransac.hpp"
#include <iostream>
#include <cmath>
#include <random>
#include <limits>
#include <chrono> // Rastgele 'seed' için

// === RANSAC YARDIMCI FONKSİYONLARI ===

/**
 * İki rastgele noktadan (p1, p2) geçen bir doğrunun
 * denklemini (Ax + By + C = 0) hesaplar.
 */
static Line lineFromPoints(const Point& p1, const Point& p2) {
    Line line;
    line.A = p2.y - p1.y;
    line.B = p1.x - p2.x;
    line.C = -line.A * p1.x - line.B * p1.y;
    return line;
}

/**
 * Bir 'p' noktasının, bir 'line' doğrusuna olan dikey mesafesini hesaplar.
 * Formül: |Ax + By + C| / sqrt(A^2 + B^2)
 */
static double distanceToLine(const Line& line, const Point& p) {
    return std::abs(line.A * p.x + line.B * p.y + line.C) / std::sqrt(line.A * line.A + line.B * line.B);
}

/**
 * İki nokta arasındaki mesafenin karesini hesaplar.
 * (sqrt kullanmamak daha hızlıdır)
 */
static double distanceSq(const Point& p1, const Point& p2) {
    return (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);
}

/**
 * === YENİ FONKSİYON (Görev A.1) ===
 * Bir nokta kümesi içindeki birbirinden en uzak iki noktayı bulur.
 * Bu, o doğrunun "uç noktalarını" temsil eder.
 */
static std::pair<Point, Point> findFarthestPoints(const std::vector<Point>& points) {
    double maxDistSq = -1.0;
    Point p1, p2;

    for (size_t i = 0; i < points.size(); ++i) {
        for (size_t j = i + 1; j < points.size(); ++j) {
            double dist = distanceSq(points[i], points[j]);
            if (dist > maxDistSq) {
                maxDistSq = dist;
                p1 = points[i];
                p2 = points[j];
            }
        }
    }
    return {p1, p2};
}

/**
 * === YENİ FONKSİYON (Görev A.1) ===
 * Bir doğru parçasını her iki uçtan da "shrinkAmount" (örn: 0.15m) kadar
 * küçültür (içeri çeker).
 */
static std::pair<Point, Point> shrinkSegment(Point p1, Point p2, double shrinkAmount) {
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    double mag = std::sqrt(dx * dx + dy * dy);

    if (mag < 2 * shrinkAmount) {
        // Eğer segment, küçültme payından daha kısaysa,
        // segmentin tam ortasında tek bir nokta olarak döndür
        Point mid = {(p1.x + p2.x) / 2.0, (p1.y + p2.y) / 2.0};
        return {mid, mid};
    }

    // Normalizasyon
    double norm_dx = dx / mag;
    double norm_dy = dy / mag;

    // Uç noktaları 'shrinkAmount' kadar içeri çek
    Point new_p1 = {p1.x + norm_dx * shrinkAmount, p1.y + norm_dy * shrinkAmount};
    Point new_p2 = {p2.x - norm_dx * shrinkAmount, p2.y - norm_dy * shrinkAmount};

    return {new_p1, new_p2};
}


// === ANA RANSAC FONKSİYONU (GÜNCELLENDİ) ===

std::vector<Line> findLinesRANSAC(
    const std::vector<Point>& allPoints,
    int minInliers,
    double distanceThreshold,
    int maxIterations)
{
    std::vector<Line> foundLines;
    std::vector<Point> remainingPoints = allPoints;

    // Rastgele sayı üreteci (RNG)
    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937 rng(seed);

    int iters = 0;
    while (iters < maxIterations && remainingPoints.size() > minInliers) {
        iters++;

        // 1. Rastgele iki farklı nokta seç
        std::uniform_int_distribution<int> dist(0, remainingPoints.size() - 1);
        int idx1 = dist(rng);
        int idx2 = dist(rng);
        if (idx1 == idx2) continue;

        Point p1 = remainingPoints[idx1];
        Point p2 = remainingPoints[idx2];

        // 2. Bu iki noktadan bir "aday doğru" oluştur
        Line candidateLine = lineFromPoints(p1, p2);
        std::vector<Point> inliers;
        // Not: Başlangıçta p1 ve p2'yi eklemiyoruz,
        // 3. adımda tüm noktalarla birlikte kontrol edilecekler.

        // 3. Diğer tüm noktaların bu doğruya yakınlığını kontrol et
        for (const auto& p : remainingPoints) {
            double dist = distanceToLine(candidateLine, p);
            if (dist < distanceThreshold) {
                inliers.push_back(p);
            }
        }

        // 4. Yeterli destekçi (inlier) bulduysak, bu bir doğrudur!
        if (inliers.size() >= minInliers) {

            // --- GÖREV A.1 BAŞLANGIÇ ---
            // 'inliers' noktalarına en iyi uyan "rafine" bir doğru denklemi
            // hesapla (En Küçük Kareler Yöntemi - İsteğe bağlı ama önerilir)
            // Şimdilik, sadece aday doğruyu kullanıyoruz.

            // 5. Doğru parçasının uç noktalarını bul
            // 'inliers' kümesindeki en uzak iki noktayı bul
            auto [farthest_p1, farthest_p2] = findFarthestPoints(inliers);

            // 6. Doğru parçasını hocanın tavsiyesine göre küçült (shrink)
            // Küçültme payı olarak 'distanceThreshold'u kullanabiliriz (örn: 0.02 * 5 = 0.1m)
            double shrinkAmount = distanceThreshold * 5.0; // 0.1m (veya sabit 0.15m)
            auto [final_p1, final_p2] = shrinkSegment(farthest_p1, farthest_p2, shrinkAmount);

            // 7. Bulunan değerleri Line nesnesine ata
            candidateLine.inlierPoints = inliers;
            candidateLine.startPoint = final_p1;
            candidateLine.endPoint = final_p2;
            // --- GÖREV A.1 BİTİŞ ---

            foundLines.push_back(candidateLine);

            // 8. Bu doğruya ait noktaları, kalan noktalar havuzundan çıkar
            std::vector<Point> nextRemainingPoints;
            for (const auto& p : remainingPoints) {
                // Not: Sadece 'inliers' listesindekileri değil, o doğruya yakın
                // olan tüm noktaları havuzdan çıkarmalıyız.
                if (distanceToLine(candidateLine, p) >= distanceThreshold) {
                    nextRemainingPoints.push_back(p);
                }
            }
            remainingPoints = std::move(nextRemainingPoints);
        }
    }

    std::cout << "RANSAC (v2) tamamlandi. Toplam " << foundLines.size() << " adet dogru parcasi bulundu." << std::endl;
    return foundLines;
}