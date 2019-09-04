
#include <cstdio>
#include <string.h>
#include <strings.h>
#include <sstream>
#include <stdio.h> // for file output
#include <fstream>
#include <iostream>
#include <algorithm> // random_shuffle

#include <boost/version.hpp>

#include <unordered_set>
#include <unordered_map>

#include "utils/logoutput.h"
#include "utils/polygon.h"
#include "utils/gettime.h"
#include "utils/SVG.h"

#include "VoronoiQuadrangulation.h"
#include "DistributedBeadingStrategy.h"
#include "InwardDistributedBeadingStrategy.h"
#include "LimitedDistributedBeadingStrategy.h"
#include "SingleBeadBeadingStrategy.h"
#include "utils/VoronoiUtils.h"
#include "NaiveBeadingStrategy.h"
#include "CenterDeviationBeadingStrategy.h"
#include "WideningBeadingStrategy.h"
#include "ConstantBeadingStrategy.h"
#include "BeadingOrderOptimizer.h"
#include "GcodeWriter.h"
#include "Statistics.h"

#include "TestGeometry/Pika.h"
#include "TestGeometry/Jin.h"
#include "TestGeometry/Moessen.h"
#include "TestGeometry/Prescribed.h"
#include "TestGeometry/Spiky.h"
#include "TestGeometry/SVGloader.h"
#include "TestGeometry/Microstructure.h"

#include "TestGeometry/VariableWidthGcodeTester.h"

using arachne::Point;

