#pragma once
#include "model/types.hpp" // Point ve Line struct'larını kullanmak için
#include <vector>

/**
 * Verilen nokta bulutu içinden RANSAC algoritmasını kullanarak doğruları bulur.
 *
 * @param allPoints Filtrelenmiş tüm geçerli noktaların vektörü.
 * @param minInliers Bir doğrunun kabul edilmesi için gereken minimum nokta sayısı (İster 2'ye göre en az 8 olmalı).
 * @param distanceThreshold Bir noktanın bir doğruya "yakın" sayılması için gereken maksimum mesafe (metre).
 * @param maxIterations RANSAC algoritmasının deneme sayısı.
 * @return Bulunan tüm geçerli doğruların (Line) bir vektörü.
 */
std::vector<Line> findLinesRANSAC(
    const std::vector<Point>& allPoints,
    int minInliers,
    double distanceThreshold,
    int maxIterations
);