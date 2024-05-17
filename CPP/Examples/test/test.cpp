#include <iostream>
#include "clipper2/clipper.h"
#include "../../Utils/clipper.svg.utils.h"
#include "../../Utils/ClipFileLoad.h"
#include "../../Utils/ClipFileSave.h"
#include <vector>
#include <cmath>
using namespace std;
using namespace Clipper2Lib;

PathD MirrorPolygon(const PathD &polygon, double originX, double originY)
{
    PathD mirrored_polygon;
    for (const PointD &pt : polygon)
    {
        // 相对于原点 (originX, originY) 进行镜像
        double mirrored_x = 2 * originX - pt.x;
        double mirrored_y = 2 * originY - pt.y;
        mirrored_polygon.push_back(PointD(mirrored_x, mirrored_y));
    }
    return mirrored_polygon;
}

// void Minkowski();
// void DisplayPolygons(const Paths64 &polygons);
// void System(const std::string &filename);

// int main()
// {
//     Minkowski();
// }

// void Minkowski()
// {
//     PolyPath64 pA, pB;

//     Path64 polygonA = MakePath({0, 0, 100, 0, 100, 100, 0, 100, 0, 0});
//     Path64 polygonB = MakePath({0, 100, 200, 0, 200, 100, 0, 200, 0, 100});

//     Path64 polygonA_2 = TranslatePath(polygonA, 300, 300);

//     Rect64 rect = GetBounds(polygonA);
//     Rect64 rect_2 = GetBounds(polygonA_2);
//     Rect64 rect_3 = GetBounds(polygonB);

//     polygonA = TranslatePath(polygonA, rect.left, -rect.bottom);
//     polygonA_2 = TranslatePath(polygonA_2, rect_2.left, -rect_2.bottom);
//     polygonB = TranslatePath(polygonB, rect_3.left, -rect_3.bottom);

//     if (Area(polygonA) > 0)
//     {
//         std::cout << "polygonA is valid" << std::endl;
//         std::reverse(polygonA.begin(), polygonA.end());
//         std::cout << GetBounds(polygonA) << std::endl;
//     }
//     if (Area(polygonA_2) > 0)
//     {
//         std::cout << "polygonA_2 is valid" << std::endl;
//         std::cout << GetBounds(polygonA_2) << std::endl;
//     }
//     if (Area(polygonB) > 0)
//     {
//         std::cout << "polygonB is valid" << std::endl;
//         std::cout << GetBounds(polygonB) << std::endl;
//     }
//     // Path64 holes1, holes2, holes3;
//     // holes1 = MakePath({10, 10, 30, 10, 20, 20, 10, 20, 10, 10});
//     // holes3 = MakePath({10, 10, 30, 10, 20, 20, 10, 20, 10, 10});
//     // std::cout << holes3 << std::endl;
//     // std::reverse(holes1.begin(), holes1.end());
//     // std::cout << holes1 << std::endl;
//     // holes2 = MakePath({10, 30, 20, 30, 20, 40, 10, 40, 50, 30});
//     // std::reverse(holes2.begin(), holes2.end());
//     // std::cout << holes2 << std::endl;
//     // // holes3 = MakePath({10, 50, 20, 50, 20, 60, 10, 60, 20, 50});
//     // // holes3.reserve(5);

//     // pA.AddChild(polygonA);
//     // pA.AddChild(holes1);
//     // pA.AddChild(holes2);

//     Paths64 minkowskiSum = MinkowskiSum(polygonA, polygonB, true);
//     std::cout << GetBounds(minkowskiSum) << std::endl;
//     Paths64 minkowskiSum2 = MinkowskiSum(polygonA_2, polygonB, true);
//     std::cout << GetBounds(minkowskiSum2) << std::endl;
//     Paths64 minkowskiSum3 = MinkowskiSum(polygonB, polygonA_2, true);
//     std::cout << GetBounds(minkowskiSum3) << std::endl;

//     std::cout << "Minkowski Sum of Polygon A and Polygon B:" << std::endl;

//     DisplayPolygons(minkowskiSum);

//     Paths64 p1, p2;
//     p1.push_back(polygonA);
//     p1.push_back(polygonB);
//     p1.push_back(polygonA_2);

