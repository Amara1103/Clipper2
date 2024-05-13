#include <iostream>
#include <fstream>
#include <unordered_map>
#include "yaml-cpp/yaml.h"
#include "clipper2/clipper.h"
#include "../../Utils/clipper.svg.utils.h"
#include "../../Utils/ClipFileLoad.h"
#include "../../Utils/ClipFileSave.h"

using namespace std;
using namespace Clipper2Lib;

struct Coordinate
{
    int rotation;
    double point2_X;
    double point2_Y;
};
struct Part
{
    std::string id;
    int quantity;
    // PolyPath64 polygon;
    PathD outsideLoop;
    PathsD insideLoops;
    // std::vector<Point> outsideLoop;
    // std::vector<std::vector<Point>> insideLoops;
};

struct Sheet
{
    double width;
    double height;
    std::vector<Part> parts;
};

std::unordered_map<std::string, Coordinate> partsCoordinate;

void parseYAML(const std::string &filename, Sheet &sheet)
{
    YAML::Node config = YAML::LoadFile(filename);

    // Parse sheet data
    sheet.width = config["sheet"]["width"].as<double>();
    sheet.height = config["sheet"]["height"].as<double>();

    // Parse parts data
    for (const auto &partNode : config["parts"])
    {
        Part part;
        part.id = partNode["id"].as<std::string>();
        part.quantity = partNode["quantity"].as<int>();

        // Parse outsideLoop
        PathD outsideLoop;
        for (const auto &pointNode : partNode["outsideLoop"])
        {
            outsideLoop.push_back(PointD(pointNode["x"].as<double>(), pointNode["y"].as<double>()));
        }
        if (!outsideLoop.empty())
        {
            outsideLoop.push_back(outsideLoop.front());
        }

        if (Area(outsideLoop) < 0)
        {
            std::reverse(outsideLoop.begin(), outsideLoop.end());
        }

        part.outsideLoop = outsideLoop;

        RectD rect = GetBounds(part.outsideLoop);
        double dx, dy;
        dx = rect.left;
        dy = rect.bottom;
        // Parse insideLoops
        PathsD insideLoop;
        if (!partNode["insideLoops"].IsNull())
        {
            for (const auto &insideLoopNode : partNode["insideLoops"])
            {
                PathD Loop;
                for (const auto &pointNode : insideLoopNode)
                {
                    Loop.push_back(PointD(pointNode["x"].as<double>(), pointNode["y"].as<double>()));
                }
                if (!Loop.empty())
                {
                    Loop.push_back(Loop.front());
                }
                if (Area(Loop) > 0)
                {
                    std::reverse(Loop.begin(), Loop.end());
                }
                insideLoop.push_back(Loop);
            }
            part.insideLoops = insideLoop;
        }
        // 将所有的坐标转换为相对于原点的坐标
        dx = 0 - dx;
        dy = sheet.height - dy;
        // std::cout << "dx dy: " << dx << " " << dy << std::endl;
        part.outsideLoop = TranslatePath<double>(part.outsideLoop, dx, dy);
        part.insideLoops = TranslatePaths<double>(part.insideLoops, dx, dy);

        // 写入
        sheet.parts.push_back(part); // move ownership of part);
    }
}

bool compareByArea(const Part &orbitingPart, const Part &stationPart)
{
    return Area(orbitingPart.outsideLoop) > Area(stationPart.outsideLoop); // 从大到小排列
}

void splitString(const std::string &inputString, std::string &id, int &quantity)
{
    // 找到下划线的位置
    size_t underscorePos = inputString.find('_');
    if (underscorePos != std::string::npos)
    {
        // 提取id和quantity的子串
        id = inputString.substr(0, underscorePos);
        std::string quantityStr = inputString.substr(underscorePos + 1);

        // 将quantity的子串转换为整数
        quantity = std::stoi(quantityStr);
    }
    else
    {
        // 如果输入格式不正确，抛出异常或者通过其他方式处理错误
        std::cerr << "Invalid input format!" << std::endl;
        // 为了避免未定义的行为，给id和quantity设置默认值
        id = "";
        quantity = 0;
    }
}

