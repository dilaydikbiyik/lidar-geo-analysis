#include "ransac.hpp"
#include <iostream>
#include <cmath>   // std::abs, std::sqrt
#include <random>  // Rastgele sayı üretmek için
#include <limits>  // std::numeric_limits
#include <chrono> // Zamanı okumak için

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


// === ANA RANSAC FONKSİYONU ===

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
        if (idx1 == idx2) continue; // Aynı noktayı seçtiysek atla

        Point p1 = remainingPoints[idx1];
        Point p2 = remainingPoints[idx2];

        // 2. Bu iki noktadan bir "aday doğru" oluştur
        Line candidateLine = lineFromPoints(p1, p2);
        std::vector<Point> inliers;
        inliers.push_back(p1);
        inliers.push_back(p2);

        // 3. Diğer tüm noktaların bu doğruya yakınlığını kontrol et
        for (size_t i = 0; i < remainingPoints.size(); ++i) {
            if (i == idx1 || i == idx2) continue; // Kendilerini tekrar kontrol etme

            double dist = distanceToLine(candidateLine, remainingPoints[i]);

            // 4. Eğer nokta yeterince "yakınsa", onu "destekçi" (inlier) olarak ekle
            if (dist < distanceThreshold) {
                inliers.push_back(remainingPoints[i]);
            }
        }

        // 5. Yeterli destekçi (inlier) bulduysak, bu bir doğrudur!
        if (inliers.size() >= minInliers) {
            std::cout << "RANSAC: " << inliers.size() << " destekci nokta ile bir dogru bulundu." << std::endl;

            // BURADA İYİLEŞTİRME YAPILABİLİR:
            // "Least Squares" (En Küçük Kareler) metodu ile bu 'inliers' noktalarına
            // en iyi uyan "rafine" bir doğru denklemi hesaplanabilir.
            // Şimdilik, sadece aday doğruyu kabul ediyoruz.

            candidateLine.inlierPoints = inliers; // Doğruya ait noktaları sakla
            foundLines.push_back(candidateLine);

            // 6. Bu doğruya ait noktaları, kalan noktalar havuzundan çıkar
            std::vector<Point> nextRemainingPoints;
            for (const auto& p : remainingPoints) {
                if (distanceToLine(candidateLine, p) >= distanceThreshold) {
                    nextRemainingPoints.push_back(p);
                }
            }
            remainingPoints = std::move(nextRemainingPoints);
        }
    }

    std::cout << "RANSAC tamamlandi. Toplam " << foundLines.size() << " adet dogru bulundu." << std::endl;
    return foundLines;
}