//     SvgWriter svg2;
//     // Paths64 temp1, temp2;
//     // for (size_t i = 0; i < pA.Count(); i++)
//     // {
//     //     if (pA[i]->IsHole())
//     //     {
//     //         temp1.push_back(pA[i]->Polygon());
//     //     }
//     //     else
//     //     {
//     //         temp2.push_back(pA[i]->Polygon());
//     //     }
//     // }
//     // SvgAddSubject(svg2, temp1, FillRule::Negative);
//     // SvgAddSubject(svg2, temp2, FillRule::Positive);
//     SvgAddSubject(svg2, p1, FillRule::NonZero);
//     // SvgAddSubject(svg2, p2, FillRule::NonZero);
//     SvgAddSolution(svg2, minkowskiSum, FillRule::NonZero, false);
//     SvgAddCaption(svg2, "Minkowski", minkowskiSum[0][0].x, minkowskiSum[0][0].y);
//     SvgAddSolution(svg2, minkowskiSum2, FillRule::NonZero, false);
//     SvgAddCaption(svg2, "Minkowski", minkowskiSum2[0][0].x, minkowskiSum2[0][0].y);
//     SvgAddSolution(svg2, minkowskiSum3, FillRule::NonZero, false);
//     SvgAddCaption(svg2, "Minkowski", minkowskiSum3[0][0].x, minkowskiSum3[0][0].y);
//     // SvgAddCaption(svg2, "Minkowski", 0, 0);
//     SvgSaveToFile(svg2, "test_min.svg", 800, 600, 20);
//     System("test_min.svg");
// }

// // Function to display polygons
// void DisplayPolygons(const Paths64 &polygons)
// {
//     for (const auto &polygon : polygons)
//     {
//         std::cout << "(" << polygon << ") ";
//     }
// }

// void System(const std::string &filename)
// {
// #ifdef _WIN32
//     system(filename.c_str());
// #else
//     system(("firefox " + filename).c_str());
// #endif
// }

std::pair<double, double> findClosestCoordinate(PathsD &coords, double targetX, double targetY)
{
    // 目标坐标
    double minDistance = std::numeric_limits<double>::max();
    std::pair<double, double> closestCoord;

    // 遍历坐标列表，两个元素为一组
    for (size_t i = 0; i < coords.size(); i++)
    {
        for (size_t j = 0; j < coords[0].size(); j++)
        {
            double x = coords[i][j].x;
            double y = coords[i][j].y;
            // 计算和目标坐标的距离
            double distance = std::abs(x - targetX) + std::abs(y - targetY);
            // 更新最小距离和对应的坐标
            if (distance < minDistance)
            {
                minDistance = distance;
                closestCoord = {x, y};
            }
        }
    }

    return closestCoord;
}

int main()
{
    // 示例坐标列表
    PathsD coords, p2;
    // coords.push_back(MakePathD({431.148, -680.344, 431.57, -680.273, 432.211, -680.156, 432.844, -679.969, 433.242, -679.812}));
    // coords.push_back(MakePathD({251.672, -676.289, 251.289, -676.172, 210.969, -652.883, 210.797, -652.719, 210.5, -652.359, 210.383, -652.172}));
    // coords.push_back(MakePathD({429.641, -676.203, 429.43, -676.117, 429.406, -676.109, 428.852, -675.781, 412.227, -665.375, 427.938, -656.312}));

    // p2.push_back(MakePathD({
    //     97.289,
    //     -676.18,
    //     278.773,
    //     -665.484,
    //     278.945,
    //     -665.391,
    //     316.664,
    //     -665.422,
    //     299.781,
    //     -675.883,
    // }));
    // p2.push_back(MakePathD({383.406, -676.102, 382.852, -675.773, 366.312, -665.422, 403.953, -665.445, 404.023, -665.484}));

    PathD p1 = MakePathD({0, 0, 10, 10, 20, 0, 0, 0});
    coords.push_back(p1);
    PathD r2 = MakePathD({0, 0, 0, 5, 5, 5, 5, 0, 0, 0});
    std::reverse(r2.begin(), r2.end());
    p2.push_back(r2);

    PathD mirr_p = MirrorPolygon(p1, 0.0, 0.0);
    // coords.push_back(mirr_p);

    PathsD result = MinkowskiSum(p1, r2, true);

    RectD r2_rect = GetBounds(r2);

    PathD temp_r2 = TranslatePath(r2, result[1][0].x - r2_rect.right, result[1][0].y - r2_rect.bottom);

    p2.push_back(temp_r2);
    SvgWriter svg2;

    SvgAddSubject(svg2, coords, FillRule::NonZero);
    SvgAddSubject(svg2, p2, FillRule::NonZero);
    SvgAddSubject(svg2, result, FillRule::NonZero);
    SvgSaveToFile(svg2, "ifp.svg", 500, 500, 0);
    SvgAddCaption(svg2, "ifp", result[0][0].x, result[0][0].y);
    // std::pair<double, double>
    //     result = findClosestCoordinate(coords, 0, 0);
    // std::cout << "result: " << result.first << " " << result.second << std::endl;

    return 0;
}
