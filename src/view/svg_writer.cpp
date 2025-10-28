#include "view/svg_writer.hpp"
#include <iostream>  // std::cout, std::cerr için
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <cmath>

static void expandBounds(double& minx,double& miny,double& maxx,double& maxy, const Point& p){
    minx = std::min(minx,p.x); miny = std::min(miny,p.y);
    maxx = std::max(maxx,p.x); maxy = std::max(maxy,p.y);
}

void saveToSVG(const std::string& out,
               const std::vector<Point>& pts,
               const std::vector<Line>& segs,
               const std::vector<Intersection>& xs,
               const SvgParams& sp)
{
    std::ofstream f(out);
    if(!f.is_open()){ std::cerr << "[!] SVG acilamadi: " << out << "\n"; return; }

    double minx=0,miny=0,maxx=0,maxy=0;
    bool first=true;
    auto feed=[&](const Point& p){
        if(first){minx=maxx=p.x; miny=maxy=p.y; first=false;}
        else expandBounds(minx,miny,maxx,maxy,p);
    };
    feed({0,0});
    for(auto&p:pts)feed(p);
    for(auto&s:segs){feed(s.startPoint);feed(s.endPoint);}
    for(auto&i:xs)feed(i.position);
    if(first){minx=-1;miny=-1;maxx=1;maxy=1;}

    double W=sp.width-2*sp.margin, H=sp.height-2*sp.margin;
    double sx=W/std::max(1e-9,(maxx-minx)), sy=H/std::max(1e-9,(maxy-miny));
    double s=std::min(sx,sy);
    auto Sx=[&](double x){return sp.margin+(x-minx)*s;};
    auto Sy=[&](double y){return sp.height-(sp.margin+(y-miny)*s);};

    f << "<svg xmlns='http://www.w3.org/2000/svg' width='"<<sp.width<<"' height='"<<sp.height<<"'>\n";
    f << "<rect width='100%' height='100%' fill='white'/>\n";

    for(auto&p:pts)
        f<<"<circle cx='"<<Sx(p.x)<<"' cy='"<<Sy(p.y)<<"' r='2' fill='lightgray'/>\n";

    f<<"<circle cx='"<<Sx(0)<<"' cy='"<<Sy(0)<<"' r='4' fill='red'/>\n";

    for(auto&s:segs)
        f<<"<line x1='"<<Sx(s.startPoint.x)<<"' y1='"<<Sy(s.startPoint.y)
         <<"' x2='"<<Sx(s.endPoint.x)<<"' y2='"<<Sy(s.endPoint.y)
         <<"' stroke='green' stroke-width='2'/>\n";

    for(auto&i:xs){
        double x=Sx(i.position.x),y=Sy(i.position.y);
        f<<"<path d='M "<<x-6<<" "<<y-6<<" L "<<x+6<<" "<<y+6
         <<" M "<<x-6<<" "<<y+6<<" L "<<x+6<<" "<<y-6
         <<"' stroke='orange' stroke-width='2'/>\n";
        f<<"<line x1='"<<Sx(0)<<"' y1='"<<Sy(0)<<"' x2='"<<x<<"' y2='"<<y
         <<"' stroke='red' stroke-width='1.5' stroke-dasharray='5,5'/>\n";
        f<<"<text x='"<<x+6<<"' y='"<<y-6<<"' font-size='12' fill='black'>"
         <<std::fixed<<std::setprecision(2)<<i.distanceToRobot<<"m / "
         <<(int)std::round(i.angleDeg)<<"°</text>\n";
    }

    f << "</svg>\n";

}
