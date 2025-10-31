#include "view/svg_writer.hpp"
#include <iostream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cmath>
#include <sstream>

static void expandBounds(double& minx, double& miny, double& maxx, double& maxy, const Point& p)
{
    minx = std::min(minx, p.x);
    miny = std::min(miny, p.y);
    maxx = std::max(maxx, p.x);
    maxy = std::max(maxy, p.y);
}

void saveToSVG(const std::string& out,
               const std::vector<Point>& pts,
               const std::vector<Line>& segs,
               const std::vector<Intersection>& xs,
               const SvgParams& sp)
{
    std::ofstream f(out);
    if (!f.is_open())
    {
        std::cerr << "[!] SVG acilamadi: " << out << "\n";
        return;
    }

    // Eksen Oranı
    double minx = -3.0;
    double maxx = 3.0;
    double miny = -3.0;
    double maxy = 3.0;

    // Ölçekleme hesapla
    double W = sp.width - 2 * sp.margin;
    double H = sp.height - 2 * sp.margin;
    double sx = W / std::max(1e-9, (maxx - minx));
    double sy = H / std::max(1e-9, (maxy - miny));
    double s = std::min(sx, sy);

    auto Sx = [&](double x) { return sp.margin + (x - minx) * s; };
    auto Sy = [&](double y) { return sp.height - (sp.margin + (y - miny) * s); };

    // SVG başlangıcı
    f << "<?xml version='1.0' encoding='UTF-8'?>\n";
    f << "<svg xmlns='http://www.w3.org/2000/svg' width='" << sp.width
        << "' height='" << sp.height << "'>\n";

    // Tanımlamalar (gradyanlar, gölgeler, desenler)
    f << "<defs>\n";

    // Arkaplan gradyanı
    f << " <linearGradient id='bgGrad' x1='0%' y1='0%' x2='0%' y2='100%'>\n";
    f << " <stop offset='0%' style='stop-color:#f8f9fa;stop-opacity:1' />\n";
    f << " <stop offset='100%' style='stop-color:#e9ecef;stop-opacity:1' />\n";
    f << " </linearGradient>\n";

    // Robot gradyanı
    f << " <radialGradient id='robotGrad'>\n";
    f << " <stop offset='0%' style='stop-color:#ff6b6b;stop-opacity:1' />\n";
    f << " <stop offset='70%' style='stop-color:#ee5a52;stop-opacity:1' />\n";
    f << " <stop offset='100%' style='stop-color:#c92a2a;stop-opacity:0.8' />\n";
    f << " </radialGradient>\n";

    // Kesişim noktası gradyanı
    f << " <radialGradient id='intersectGrad'>\n";
    f << " <stop offset='0%' style='stop-color:#ffd43b;stop-opacity:1' />\n";
    f << " <stop offset='100%' style='stop-color:#ff922b;stop-opacity:1' />\n";
    f << " </radialGradient>\n";

    // Gölge filtresi
    f << " <filter id='shadow' x='-50%' y='-50%' width='200%' height='200%'>\n";
    f << " <feGaussianBlur in='SourceAlpha' stdDeviation='2'/>\n";
    f << " <feOffset dx='2' dy='2' result='offsetblur'/>\n";
    f << " <feComponentTransfer><feFuncA type='linear' slope='0.3'/></feComponentTransfer>\n";
    f << " <feMerge><feMergeNode/><feMergeNode in='SourceGraphic'/></feMerge>\n";
    f << " </filter>\n";

    // Parlama efekti
    f << " <filter id='glow' x='-50%' y='-50%' width='200%' height='200%'>\n";
    f << " <feGaussianBlur stdDeviation='3' result='coloredBlur'/>\n";
    f << " <feMerge><feMergeNode in='coloredBlur'/><feMergeNode in='SourceGraphic'/></feMerge>\n";
    f << " </filter>\n";

    // Grid deseni
    f << " <pattern id='grid' width='40' height='40' patternUnits='userSpaceOnUse'>\n";
    f << " <path d='M 40 0 L 0 0 0 40' fill='none' stroke='#dee2e6' stroke-width='0.5'/>\n";
    f << " </pattern>\n";

    f << "</defs>\n\n";

    // Arkaplan
    f << "<rect width='100%' height='100%' fill='url(#bgGrad)'/>\n";
    f << "<rect width='100%' height='100%' fill='url(#grid)'/>\n\n";

    // Eksenler(metre gostergeleri)
    f << "<g id='axes' style='stroke:#6c757d; stroke-width:1; opacity:0.9;'>\n";

    // Eksen çizgileri (X ve Y) - Sadece görünür alan içinde
    f << " <line x1='" << Sx(minx) << "' y1='" << Sy(0)
        << "' x2='" << Sx(maxx) << "' y2='" << Sy(0) << "'/> \n";
    f << " <line x1='" << Sx(0) << "' y1='" << Sy(miny)
        << "' x2='" << Sx(0) << "' y2='" << Sy(maxy) << "'/> \n";

    f << " <g style='font-family:Arial, sans-serif; font-size:10px; fill:#495057; stroke:none;'>\n";

    // X ekseni için metre işaretleri (tickler)
    for (int m = std::floor(minx); m <= std::ceil(maxx); ++m)
    {
        if (m == 0) continue; // Orijini atla
        double tx = Sx(m);
        double ty = Sy(0);
        f << " <line x1='" << tx << "' y1='" << (ty - 3)
            << "' x2='" << tx << "' y2='" << (ty + 3) << "' style='stroke:#6c757d; stroke-width:1;'/>\n";
        f << " <text x='" << tx << "' y='" << (ty + 15)
            << "' text-anchor='middle'>" << m << "</text>\n";
    }

    // Y ekseni için metre işaretleri (tickler)
    for (int m = std::floor(miny); m <= std::ceil(maxy); ++m)
    {
        if (m == 0) continue; // Orijini atla
        double tx = Sx(0);
        double ty = Sy(m);
        f << " <line x1='" << (tx - 4) << "' y1='" << ty
            << "' x2='" << (tx + 4) << "' y2='" << ty << "' style='stroke:#6c757d;'/>\n";
        f << " <text x='" << (tx - 10) << "' y='" << (ty + 4)
            << "' text-anchor='end'>" << m << "</text>\n";
    }

    // Orijin (0,0) etiketi
    f << " <text x='" << (Sx(0) - 10) << "' y='" << (Sy(0) + 15)
        << "' text-anchor='end' font-size='11'>0</text>\n";
    f << " </g>\n";
    f << "</g>\n\n";

    // Başlık paneli
    f << "<rect x='10' y='10' width='" << (sp.width - 20)
        << "' height='50' rx='8' fill='white' opacity='0.9' filter='url(#shadow)'/>\n";
    f << "<text x='" << (sp.width / 2) << "' y='35' text-anchor='middle' "
        << "font-family='Arial, sans-serif' font-size='20' font-weight='bold' fill='#212529'>"
        << "LIDAR Geometrik Analiz</text>\n";
    f << "<text x='" << (sp.width / 2) << "' y='52' text-anchor='middle' "
        << "font-family='Arial, sans-serif' font-size='11' fill='#6c757d'>"
        << "Nokta: " << pts.size() << " | Doğru: " << segs.size()
        << " | Kesişim: " << xs.size() << "</text>\n\n";

    // LIDAR noktaları - küçük ve hafif
    f << "<g id='lidar-points'>\n";
    for (auto& p : pts)
    {
        double px = Sx(p.x);
        double py = Sy(p.y);
        f << " <circle cx='" << px << "' cy='" << py
            << "' r='1.75' fill='#adb5bd' opacity='0.6'/>\n";
    }
    f << "</g>\n\n";

    // Tespit edilen doğrular - parlak yeşil
    f << "<g id='detected-lines'>\n";
    for (size_t i = 0; i < segs.size(); i++)
    {
        auto& s = segs[i];
        f << " <line x1='" << Sx(s.startPoint.x) << "' y1='" << Sy(s.startPoint.y)
            << "' x2='" << Sx(s.endPoint.x) << "' y2='" << Sy(s.endPoint.y)
            << "' stroke='#51cf66' stroke-width='3' stroke-linecap='round' "
            << "opacity='0.8' filter='url(#glow)'/>\n";
    }
    f << "</g>\n\n";

    // Kesişim noktaları ve mesafe çizgileri
    if (!xs.empty())
    {
        f << "<g id='intersections'>\n";

        // Kutu çakışmalarını önlemek için pozisyonları sakla
        std::vector<std::tuple<double, double, double, double>> used_boxes; // x, y, w, h

        for (size_t i = 0; i < xs.size(); i++)
        {
            auto& inter = xs[i];
            double ix = Sx(inter.position.x);
            double iy = Sy(inter.position.y);
            double rx = Sx(0);
            double ry = Sy(0);

            // Mesafe çizgisi - kesikli kırmızı
            f << " <line x1='" << rx << "' y1='" << ry
                << "' x2='" << ix << "' y2='" << iy
                << "' stroke='#ff6b6b' stroke-width='2' stroke-dasharray='8,4' "
                << "opacity='0.7'/>\n";

            // Kesişim işareti - X şekli
            double mark_size = 8;
            f << " <g filter='url(#shadow)'>\n";
            f << " <line x1='" << (ix - mark_size) << "' y1='" << (iy - mark_size)
                << "' x2='" << (ix + mark_size) << "' y2='" << (iy + mark_size)
                << "' stroke='url(#intersectGrad)' stroke-width='3' stroke-linecap='round'/>\n";
            f << " <line x1='" << (ix - mark_size) << "' y1='" << (iy + mark_size)
                << "' x2='" << (ix + mark_size) << "' y2='" << (iy - mark_size)
                << "' stroke='url(#intersectGrad)' stroke-width='3' stroke-linecap='round'/>\n";
            f << " </g>\n";

            // Bilgi kutusu - akıllı yerleştirme
            double box_w = 45;
            double box_h = 32;
            double box_padding = 15;

            // Olası pozisyonlar: sağ, sol, üst, alt - daha fazla mesafe
            std::vector<std::pair<double, double>> candidates = {
                {ix + 18, iy - box_h / 2}, // Sağ
                {ix - box_w - 18, iy - box_h / 2}, // Sol
                {ix - box_w / 2, iy - box_h - 18}, // Üst
                {ix - box_w / 2, iy + 18} // Alt
            };

            double box_x = candidates[0].first;
            double box_y = candidates[0].second;
            bool found_spot = false;

            // Her pozisyonu test et
            for (auto& cand : candidates)
            {
                double test_x = cand.first;
                double test_y = cand.second;

                // Ekran sınırlarını kontrol et
                if (test_x < 10 || test_x + box_w > sp.width - 10 ||
                    test_y < 70 || test_y + box_h > sp.height - 120)
                {
                    continue;
                }

                // Diğer kutularla çakışma kontrolü
                bool overlaps = false;
                for (auto& used : used_boxes)
                {
                    double ux = std::get<0>(used);
                    double uy = std::get<1>(used);
                    double uw = std::get<2>(used);
                    double uh = std::get<3>(used);

                    if (!(test_x + box_w + box_padding < ux ||
                        test_x > ux + uw + box_padding ||
                        test_y + box_h + box_padding < uy ||
                        test_y > uy + uh + box_padding))
                    {
                        overlaps = true;
                        break;
                    }
                }

                if (!overlaps)
                {
                    box_x = test_x;
                    box_y = test_y;
                    found_spot = true;
                    break;
                }
            }

            // Eğer hiçbir yer bulunamazsa sadece görünür tut
            if (found_spot)
            {
                used_boxes.push_back({box_x, box_y, box_w, box_h});
            }

            f << " <rect x='" << box_x << "' y='" << box_y
                << "' width='" << box_w << "' height='" << box_h
                << "' rx='5' fill='white' stroke='#ff922b' stroke-width='1.5' "
                << "opacity='0.95' filter='url(#shadow)'/>\n";

            f << " <text x='" << (box_x + box_w / 2) << "' y='" << (box_y + 13)
                << "' text-anchor='middle' font-family='Arial, sans-serif' "
                << "font-size='11' font-weight='bold' fill='#ff6b6b'>"
                << std::fixed << std::setprecision(2) << inter.distanceToRobot << " m</text>\n";

            f << " <text x='" << (box_x + box_w / 2) << "' y='" << (box_y + 25)
                << "' text-anchor='middle' font-family='Arial, sans-serif' "
                << "font-size='9' fill='#495057'>"
                << (int)std::round(inter.angleDeg) << "°</text>\n";
        }
        f << "</g>\n\n";
    }

    // Robot pozisyonu - Base64 encoded SVG robot image ile animasyon
    double robot_x = Sx(0);
    double robot_y = Sy(0);
    double robot_size = 30; // Minik boyut

    f << "<g id='robot' transform='translate(" << (robot_x - robot_size / 2) << "," << (robot_y - robot_size / 2) <<
        ")'>\n";

    // Yukarı aşağı hareket animasyonu
    f << " <animateTransform attributeName='transform' type='translate' "
        << "values='" << (robot_x - robot_size / 2) << "," << (robot_y - robot_size / 2) << "; "
        << (robot_x - robot_size / 2) << "," << (robot_y - robot_size / 2 - 4) << "; "
        << (robot_x - robot_size / 2) << "," << (robot_y - robot_size / 2) << "' "
        << "dur='2s' repeatCount='indefinite'/>\n";

    // Sevimli robot SVG (basitleştirilmiş ve küçük)
    f << " <g transform='scale(0.75)'>\n";

    // Robot kafası - mavi
    f << " <rect x='8' y='2' width='24' height='20' rx='6' fill='#4dabf7' stroke='#1971c2' stroke-width='2'/>\n";
    f << " <rect x='12' y='6' width='16' height='12' rx='3' fill='#74c0fc'/>\n";
    f << " <circle cx='18' cy='10' r='2' fill='#1971c2'/>\n";
    f << " <circle cx='22' cy='10' r='2' fill='#1971c2'/>\n";
    f << " <circle cx='17' cy='9' r='0.8' fill='white'/>\n";
    f << " <circle cx='21' cy='9' r='0.8' fill='white'/>\n";
    f << " <path d='M 17 14 Q 20 16 23 14' stroke='#1971c2' stroke-width='1.5' fill='none' stroke-linecap='round'/>\n";

    // Antenler
    f << " <line x1='14' y1='2' x2='14' y2='-2' stroke='#1971c2' stroke-width='1.5'/>\n";
    f << " <line x1='26' y1='2' x2='26' y2='-2' stroke='#1971c2' stroke-width='1.5'/>\n";
    f << " <circle cx='14' cy='-3' r='2.5' fill='#ff6b6b'>\n";
    f << " <animate attributeName='opacity' values='1;0.3;1' dur='1s' repeatCount='indefinite'/>\n";
    f << " </circle>\n";
    f << " <circle cx='26' cy='-3' r='2.5' fill='#ffd43b'>\n";
    f << " <animate attributeName='opacity' values='0.3;1;0.3' dur='1s' repeatCount='indefinite'/>\n";
    f << " </circle>\n";

    // Kollar
    f << " <rect x='2' y='8' width='5' height='10' rx='2.5' fill='#ff922b'/>\n";
    f << " <rect x='33' y='8' width='5' height='10' rx='2.5' fill='#ff922b'/>\n";
    f << " <circle cx='4.5' cy='19' r='3' fill='#ffd43b'/>\n";
    f << " <circle cx='35.5' cy='19' r='3' fill='#ffd43b'/>\n";

    // Gövde
    f << " <rect x='10' y='22' width='20' height='12' rx='3' fill='#1971c2'/>\n";
    f << " <circle cx='15' cy='28' r='2' fill='#ffd43b'/>\n";
    f << " <circle cx='25' cy='28' r='2' fill='#ffd43b'/>\n";

    // Bacaklar
    f << " <rect x='12' y='34' width='6' height='4' rx='1' fill='#495057'/>\n";
    f << " <rect x='22' y='34' width='6' height='4' rx='1' fill='#495057'/>\n";

    // Gölge
    f << " <ellipse cx='20' cy='40' rx='12' ry='2' fill='black' opacity='0.15'/>\n";

    f << " </g>\n";
    f << "</g>\n\n";

    // Lejant
    double legend_x = sp.width - 160;
    double legend_y = 75;

    f << "<g id='legend'>\n";
    f << " <rect x='" << legend_x << "' y='" << legend_y
        << "' width='150' height='100' rx='8' fill='white' opacity='0.95' "
        << "stroke='#dee2e6' stroke-width='1' filter='url(#shadow)'/>\n";

    f << " <text x='" << (legend_x + 10) << "' y='" << (legend_y + 20)
        << "' font-family='Arial, sans-serif' font-size='12' font-weight='bold' fill='#212529'>Lejant</text>\n";

    // Lejant öğeleri
    double ly = legend_y + 38;
    f << " <circle cx='" << (legend_x + 15) << "' cy='" << ly
        << "' r='2' fill='#adb5bd'/>\n";
    f << " <text x='" << (legend_x + 25) << "' y='" << (ly + 4)
        << "' font-family='Arial, sans-serif' font-size='10' fill='#495057'>LIDAR Noktası</text>\n";

    ly += 18;
    f << " <line x1='" << (legend_x + 10) << "' y1='" << ly
        << "' x2='" << (legend_x + 20) << "' y2='" << ly
        << "' stroke='#51cf66' stroke-width='2'/>\n";
    f << " <text x='" << (legend_x + 25) << "' y='" << (ly + 4)
        << "' font-family='Arial, sans-serif' font-size='10' fill='#495057'>Tespit Edilen Doğru</text>\n";

    ly += 18;
    double l_mark_size = 5;
    f << "  <g transform='translate(" << (legend_x + 15) << "," << ly << ")'>\n";
    f << "    <line x1='" << -l_mark_size << "' y1='" << -l_mark_size
      << "' x2='" << l_mark_size << "' y2='" << l_mark_size
      << "' stroke='url(#intersectGrad)' stroke-width='2' stroke-linecap='round'/>\n";
    f << "    <line x1='" << -l_mark_size << "' y1='" << l_mark_size
      << "' x2='" << l_mark_size << "' y2='" << -l_mark_size
      << "' stroke='url(#intersectGrad)' stroke-width='2' stroke-linecap='round'/>\n";
    f << "  </g>\n";
    f << "  <text x='" << (legend_x + 25) << "' y='" << (ly + 4)
      << "' font-family='Arial, sans-serif' font-size='10' fill='#495057'>Kesişim Noktası</text>\n";

    ly += 18;
    f << " <g transform='translate(" << (legend_x + 15) << "," << (ly - 3) << ") scale(0.5)'>\n";
    f << " <rect x='-8' y='-6' width='16' height='12' rx='2' fill='#4dabf7'/>\n";
    f << " <circle cx='0' cy='4' r='1.5' fill='#ffd43b'/>\n";
    f << " </g>\n";
    f << " <text x='" << (legend_x + 25) << "' y='" << (ly + 4)
        << "' font-family='Arial, sans-serif' font-size='10' fill='#495057'>Robot Konumu</text>\n";

    f << "</g>\n";

    f << "</svg>\n";
    f.close();
}