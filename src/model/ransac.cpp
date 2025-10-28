#include "ransac.hpp"
#include <iostream>
#include <cmath>
#include <random>
#include <limits>
#include <chrono> // Rastgele 'seed' için
#include <numeric> // En Küçük Kareler (Least Squares) hesabı için

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
 * Verilen 'inliers' nokta kümesine En Küçük Kareler Yöntemi (Least Squares)
 * ile en iyi uyan doğru denklemini hesaplar.
 * Bu, RANSAC'ın ilk bulduğu kaba denklemi rafine ederek hassasiyeti artırır.
 * PCA (Temel Bileşen Analizi) ilkelerine dayalı bir Total Least Squares (TLS) uygular.
 */
static Line refineLineWithLeastSquares(const std::vector<Point>& inliers) {
    if (inliers.size() < 2) {
        return Line{}; // Geçersiz
    }

    // 1. Noktaların geometrik merkezini (centroid) bul
    double sumX = 0.0;
    double sumY = 0.0;
    for (const auto& p : inliers) {
        sumX += p.x;
        sumY += p.y;
    }
    const double meanX = sumX / inliers.size();
    const double meanY = sumY / inliers.size();

    // 2. Kovaryans matrisini (scatter matrix) hesapla
    double Sxx = 0.0;
    double Sxy = 0.0;
    double Syy = 0.0;
    for (const auto& p : inliers) {
        double dx = p.x - meanX;
        double dy = p.y - meanY;
        Sxx += dx * dx;
        Sxy += dx * dy;
        Syy += dy * dy;
    }

    // 3. PCA ile doğrunun yönünü bul
    // Doğrunun normal vektörü (A, B), kovaryans matrisinin
    // en küçük özdeğerine (eigenvalue) karşılık gelen özvektördür (eigenvector).
    double T = Sxx + Syy;
    double D = Sxx * Syy - Sxy * Sxy;
    double lambda_small = T / 2.0 - std::sqrt(T * T / 4.0 - D); // En küçük özdeğer

    // Özvektörü (A, B) bul: (Sxx - lambda)A + Sxy*B = 0
    double A = Sxy;
    double B = lambda_small - Sxx;

    // Normalizasyon
    double mag = std::sqrt(A * A + B * B);
    if (mag < 1e-9) {
        // Noktalar çakışıksa veya bir hata oluşursa,
        // diğer denklemi dene: Sxy*A + (Syy - lambda)B = 0
        A = lambda_small - Syy;
        B = Sxy;
        mag = std::sqrt(A * A + B * B);

        if (mag < 1e-9) {
            // Hala sorun varsa, eski yönteme (iki uç nokta) dön
            return lineFromPoints(inliers.front(), inliers.back());
        }
    }

    A /= mag;
    B /= mag;

    // 4. C katsayısını hesapla: Ax + By + C = 0
    // Doğru, centroid'den (meanX, meanY) geçmelidir.
    double C = -A * meanX - B * meanY;

    Line refinedLine;
    refinedLine.A = A;
    refinedLine.B = B;
    refinedLine.C = C;
    return refinedLine;
}


/**
 * Bir nokta kümesi içindeki birbirinden en uzak iki noktayı bulur.
 * Bu, o doğrunun "doğru parçası" olarak uç noktalarını temsil eder.
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
 * Bir doğru parçasını, sensör gürültüsünü tolere etmek için
 * her iki uçtan da "shrinkAmount" (örn: 0.15m) kadar küçültür (içeri çeker).
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
        if (idx1 == idx2) continue;

        Point p1 = remainingPoints[idx1];
        Point p2 = remainingPoints[idx2];

        // 2. Bu iki noktadan bir "aday doğru" oluştur (ilk hipotez)
        Line candidateLine = lineFromPoints(p1, p2);
        std::vector<Point> inliers;

        // 3. Diğer tüm noktaların bu doğruya yakınlığını kontrol et
        for (const auto& p : remainingPoints) {
            double dist = distanceToLine(candidateLine, p);
            if (dist < distanceThreshold) {
                inliers.push_back(p);
            }
        }

        // 4. Yeterli destekçi (inlier) bulduysak, bu bir doğrudur!
        if (inliers.size() >= minInliers) {

            // 4a. (İyileştirme) Modeli Rafine Et:
            // 'inliers' kümesini (örn: 20 nokta) alıp, bu 20 noktaya
            // En Küçük Kareler Yöntemi ile en iyi uyan "rafine" doğru denklemini hesapla.
            Line refinedLine = refineLineWithLeastSquares(inliers);

            // 4b. Doğru Parçası Tespiti:
            // Bulunan 'inliers' kümesinden fiziksel bir doğru parçası oluştur.

            // Uç noktaları bul (inliers kümesindeki en uzak iki nokta)
            auto [farthest_p1, farthest_p2] = findFarthestPoints(inliers);

            // Doğru parçasını gürültüye karşı küçült (shrink)
            double shrinkAmount = distanceThreshold * 5.0; // örn: 0.1m
            auto [final_p1, final_p2] = shrinkSegment(farthest_p1, farthest_p2, shrinkAmount);

            // 5. Bulunan rafine edilmiş hattı ve doğru parçası uçlarını ata
            refinedLine.inlierPoints = inliers;
            refinedLine.startPoint = final_p1;
            refinedLine.endPoint = final_p2;

            foundLines.push_back(refinedLine); // Rafine edilmiş hattı ekle

            // 6. Bu doğruya ait noktaları, kalan noktalar havuzundan çıkar
            std::vector<Point> nextRemainingPoints;

            // Not: Noktaları, 2 noktalı 'candidateLine'a göre değil,
            // 'refinedLine' (En Küçük Kareler) hattına göre havuzdan çıkar.
            for (const auto& p : remainingPoints) {
                if (distanceToLine(refinedLine, p) >= distanceThreshold) {
                    nextRemainingPoints.push_back(p);
                }
            }
            remainingPoints = std::move(nextRemainingPoints);
        }
    }

    return foundLines;
}