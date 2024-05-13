#include <iostream>
#include "clipper2/clipper.h"
#include "../../Utils/clipper.svg.utils.h"
#include "../../Utils/ClipFileLoad.h"
#include "../../Utils/ClipFileSave.h"

using namespace std;
using namespace Clipper2Lib;

void Minkowski();
void DisplayPolygons(const Paths64 &polygons);
void System(const std::string &filename);

int main()
{
    Minkowski();
}

void Minkowski()
{
    PolyPath64 pA, pB;

    Path64 polygonA = MakePath({0, 0, 100, 0, 100, 100, 0, 100, 0, 0});
    Path64 polygonB = MakePath({0, 100, 200, 0, 200, 100, 0, 200, 0, 100});

    Path64 holes1, holes2, holes3;
    holes1 = MakePath({10, 10, 30, 10, 20, 20, 10, 20, 10, 10});
    holes3 = MakePath({10, 10, 30, 10, 20, 20, 10, 20, 10, 10});
    std::cout << holes3 << std::endl;
    std::reverse(holes1.begin(), holes1.end());
    std::cout << holes1 << std::endl;
    holes2 = MakePath({10, 30, 20, 30, 20, 40, 10, 40, 50, 30});
    std::reverse(holes2.begin(), holes2.end());
    std::cout << holes2 << std::endl;
    // holes3 = MakePath({10, 50, 20, 50, 20, 60, 10, 60, 20, 50});
    // holes3.reserve(5);

    pA.AddChild(polygonA);
    pA.AddChild(holes1);
    pA.AddChild(holes2);

    Paths64 minkowskiSum = MinkowskiSum(polygonA, polygonB, true);
    Paths64 minkowskiSum2 = MinkowskiSum(polygonB, polygonA, true);

    std::cout << "Minkowski Sum of Polygon A and Polygon B:" << std::endl;

    DisplayPolygons(minkowskiSum);

    Paths64 p1, p2;
    p1.push_back(polygonA);
    p2.push_back(polygonB);

    SvgWriter svg2;
    Paths64 temp1, temp2;
    for (size_t i = 0; i < pA.Count(); i++)
    {
        if (pA[i]->IsHole())
        {
            temp1.push_back(pA[i]->Polygon());
        }
        else
        {
            temp2.push_back(pA[i]->Polygon());
        }
    }
    SvgAddSubject(svg2, temp1, FillRule::Negative);
    SvgAddSubject(svg2, temp2, FillRule::Positive);
    // SvgAddSubject(svg2, p1, FillRule::NonZero);
    // SvgAddSubject(svg2, p2, FillRule::NonZero);
    SvgAddSolution(svg2, minkowskiSum, FillRule::NonZero, false);
    SvgAddSolution(svg2, minkowskiSum2, FillRule::NonZero, false);
    // SvgAddCaption(svg2, "Minkowski", 0, 0);
    SvgSaveToFile(svg2, "test_min.svg", 800, 600, 20);
    System("test_min.svg");
}

// Function to display polygons
void DisplayPolygons(const Paths64 &polygons)
{
    for (const auto &polygon : polygons)
    {
        std::cout << "(" << polygon << ") ";
    }
}

void System(const std::string &filename)
{
#ifdef _WIN32
    system(filename.c_str());
#else
    system(("firefox " + filename).c_str());
#endif
}