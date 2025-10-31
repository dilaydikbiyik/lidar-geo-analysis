#include "ransac.hpp"
#include <iostream>
#include <cmath>
#include <random>
#include <limits>
#include <chrono>
#include <numeric>

// RANSAC YARDIMCI FONKSİYONLARI
static Line lineFromPoints(const Point& p1, const Point& p2) {
    Line line;
    line.A = p2.y - p1.y;
    line.B = p1.x - p2.x;
    line.C = -line.A * p1.x - line.B * p1.y;
    return line;
}

static double distanceToLine(const Line& line, const Point& p) {
    return std::abs(line.A * p.x + line.B * p.y + line.C) / std::sqrt(line.A * line.A + line.B * line.B);
}

static double distanceSq(const Point& p1, const Point& p2) {
    return (p1.x - p2.x) * (p1.x - p2.x) + (p1.y - p2.y) * (p1.y - p2.y);
}

static Line refineLineWithLeastSquares(const std::vector<Point>& inliers) {
    if (inliers.size() < 2) {
        return Line{};
    }

    double sumX = 0.0;
    double sumY = 0.0;
    for (const auto& p : inliers) {
        sumX += p.x;
        sumY += p.y;
    }
    const double meanX = sumX / inliers.size();
    const double meanY = sumY / inliers.size();

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

    double T = Sxx + Syy;
    double D = Sxx * Syy - Sxy * Sxy;
    double lambda_small = T / 2.0 - std::sqrt(T * T / 4.0 - D);

    double A = Sxy;
    double B = lambda_small - Sxx;

    double mag = std::sqrt(A * A + B * B);
    if (mag < 1e-9) {
        A = lambda_small - Syy;
        B = Sxy;
        mag = std::sqrt(A * A + B * B);

        if (mag < 1e-9) {
            return lineFromPoints(inliers.front(), inliers.back());
        }
    }

    A /= mag;
    B /= mag;

    double C = -A * meanX - B * meanY;

    Line refinedLine;
    refinedLine.A = A;
    refinedLine.B = B;
    refinedLine.C = C;
    return refinedLine;
}

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

static std::pair<Point, Point> shrinkSegment(Point p1, Point p2, double shrinkAmount) {
    double dx = p2.x - p1.x;
    double dy = p2.y - p1.y;
    double mag = std::sqrt(dx * dx + dy * dy);
    if (mag < 2 * shrinkAmount) {

        Point mid = {(p1.x + p2.x) / 2.0, (p1.y + p2.y) / 2.0};
        return {mid, mid};
    }

    double norm_dx = dx / mag;
    double norm_dy = dy / mag;

    Point new_p1 = {p1.x + norm_dx * shrinkAmount, p1.y + norm_dy * shrinkAmount};
    Point new_p2 = {p2.x - norm_dx * shrinkAmount, p2.y - norm_dy * shrinkAmount};

    return {new_p1, new_p2};
}


// ANA RANSAC FONKSİYONU
std::vector<Line> findLinesRANSAC(
    const std::vector<Point>& allPoints,
    int minInliers,
    double distanceThreshold,
    int maxIterations)
{
    std::vector<Line> foundLines;
    std::vector<Point> remainingPoints = allPoints;

    auto seed = std::chrono::high_resolution_clock::now().time_since_epoch().count();
    std::mt19937 rng(seed);

    int iters = 0;
    while (iters < maxIterations && remainingPoints.size() > minInliers) {
        iters++;

        std::uniform_int_distribution<int> dist(0, remainingPoints.size() - 1);
        int idx1 = dist(rng);
        int idx2 = dist(rng);
        if (idx1 == idx2) continue;

        Point p1 = remainingPoints[idx1];
        Point p2 = remainingPoints[idx2];

        Line candidateLine = lineFromPoints(p1, p2);
        std::vector<Point> inliers;

        for (const auto& p : remainingPoints) {
            double dist = distanceToLine(candidateLine, p);
            if (dist < distanceThreshold) {
                inliers.push_back(p);
            }
        }

        if (inliers.size() >= minInliers) {

            Line refinedLine = refineLineWithLeastSquares(inliers);

            auto [farthest_p1, farthest_p2] = findFarthestPoints(inliers);

            double shrinkAmount = distanceThreshold * 5.0; // örn: 0.1m
            auto [final_p1, final_p2] = shrinkSegment(farthest_p1, farthest_p2, shrinkAmount);

            refinedLine.inlierPoints = inliers;
            refinedLine.startPoint = final_p1;
            refinedLine.endPoint = final_p2;

            foundLines.push_back(refinedLine);

            std::vector<Point> nextRemainingPoints;

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