namespace arachne
{

Polygons generateTestPoly(int size, Point border)
{
    Polygons polys;
    PolygonRef poly = polys.newPoly();
    for (int i = 0; i < size; i++)
    {
        poly.emplace_back(rand() % border.X, rand() % border.Y);
    }
    polys = polys.execute(ClipperLib::pftPositive);
    return polys;
}

static Polygons test_poly_1;
static Polygons squares;
static Polygons circle;
static Polygons circle_flawed;
static Polygons cross_shape;
static Polygons gMAT_example;
static Polygons test_various_aspects;
static Polygons simple_MAT_example;
static Polygons simple_MAT_example_rounded_corner;
static Polygons beading_conflict;
static Polygons legend;
static Polygons wedge;
static Polygons limit_wedge;
static Polygons double_wedge;
static Polygons flawed_wedge;
static Polygons clean_and_flawed_wedge_part;
static Polygons rounded_wedge;
static Polygons flawed_wall;
static Polygons marked_local_opt;
static Polygons parabola;
static Polygons pikachu;
static Polygons jin;
static Polygons um;
static Polygons spikes;
static Polygons enclosed_region;

void generateTestPolys()
{
    PolygonRef poly = test_poly_1.newPoly();
    poly.emplace_back(0, 0);
    poly.emplace_back(10000, 0);
    poly.emplace_back(5000, 1000);
    poly.emplace_back(4000, 2000);
    poly.emplace_back(3000, 5000);
    poly.emplace_back(2000, 6000);
    poly.emplace_back(1000, 5000);
    poly.emplace_back(0, 3000);
    PolygonRef hole = test_poly_1.newPoly();
    hole.emplace_back(1000,1000);
    hole.emplace_back(1100,900);
    hole.emplace_back(1000,900);

    PolygonRef square_1 = squares.newPoly();
    square_1.emplace_back(0, 0);
    square_1.emplace_back(0, 10000);
    square_1.emplace_back(10000, 10000);
    square_1.emplace_back(10000, 0);
    
    
    PolygonRef circle_1 = circle.newPoly();
    coord_t r = 10000;
    for (float a = 0; a < 360; a += 10)
    {
        float rad = a / 180 * M_PI;
        circle_1.emplace_back(r * cos(rad), r * sin(rad));
    }
    
    PolygonRef circle_flawed_1 = circle_flawed.newPoly();
    for (float a = 0; a < 360; a += 10)
    {
        r = 5000 + rand() % 2500;
        a += (rand() % 100) / 50.0;
        float rad = a / 180 * M_PI;
        circle_flawed_1.emplace_back(r * cos(rad), r * sin(rad));
    }
    
    {
        PolygonRef cross1 = cross_shape.newPoly();
        cross1.emplace_back(400, 0);
        cross1.emplace_back(400,400);
        cross1.emplace_back(0,400);
        cross1.emplace_back(0,500);
        cross1.emplace_back(400,500);
        cross1.emplace_back(400,900);
        cross1.emplace_back(500,900);
        cross1.emplace_back(500,500);
        cross1.emplace_back(900,500);
        cross1.emplace_back(900,400);
        cross1.emplace_back(500,400);
        cross1.emplace_back(500,0);
        PointMatrix scaler = PointMatrix::scale(13.8);
        cross1.applyMatrix(scaler);
    }

    {
        PolygonRef gMAT_example_outline = gMAT_example.newPoly();
        gMAT_example_outline.emplace_back(0, -20);
        gMAT_example_outline.emplace_back(7000, -20);
        gMAT_example_outline.emplace_back(8050, -20);
        gMAT_example_outline.emplace_back(8050, 2000);
        gMAT_example_outline.emplace_back(7000, 2000);
        gMAT_example_outline.emplace_back(7000, 11500);
        gMAT_example_outline.emplace_back(6500, 12000);
        gMAT_example_outline.emplace_back(0, 12000);
        PolygonRef gMAT_example_triangle = gMAT_example.newPoly();
        gMAT_example_triangle.emplace_back(1000, 7000);
        gMAT_example_triangle.emplace_back(1000, 7400);
        gMAT_example_triangle.emplace_back(5000, 9000);
        gMAT_example_triangle.emplace_back(1000, 10600);
        gMAT_example_triangle.emplace_back(1000, 11000);
        gMAT_example_triangle.emplace_back(6000, 9000);
        PolygonRef gMAT_example_round = gMAT_example.newPoly();
        gMAT_example_round.emplace_back(1000, 2000);
        gMAT_example_round.emplace_back(1000, 5600);
        gMAT_example_round.emplace_back(2000, 6000);
        gMAT_example_round.emplace_back(5000, 6000);
        gMAT_example_round.emplace_back(5000, 3000);
        gMAT_example_round.emplace_back(4000, 2000);
        gMAT_example_round.emplace_back(2200, 2000);
        gMAT_example_round.emplace_back(2200, 4400);

        gMAT_example_round.emplace_back(2500, 4700);
        gMAT_example_round.emplace_back(2800, 4800);
        gMAT_example_round.emplace_back(3000, 4800);
        gMAT_example_round.emplace_back(3300, 4700);
        
        gMAT_example_round.emplace_back(3600, 4400);
        gMAT_example_round.emplace_back(3600, 3000);
        gMAT_example_round.emplace_back(4200, 3000);
        gMAT_example_round.emplace_back(4200, 4400);
        
        gMAT_example_round.emplace_back(4000, 4800);
        gMAT_example_round.emplace_back(3800, 5000);
        gMAT_example_round.emplace_back(3400, 5200);
        gMAT_example_round.emplace_back(2600, 5200);
        gMAT_example_round.emplace_back(2200, 5000);
        gMAT_example_round.emplace_back(2000, 4800);
        
        gMAT_example_round.emplace_back(1800, 4400);
        gMAT_example_round.emplace_back(1800, 2000);
        
        PolygonRef circle_hole = gMAT_example.newPoly();
        coord_t r = 700;
        for (float a = 360; a > 0; a -= 360 / 37)
        {
            circle_hole.add(Point(cos(a/180 * M_PI) * r, sin(a/180 * M_PI) * r) + Point(1600,9000));
        }
    }
    {
        {
            PolygonRef poly = test_various_aspects.newPoly();
            float s = MM2INT(1);
            poly.emplace_back(s*4.333,s*0.000);
            poly.emplace_back(s*4.667,s*0.333);
            poly.emplace_back(s*4.667,s*1.083);
            poly.emplace_back(s*3.667,s*1.417);
            poly.emplace_back(s*4.667,s*1.667);
            poly.emplace_back(s*4.667,s*5.667);
            poly.emplace_back(s*-0.000,s*5.667);
            poly.emplace_back(s*0.000,s*3.000);
            poly.emplace_back(s*0.000,s*2.667);
            poly.emplace_back(s*-0.000,s*1.000);
            poly.emplace_back(s*0.107,s*0.676);
            poly.emplace_back(s*0.351,s*0.434);
            poly.emplace_back(s*0.651,s*0.262);
            poly.emplace_back(s*0.975,s*0.141);
            poly.emplace_back(s*1.312,s*0.061);
            poly.emplace_back(s*1.654,s*0.015);
            poly.emplace_back(s*2.000,s*0.000);
        }
        {
            PolygonRef poly = test_various_aspects.newPoly();
            float s = MM2INT(1);
            poly.emplace_back(s*0.195,s*3.000);
            poly.emplace_back(s*0.195,s*5.233);
            poly.emplace_back(s*2.333,s*5.233);
            poly.emplace_back(s*4.000,s*4.805);
            poly.emplace_back(s*2.333,s*4.389);
            poly.emplace_back(s*2.068,s*4.276);
            poly.emplace_back(s*1.877,s*4.021);
            poly.emplace_back(s*1.756,s*3.708);
            poly.emplace_back(s*1.676,s*3.258);
            poly.emplace_back(s*1.676,s*2.797);
            poly.emplace_back(s*1.756,s*2.347);
            poly.emplace_back(s*1.877,s*2.035);
            poly.emplace_back(s*2.068,s*1.779);
            poly.emplace_back(s*2.333,s*1.667);
            poly.emplace_back(s*3.333,s*1.417);
            poly.emplace_back(s*2.333,s*1.000);
            poly.emplace_back(s*0.195,s*1.000);
            poly.emplace_back(s*0.195,s*2.667);
        }
    }
    
    {
        PolygonRef simple_MAT_example_ = simple_MAT_example.newPoly();
        simple_MAT_example_.emplace_back(0, 2000);
        simple_MAT_example_.emplace_back(1000, 2000);
        simple_MAT_example_.emplace_back(400, 1000);
        simple_MAT_example_.emplace_back(1000, 0);
        simple_MAT_example_.emplace_back(0, 0);
    }
    
    {
        coord_t size = 200;
        PolygonRef p = simple_MAT_example_rounded_corner.newPoly();
        p.emplace_back(0, 2000);
        p.emplace_back(1000, 2000);
        p.emplace_back(400, 1000);
        p.emplace_back(1000, 0);
        p.emplace_back(size, 0);
        p.emplace_back(.65 * size, .25 * size);
        p.emplace_back(.25 * size, .65 * size);
        p.emplace_back(0, size);
    }
    {
        PolygonRef p = beading_conflict.newPoly();
        coord_t l = 1000;
//         coord_t a = 300;
//         coord_t b = 950;
        coord_t a = 600;
        coord_t b = 1395;
        coord_t dy = 2;
        coord_t dx = 5;
        p.emplace_back(l, 0);
        p.emplace_back(l, l);
        p.emplace_back(l-dx, l+dy);
        p.emplace_back(0, l);
        p.emplace_back(0, l + a);
        p.emplace_back(l-dx, l + a -dy);
        p.emplace_back(l, l + a);
        p.emplace_back(l, l * 2 + a);
        p.emplace_back(l + b, l * 2 + a);
        p.emplace_back(l + b, l + a);
        p.emplace_back(l + b +dx, l + a -dy);
        p.emplace_back(l * 2 + b, l + a);
        p.emplace_back(l * 2 + b, l);
        p.emplace_back(l + b +dx, l +dy);
        p.emplace_back(l + b, l);
        p.emplace_back(l + b, 0);
    }
    
    {
        PolygonRef p = legend.newPoly();
        
        coord_t l = 1000;
        coord_t gap = 400;
        coord_t bridge = 400;
        
        p.emplace_back(0, 0);
        p.emplace_back(0, l);
        p.emplace_back(l-gap/2, l);
        p.emplace_back(l, l/2 + bridge/2);
        p.emplace_back(l * 3/2, l * 2);
        p.emplace_back(l * 2, bridge);
        p.emplace_back(l * 2 + gap, l);
        p.emplace_back(l * 3, l);
        p.emplace_back(l * 3, 0);
        p.emplace_back(l + gap/2, 0);
        p.emplace_back(l, l/2 - bridge/2);
        p.emplace_back(l - gap/2, 0);
    }

    {
        PolygonRef wedge_1 = wedge.newPoly();
        wedge_1.emplace_back(2500, 0);
        wedge_1.emplace_back(0, 2500);
        wedge_1.emplace_back(20000, 20000);
//         PointMatrix scaler = PointMatrix::scale(.846); // .846 causes a transition which is just beyond the marked skeleton
        PointMatrix scaler = PointMatrix::scale(.5); // .846 causes a transition which is just beyond the marked skeleton
        wedge_1.applyMatrix(scaler);
        PointMatrix rot(-45);
        wedge_1.applyMatrix(rot);
    }

    {
        PolygonRef wedge_1 = limit_wedge.newPoly();
        coord_t length = 10000;
        wedge_1.emplace_back(0, 0);
        wedge_1.emplace_back(-length * tan(M_PI / 8), length + 100);
        wedge_1.emplace_back(length * tan(M_PI / 8), length + 100);
//         PointMatrix scaler = PointMatrix::scale(.846); // .846 causes a transition which is just beyond the marked skeleton
        PointMatrix scaler = PointMatrix::scale(.5); // .846 causes a transition which is just beyond the marked skeleton
        wedge_1.applyMatrix(scaler);
    }

    {
        PolygonRef wedge_1 = double_wedge.newPoly();
        wedge_1.emplace_back(2500, 0);
        wedge_1.emplace_back(-20000, -20000);
        wedge_1.emplace_back(0, 2500);
        wedge_1.emplace_back(20000, 20000);
//         PointMatrix scaler = PointMatrix::scale(.846); // .846 causes a transition which is just beyond the marked skeleton
        PointMatrix scaler = PointMatrix::scale(.5); // .846 causes a transition which is just beyond the marked skeleton
        wedge_1.applyMatrix(scaler);
        PointMatrix rot(-45);
        wedge_1.applyMatrix(rot);
    }

    rounded_wedge = wedge.offset(-400, ClipperLib::jtRound).offset(400, ClipperLib::jtRound); // TODO: this offset gives problems!!
//     rounded_wedge = wedge.offset(-200, ClipperLib::jtRound).offset(200, ClipperLib::jtRound); // TODO: this offset also gives problems!!
//     rounded_wedge = wedge.offset(-205, ClipperLib::jtRound).offset(205, ClipperLib::jtRound);
    
    {
        coord_t l = 20000;
        coord_t h = 2000;
        coord_t r = 200;
        coord_t step = 2000;
        PolygonRef flawed_wedgel_1 = flawed_wedge.newPoly();
        for (coord_t x = 0; x <= l; x += step)
        {
            flawed_wedgel_1.emplace_back(x, (h + rand() % r - r/2) * x / l);
        }
        for (coord_t x = l - step / 2; x >= 0; x -= 800)
        {
            flawed_wedgel_1.emplace_back(x, (rand() % r - r/2) * x / l);
        }
        
        Point3Matrix rot = Point3Matrix(PointMatrix(-90.0));
        flawed_wedgel_1.applyMatrix(rot);
    }
    {
        coord_t large_w = 2100;
        coord_t small_w = 610;
        coord_t h = 3000;
        coord_t d = 1400;
        coord_t step_count = 7;
        coord_t deflection = 20;
        PolygonRef poly = clean_and_flawed_wedge_part.newPoly();
        poly.emplace_back(0, 0);
        poly.emplace_back(large_w / 2 - small_w / 2, h);
        poly.emplace_back(large_w / 2 + small_w / 2, h);
        poly.emplace_back(large_w, 0);
        poly.emplace_back(large_w / 2 + small_w / 2, -h);
//         poly.emplace_back(large_w / 2 - small_w / 2, -h);
        Point from(large_w / 2 - small_w / 2, -h);
        Point to(0,0);
        bool alternate = true;
        for (coord_t step = 0; step < step_count; step++)
        {
            Point mid = from + (to - from) * step / step_count;
            mid += Point(alternate? deflection : -deflection, 0);
            poly.add(mid);
            alternate = !alternate;
        }
    }
    {
        coord_t l = 10000;
        coord_t h = 1000;
        coord_t r = 100;
        coord_t step = 2000;
        PolygonRef flawed_wall_1 = flawed_wall.newPoly();
        for (coord_t x = 0; x <= l; x += step)
        {
            flawed_wall_1.emplace_back(x, h + rand() % r - r/2);
        }
        for (coord_t x = l - step / 2; x >= 0; x -= 800)
        {
            flawed_wall_1.emplace_back(x, rand() % r - r/2);
        }
        
        Point3Matrix rot = Point3Matrix(PointMatrix(60.0));
        flawed_wall_1.applyMatrix(rot);
    }
    {
        PolygonRef marked_local_opt_1 = marked_local_opt.newPoly();
        marked_local_opt_1.emplace_back(5000, 0);
        marked_local_opt_1.emplace_back(0, 400);
        marked_local_opt_1.emplace_back(5000, 610);
        marked_local_opt_1.emplace_back(10000, 400);
        Point3Matrix rot = Point3Matrix(PointMatrix(60.0));
        marked_local_opt_1.applyMatrix(rot);
    }
    {
        PolygonRef pb = parabola.newPoly();
        coord_t w = 2000;
        coord_t h = 8000;
        coord_t step = 100;
        for (coord_t x = -w / 2; x <= w / 2; x += step)
        {
            pb.add(Point(x, x * x * h / w / w));
        }
    }
    
    pikachu = generatePika();
    
    jin = generateJin();
    
    {
        coord_t r = 3600;
        coord_t inr = 2400;
        coord_t b = 10000;
        coord_t h = b;
        PolygonRef um_1 = um.newPoly();
        um_1.emplace_back(r, r);
        um_1.emplace_back(r, h);
        um_1.emplace_back(b - r, h);
        um_1.emplace_back(b - r, r);
        um = um.offset(r, ClipperLib::jtRound);
        Polygon bb;
        bb.emplace_back(inr, inr);
        bb.emplace_back(inr, h + inr);
        bb.emplace_back(b - inr, h + inr);
        bb.emplace_back(b - inr, inr);
        Polygons bs;
        bs.add(bb);
        um = um.difference(bs);
        Polygon a;
        a.emplace_back(-r, h);
        a.emplace_back(-r, h + 2 * r);
        a.emplace_back(b + r, h + 2 * r);
        a.emplace_back(b + r, h);
        Polygons as;
        as.add(a);
        um = um.difference(as);
    }
    {
        coord_t min_r = 3000;
        coord_t max_r = 8000;
        Polygons circles;
        PolygonUtils::makeCircle(Point(0,0), 1600, circles);
        for (int a = 0; a < 360; a += 360 / 10)
        {
            Polygons dot;
            coord_t r = min_r + (max_r - min_r) * a / 360;
            PolygonUtils::makeCircle(Point(-r * cos(a /180.0 * M_PI), r * sin(a /180.0 * M_PI)), 10, dot);
            dot = dot.unionPolygons(circles);
            dot = dot.approxConvexHull();
            spikes = spikes.unionPolygons(dot);
        }
    }
    {
        PolygonRef outline = enclosed_region.newPoly();
        outline.emplace_back(0, 0);
        outline.emplace_back(0, 10000);
        outline.emplace_back(10000, 10000);
        outline.emplace_back(10000, 0);
        PolygonRef hole = enclosed_region.newPoly();
        hole.emplace_back(1000, 1000);
        hole.emplace_back(9000, 1000);
        hole.emplace_back(9000, 9000);
        hole.emplace_back(1000, 9000);
        PolygonRef outline2 = enclosed_region.newPoly();
        outline2.emplace_back(2000, 2000);
        outline2.emplace_back(2000, 8000);
        outline2.emplace_back(8000, 8000);
        outline2.emplace_back(8000, 2000);
        PolygonRef hole2 = enclosed_region.newPoly();
        hole2.emplace_back(4000, 4000);
        hole2.emplace_back(6000, 4000);
        hole2.emplace_back(6000, 6000);
        hole2.emplace_back(4000, 6000);

        PointMatrix rot = PointMatrix(170.0);
        enclosed_region.applyMatrix(rot);
    }
}


enum class StrategyType
{
    Naive,
    NaiveStrategy,
    Constant,
    Center,
    Distributed,
    InwardDistributed,
    LimitedDistributed,
    SingleBead,
    COUNT
};

std::string to_string(StrategyType type)
{
    switch (type)
    {
        case StrategyType::Naive: return "Naive";
        case StrategyType::NaiveStrategy: return "NaiveStrategy";
        case StrategyType::Constant: return "Constant";
        case StrategyType::Center: return "Center";
        case StrategyType::Distributed: return "Distributed";
        case StrategyType::InwardDistributed: return "InwardDistributed";
        case StrategyType::LimitedDistributed: return "LimitedDistributed";
        case StrategyType::SingleBead: return "SingleBead";
        default: return "unknown_strategy";
    }
}

BeadingStrategy* makeStrategy(StrategyType type, coord_t prefered_bead_width = MM2INT(0.5), float transitioning_angle = M_PI / 4, bool widening = false)
{
    BeadingStrategy* ret = nullptr;
    switch (type)
    {
        case StrategyType::NaiveStrategy:      ret = new NaiveBeadingStrategy(prefered_bead_width);                                      break;
        case StrategyType::Constant:           ret = new ConstantBeadingStrategy(prefered_bead_width, 4, .99 * M_PI);                    break;
        case StrategyType::Center:             ret = new CenterDeviationBeadingStrategy(prefered_bead_width, transitioning_angle);       break;
        case StrategyType::Distributed:        ret = new DistributedBeadingStrategy(prefered_bead_width, transitioning_angle);           break;
        case StrategyType::InwardDistributed:  ret = new InwardDistributedBeadingStrategy(prefered_bead_width, transitioning_angle);     break;
        case StrategyType::LimitedDistributed: ret = new LimitedDistributedBeadingStrategy(prefered_bead_width, 6, transitioning_angle); break;
        case StrategyType::SingleBead:         ret = new SingleBeadBeadingStrategy(prefered_bead_width, transitioning_angle);            break;
        default:
            logError("Cannot make strategy!\n");
            return nullptr;
    }
    if (widening)
    {
        return new WideningBeadingStrategy(ret, MM2INT(0.05), MM2INT(0.3));
    }
    else
    {
        return ret;
    }
}

void test(Polygons& polys, coord_t nozzle_size, std::string output_prefix, StrategyType type, bool generate_gcodes = true, bool analyse = false, bool generate_MAT_STL = false)
{
    std::string type_str = to_string(type);
    logAlways(">> Performing %s strategy...\n", type_str.c_str());
    float transitioning_angle = M_PI / 4;

    BeadingStrategy* beading_strategy = makeStrategy(type, nozzle_size, transitioning_angle);
    if (!beading_strategy) return;


    TimeKeeper tk;

    coord_t discretization_step_size = 200;
    coord_t transition_filter_dist = 1000;
    coord_t beading_propagation_transition_dist = 400;
    bool reduce_overlapping_segments = true;
    bool filter_outermost_marked_edges = false;
    if (type == StrategyType::SingleBead)
    {
        transition_filter_dist = 50;
        reduce_overlapping_segments = false;
    }
    else if (type == StrategyType::Constant)
    {
        filter_outermost_marked_edges = true;
    }
    VoronoiQuadrangulation vq(polys, transitioning_angle, discretization_step_size, transition_filter_dist, beading_propagation_transition_dist);

    std::vector<std::list<ExtrusionLine>> result_polylines_per_index = vq.generateToolpaths(*beading_strategy, filter_outermost_marked_edges);

    std::vector<std::list<ExtrusionLine>> result_polygons_per_index;
    BeadingOrderOptimizer::optimize(result_polygons_per_index, result_polylines_per_index, reduce_overlapping_segments);
    double processing_time = tk.restart();
    logAlways("Processing took %fs\n", processing_time);

    if (generate_gcodes)
    {
        AABB aabb(polys);
        {
            std::ostringstream ss;
            ss << "output/" << output_prefix << "_" << to_string(type) << "_arachne_P3.gcode";
            GcodeWriter gcode(ss.str(), GcodeWriter::type_P3);
            gcode.printBrim(aabb, 3, nozzle_size, nozzle_size * 1.5);
            gcode.print(result_polygons_per_index, result_polylines_per_index, aabb);
        }
//         if (false)
        {
            std::ostringstream ss;
            ss << "output/" << output_prefix << "_" << to_string(type) << "_arachne_UM3.gcode";
            GcodeWriter gcode(ss.str(), GcodeWriter::type_UM3);
            gcode.printBrim(aabb, 3, nozzle_size, nozzle_size * 1.5);
            gcode.print(result_polygons_per_index, result_polylines_per_index, aabb);
            logAlways("Writing gcode took %fs\n", tk.restart());
        }
    }

    if (generate_MAT_STL)
    {
        {
            STLwriter stl("output/vq_bead_count.stl");
            vq.debugOutput(stl, true);
        }
        logAlways("Writing MAT STL took %fs\n", tk.restart());
    }

    if (analyse)
    {
        Statistics stats(to_string(type), output_prefix, polys, processing_time);
        stats.analyse(result_polygons_per_index, result_polylines_per_index, &vq);
        logAlways("Analysis took %fs\n", tk.restart());
        stats.saveResultsCSV();
        stats.visualize(nozzle_size, true);
        logAlways("Visualization took %fs\n", tk.restart());
    }

    delete beading_strategy;

}

void testNaive(Polygons& polys, coord_t nozzle_size, std::string output_prefix, bool generate_gcodes = false, bool analyse = false)
{
    logAlways(">> Simulating naive method...\n");

    TimeKeeper tk;

    std::vector<Polygons> insets;
    Polygons last_inset = polys.offset(-nozzle_size / 2, ClipperLib::jtRound);
    while (!last_inset.empty())
    {
        insets.emplace_back(last_inset);
        last_inset = last_inset.offset(-nozzle_size, ClipperLib::jtRound);
    }
    double processing_time = tk.restart();
    logAlways("Naive processing took %fs\n", processing_time);

    std::vector<std::list<ExtrusionLine>> result_polygons_per_index;
    std::vector<std::list<ExtrusionLine>> result_polylines_per_index;
    result_polygons_per_index.resize(insets.size());
    for (coord_t inset_idx = 0; inset_idx < insets.size(); inset_idx++)
    {
        for (PolygonRef poly : insets[inset_idx])
        {
            constexpr bool is_odd = false;
            result_polygons_per_index[inset_idx].emplace_back(inset_idx, is_odd);
            ExtrusionLine& junction_poly = result_polygons_per_index[inset_idx].back();
            for (Point p : poly)
            {
                junction_poly.junctions.emplace_back(p, nozzle_size, inset_idx);
            }
        }
    }

    if (generate_gcodes)
    {
        AABB aabb(polys);
        {
            std::ostringstream ss;
            ss << "output/" << output_prefix << "_naive_arachne_P3.gcode";
            GcodeWriter gcode(ss.str(), GcodeWriter::type_P3);
            gcode.printBrim(aabb, 3, nozzle_size, nozzle_size * 1.5);
            gcode.print(result_polygons_per_index, result_polylines_per_index, aabb);
        }
//         if (false)
        {
            std::ostringstream ss;
            ss << "output/" << output_prefix << "_naive_arachne_UM3.gcode";
            GcodeWriter gcode(ss.str(), GcodeWriter::type_UM3);
            gcode.printBrim(aabb, 3, nozzle_size, nozzle_size * 1.5);
            gcode.print(result_polygons_per_index, result_polylines_per_index, aabb);
            logAlways("Writing gcodes took %fs\n", tk.restart());
        }
    }
    
    if (analyse)
    {
        Statistics stats("naive", output_prefix, polys, processing_time);
        stats.analyse(result_polygons_per_index, result_polylines_per_index);
        stats.saveResultsCSV();
        logAlways("Analysis took %fs\n", tk.restart());
        stats.visualize(nozzle_size);
        logAlways("Visualization took %fs\n", tk.restart());
    }
    
}

void writeVarWidthTest()
{
    std::vector<std::list<ExtrusionLine>> result_polygons_per_index;
    std::vector<std::list<ExtrusionLine>> result_polylines_per_index;
    result_polylines_per_index = VariableWidthGcodeTester::zigzag();

    AABB aabb;
    for (auto ps : result_polylines_per_index)
        for (auto p : ps)
            for (ExtrusionJunction& j : p.junctions)
                aabb.include(j.p);
    
        
    {
        std::ostringstream ss;
        ss << "output/variable_width_test_P3.gcode";
        GcodeWriter gcode(ss.str(), GcodeWriter::type_P3, 200);
        gcode.printBrim(aabb, 3);
        gcode.print(result_polygons_per_index, result_polylines_per_index, aabb);
    }
//     if (false)
    {
        std::ostringstream ss;
        ss << "output/variable_width_test_UM3.gcode";
        GcodeWriter gcode(ss.str(), GcodeWriter::type_UM3, 200);
        gcode.printBrim(aabb, 3);
        gcode.print(result_polygons_per_index, result_polylines_per_index, aabb);
    }
    
    Polygons fake_outline; fake_outline.add(aabb.toPolygon());
    Statistics stats("var_width", "test", fake_outline, 1.0);
    stats.analyse(result_polygons_per_index, result_polylines_per_index);
    stats.visualize(400, false, true, true, false, false);
}

void test(std::string input_outline_filename, std::string output_prefix)
{
//     writeVarWidthTest();
//     std::exit(0);

    // Preparing Input Geometries.
    int r;
    r = time(0);
    r = 1566731558;
    srand(r);
//     logAlways("r = %d;\n", r);
//     logDebug("boost version: %s\n", BOOST_LIB_VERSION);
    
    
    // problem of 2 nearby 3-way intersections I wouldn't know the solution to
    /*
    srand(1564134608);
    Polygons polys = generateTestPoly(30, Point(20000, 20000));
    AABB ab(Point(16436,6754) - Point(1000,1000), Point(16436,6754) + Point(1000,1000));
    Polygons abs; abs.add(ab.toPolygon());
    polys = polys.intersection(abs);
    */
    
    generateTestPolys();

    /*
    Polygons polys = SVGloader::load(input_outline_filename);
    AABB aabb(polys);
    polys.applyMatrix(Point3Matrix::translate(aabb.min * -1));
    */
    
    /*
    AABB aabb(Point(0,0), Point(1000,1000));
    Polygons qwe;
    qwe.add(aabb.toPolygon());
    Statistics stats("asf", "asf", qwe, 1.0);
    
    std::vector<std::list<ExtrusionLine>> result_polylines_per_index;

    std::vector<std::list<ExtrusionLine>> result_polygons_per_index;
    result_polylines_per_index.emplace_back();
    ExtrusionLine line(0, false);
    line.junctions.emplace_back(ExtrusionJunction(Point(0,0), 300, 0));
    line.junctions.emplace_back(ExtrusionJunction(Point(10000,0), 500, 0));
    result_polylines_per_index.back().emplace_back(line);
    stats.analyse(result_polygons_per_index, result_polylines_per_index);
    stats.visualize(400);
    
    std::exit(0);*/
    
    
    /*
    coord_t scale = 10000;
    Point p(20000,70000);
    AABB ab(p - scale * Point(1,1), p + scale * Point(1,1));
    Polygons abs; abs.add(ab.toPolygon());
    polys = polys.intersection(abs);
    */

    /*
    Polygons polys = generateTestPoly(40, Point(20000, 20000));
    coord_t scale = 2000;
    AABB ab(Point(6000,10000) - scale * Point(1,1), Point(6000,10000) + scale * Point(1,1));
    Polygons abs; abs.add(ab.toPolygon());
    polys = polys.intersection(abs);*/

    PointMatrix mirror = PointMatrix::scale(1);
    mirror.matrix[3] = -1;

//     Polygons polys = test_poly_1;
//     Polygons polys = squares;
//     Polygons polys = circle;
//     Polygons polys = circle_flawed;
//     Polygons polys = cross_shape;
    Polygons polys = gMAT_example; polys.applyMatrix(mirror);
//     Polygons polys = test_various_aspects; polys.applyMatrix(PointMatrix::scale(2.2));
//     Polygons polys = simple_MAT_example; polys.applyMatrix(PointMatrix::scale(3)); polys.applyMatrix(PointMatrix(-90));
//     Polygons polys = simple_MAT_example_rounded_corner; polys.applyMatrix(PointMatrix::scale(3)); polys.applyMatrix(PointMatrix(-90));
//     Polygons polys = beading_conflict;
//     Polygons polys = wedge; // polys.applyMatrix(PointMatrix::scale(3));
//     Polygons polys = wedge; polys.applyMatrix(PointMatrix::scale(3));
//     Polygons polys = limit_wedge; //polys.applyMatrix(PointMatrix::scale(3));
//     Polygons polys = double_wedge; // polys.applyMatrix(PointMatrix::scale(3));
//     Polygons polys = flawed_wedge;
//     Polygons polys = clean_and_flawed_wedge_part; polys.applyMatrix(mirror);
//     Polygons polys = flawed_wall;
//     Polygons polys = marked_local_opt;
//     Polygons polys = legend;
//     Polygons polys = parabola;
//     Polygons polys = pikachu; polys.applyMatrix(PointMatrix::scale(1)); polys.applyMatrix(mirror);
//     Polygons polys = um;
//     Polygons polys = spikes;
//     Polygons polys = enclosed_region;
//     Polygons polys = jin;
//     Microstructure m; Polygons polys = m.squareGrid(Point(2,2), Point(2000,2000));
//     Microstructure m; Polygons polys = m.hexGrid(Point(12,12), 8000); polys.applyMatrix(PointMatrix::scale(0.5));
//     Polygons polys = MoessenTests::generateCircles(Point(3, 3), 100, 400, 500, 52);
//     Polygons polys = MoessenTests::generateCircles(Point(2, 2), 100, 400, 500, 8);
//     srand(1563874501); Polygons polys = MoessenTests::generateCircles(Point(3, 3), 100, 400, 1000, 8);
//     Polygons polys = MoessenTests::generateTriangles(Point(4, 2), 100, 600, 1000);
//     Polygons polys = MoessenTests::generateTriangles(Point(2, 1), 100, 500, 1000);
//     Polygons polys = MoessenTests::generateTriangles(Point(4, 2), 300, 301, 1000);
//     Polygons polys = MoessenTests::generateTriangles(Point(4, 2), 400, 401, 1000);
//     Polygons polys = Prescribed::fromDistances({Point(0,800), Point(400,300), Point(610,610), Point(1400, 200)});
//     Polygons polys = Spiky::oneSpike(200);
//     Polygons polys = Spiky::twoSpikes();
//     Polygons polys = Spiky::twoSpikesDiamond(MM2INT(0.8), MM2INT(4.0), MM2INT(.1));
//     polys.applyMatrix(PointMatrix(45.0));
//     Polygons polys = Spiky::oneSpikeOneCorner(MM2INT(0.8), MM2INT(4.0), MM2INT(.1));
//     Polygons polys = Spiky::fourSpikes();
//     Polygons polys = Spiky::doubleOutSpike(800, 380);

    polys = polys.unionPolygons();
    polys.simplify();

#ifdef DEBUG
    {
        SVG svg("output/outline_viz.svg", AABB(polys));
        svg.writeAreas(polys, SVG::Color::NONE, SVG::Color::BLACK);
    }
    {
        SVG svg("output/outline.svg", AABB(polys), INT2MM(1));
        svg.writeAreas(polys, SVG::Color::NONE, SVG::Color::BLACK);
    }
#endif

    coord_t nozzle_size = MM2INT(0.6);
    polys.applyMatrix(PointMatrix::scale(INT2MM(nozzle_size) / 0.4));

    if (false && output_prefix.compare("TEST") != 0)
    {
        std::ostringstream ss;
        ss << "output/" << output_prefix << "_" << to_string(StrategyType::InwardDistributed) << "_results.csv";
        std::ifstream file(ss.str().c_str());
        if (file.good())
        {
            logAlways("Test already has results saved\n");
            std::exit(-1);
        }
    }

    bool generate_gcodes = false;
    bool analyse = true;
    bool generate_MAT_STL = true;

//     std::vector<StrategyType> strategies({ StrategyType::Naive, StrategyType::Center, StrategyType::InwardDistributed });
    std::vector<StrategyType> strategies({ StrategyType::Naive, StrategyType::Distributed });
//     std::vector<StrategyType> strategies({ StrategyType::Distributed });
//     std::vector<StrategyType> strategies({ StrategyType::InwardDistributed });
//     std::vector<StrategyType> strategies({ StrategyType::Center });
//     std::vector<StrategyType> strategies({ StrategyType::Distributed, StrategyType::InwardDistributed });
//     std::vector<StrategyType> strategies({ StrategyType::Constant, StrategyType::Center, StrategyType::Distributed, StrategyType::InwardDistributed, StrategyType::Naive });
//     std::random_shuffle(strategies.begin(), strategies.end());
    for (StrategyType type : strategies )
    {
        if (type == StrategyType::Naive)
        {
            testNaive(polys, nozzle_size, output_prefix, generate_gcodes, analyse);
        }
        else
        {
            test(polys, nozzle_size, output_prefix, type, generate_gcodes, analyse, generate_MAT_STL);
        }
    }
}



} // namespace arachne

int main(int argc, char *argv[])
{
    std::string input_outline_filename;
    std::string output_prefix;
    if (argc >= 2) input_outline_filename = argv[1];
    if (argc >= 3) output_prefix = argv[2];
    long n = 1;
    for (int i = 0; i < n; i++)
    {
        arachne::test(input_outline_filename, output_prefix);
//         if (++i % std::max(1l, n / 100) == 0)
//             std::cerr << (i / 100) << "%\n";
    }
    return 0;
}