int main()
{
    // 0.解析YAML，读取多边形数据。规定外边界均为逆时针，内边界顺时针
    Sheet sheet;
    int Rotations = 4;
    parseYAML("/Users/gzz/Clipper2/CPP/Examples/test/62413.yaml", sheet);

    // Access parsed data
    std::cout << "Sheet Width: " << sheet.width << std::endl;
    std::cout << "Sheet Height: " << sheet.height << std::endl;

    PathD surface = MakePathD({0, 0, int(sheet.width), 0, int(sheet.width), int(sheet.height), 0, int(sheet.height), 0, 0});
    std::cout << "Surface: " << surface << std::endl;

    // 1.按照面积大小进行排序
    std::sort(sheet.parts.begin(), sheet.parts.end(), compareByArea);

    // 标记未放置的零件
    std::unordered_map<std::string, int> notNestedParts;
    for (auto it = sheet.parts.begin(); it != sheet.parts.end(); ++it)
    {
        auto &part = *it;
        notNestedParts[part.id] = part.quantity;
    }

    // 2.计算可放置区域
    SvgWriter svg2;
    for (auto it1 = sheet.parts.begin(); it1 != sheet.parts.end(); ++it1)
    {
        auto &orbitingPart = *it1;
        int rotation;
        // TODO 加旋转
        // for (int i = 0; i < Rotations; ++i)
        // {
        //     rotation = i * 360.0 / Rotations;

        // }
        if (Area(surface) < 0)
        {
            std::reverse(surface.begin(), surface.end());
        }
        PathsD ifp = MinkowskiSum(orbitingPart.outsideLoop, surface, true);
        if (Area(ifp[1]) < 0)
        {
            std::reverse(ifp[1].begin(), ifp[1].end());
        }
        RectD ifpRect = GetBounds(ifp[1]);
        RectD surfaceRect = GetBounds(surface);
        ifp[1] = TranslatePath(ifp[1], 0 - ifpRect.left, sheet.height - ifpRect.bottom);
        PathsD temp;
        temp.push_back(ifp[1]);
        PathsD nfps;
        PathsD ifps;
        ifps = Union(ifps, temp, FillRule::EvenOdd);
        for (auto it2 = partsCoordinate.begin(); it2 != partsCoordinate.end(); ++it2)
        {
            std::string id;
            int quantity;
            splitString(it2->first, id, quantity);
            string targetId = id;
            Part stationPart;
            for (const auto &part : sheet.parts)
            {
                if (part.id == targetId)
                {
                    stationPart = part;
                    break; // 找到了，可以结束循环
                }
            }
            PathsD nfp = MinkowskiSum(stationPart.outsideLoop, orbitingPart.outsideLoop, true);
            nfps = Union(nfps, nfp, FillRule::NonZero);

            if (Area(stationPart.insideLoops) < 0)
            {
                std::reverse(stationPart.insideLoops.begin(), stationPart.insideLoops.end());
            }
            for (const auto &statinInsideLoop : stationPart.insideLoops)
            {
                PathsD ifp = MinkowskiSum(statinInsideLoop, orbitingPart.outsideLoop, true);
                ifps = Union(ifps, ifp, FillRule::NonZero);
            }
        }
        PathsD placeableArea = Union(ifps, nfps, FillRule::NonZero);
        std::cout << orbitingPart.id << std::endl;
        // std::cout << "ifps: " << ifps << std::endl;
        // std::cout << "nfps: " << nfps << std::endl;
        std::cout << "placeableArea: " << placeableArea << std::endl;

        PathsD t1;
        t1.push_back(surface);
        t1.push_back(orbitingPart.outsideLoop);
        // t1.push_back(MakePathD({rect.left, rect.top, rect.right, rect.top, rect.right, rect.bottom, rect.left, rect.bottom, rect.left, rect.top}));
        SvgAddSubject(svg2, t1, FillRule::Negative);
        SvgAddSubject(svg2, nfps, FillRule::Positive);
        SvgAddCaption(svg2, orbitingPart.id + "_" + "nfp", 20, 50);
        SvgAddSubject(svg2, ifps, FillRule::Negative);
        SvgAddCaption(svg2, orbitingPart.id + "_" + "ifp", 20, 10);
        SvgAddSubject(svg2, placeableArea, FillRule::Negative);
        SvgAddCaption(svg2, orbitingPart.id + "_" + "placeableArea", 20, 30);

        SvgSaveToFile(svg2, orbitingPart.id + "_" + ".svg", sheet.width, sheet.height, 0);
    }

    // 3.摆放部件

    // for (auto it1 = sheet.parts.begin(); it1 != sheet.parts.end(); ++it1)
    // {
    //     for (auto it2 = std::next(it1); it2 != sheet.parts.end(); ++it2)
    //     {
    //         const auto &orbitingPart = *it1;
    //         const auto &stationPart = *it2;

    //         double area1 = Area(orbitingPart.outsideLoop);
    //         // std::cout << "Part ID: " << orbitingPart.id << std::endl;
    //         // std::cout << "Quantity: " << orbitingPart.quantity << std::endl;

    //         // std::cout << "Part ID: " << stationPart.id << std::endl;
    //         // std::cout << "Quantity: " << stationPart.quantity << std::endl;

    //         PathsD minkowski = MinkowskiSum(orbitingPart.outsideLoop, stationPart.outsideLoop, true);

    //         PathsD t1;
    //         t1.push_back(orbitingPart.outsideLoop);
    //         t1.push_back(stationPart.outsideLoop);

    //         SvgWriter svg2;
    //         SvgAddSubject(svg2, t1, FillRule::NonZero);
    //         SvgAddSubject(svg2, minkowski, FillRule::NonZero);
    //         SvgSaveToFile(svg2, orbitingPart.id + "+" + stationPart.id + ".svg", sheet.width, sheet.height, 20);
    //     }
    // }

    // for (const auto &orbitingPart : sheet.parts)
    // {

    // std::cout << "Outside Loop:" << std::endl;
    // // for (const auto &point : part.outsideLoop)
    // // {
    // //     std::cout << "(" << point.x << ", " << point.y << ")" << std::endl;
    // // }
    // std::cout << "First Point: (" << part.outsideLoop.front().x << ", " << part.outsideLoop.front().y << ")" << std::endl;
    // std::cout << "Last Point: (" << part.outsideLoop.back().x << ", " << part.outsideLoop.back().y << ")" << std::endl;

    // std::cout << "Inside Loops:" << std::endl;
    // for (const auto &insideLoop : part.insideLoops)
    // {
    //     std::cout << "Inside Loop:" << std::endl;
    //     // for (const auto &point : insideLoop)
    //     // {
    //     //     std::cout << "(" << point.x << ", " << point.y << ")" << std::endl;
    //     // }
    //     std::cout << "First Point: (" << insideLoop.front().x << ", " << insideLoop.front().y << ")" << std::endl;
    //     std::cout << "Last Point: (" << insideLoop.back().x << ", " << insideLoop.back().y << ")" << std::endl;
    // }
    // }

    return 0;
}
