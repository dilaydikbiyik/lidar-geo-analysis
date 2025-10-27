#pragma once
#include "model/types.hpp"
#include <vector>
#include <optional> // Bir kesişim oladabilir, olmayabilir

/**
 * İki doğru parçasının (Line struct'ları içindeki startPoint/endPoint)
 * fiziksel olarak kesişip kesişmediğini kontrol eder ve kesişim noktasını döndürür.
 * * @param segA Birinci doğru parçası
 * @param segB İkinci doğru parçası
 * @return Kesişim varsa Point, yoksa std::nullopt
 */
std::optional<Point> getSegmentIntersection(const Line& segA, const Line& segB);


/**
 * Verilen tüm doğru parçalarını analiz eder.
 * Fiziksel olarak kesişen ve minimum açı şartını (örn: 60 derece) sağlayan
 * tüm geçerli kesişimleri (Intersection) bulur.
 * * @param segments RANSAC'tan gelen doğru parçaları
 * @param minAngleDeg Kesişimin geçerli sayılması için gereken minimum açı
 * @return Geçerli tüm kesişimlerin bir vektörü
 */
std::vector<Intersection> findPhysicalIntersections(
    const std::vector<Line>& segments,
    double minAngleDeg
);