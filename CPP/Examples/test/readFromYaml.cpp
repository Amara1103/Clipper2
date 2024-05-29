#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <unordered_map>
#include "yaml-cpp/yaml.h"
#include "clipper2/clipper.h"
#include "../../Utils/clipper.svg.utils.h"
#include "../../Utils/ClipFileLoad.h"
#include "../../Utils/ClipFileSave.h"
#include <filesystem>
#include <string>
#include <thread>
using namespace std::filesystem;
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
    PathD outsideLoop;
    PathsD insideLoops;
};

struct Sheet
{
    std::string name;
    double width;
    double height;
    std::vector<Part> parts;
};

void parseYAML(const std::string &filename, Sheet &sheet)
{
    YAML::Node config = YAML::LoadFile(filename);

    size_t lastSlash = filename.find_last_of("/");

    // 从最后一个斜杠位置开始找第一个点的位置
    size_t lastDot = filename.find_last_of(".");

    // 提取文件名（不包含路径和扩展名）
    std::string sheet_name = filename.substr(lastSlash + 1, lastDot - lastSlash - 1);

    sheet.name = sheet_name;

    // Parse sheet data
    sheet.width = config["sheet"]["width"].as<double>();
    sheet.height = config["sheet"]["height"].as<double>();

    double scale = 512.0 * 512.0 / sheet.width / sheet.height;
    sheet.width *= scale;
    sheet.height *= scale;
    // double scale = 1.0;

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
            outsideLoop.push_back(PointD(pointNode["x"].as<double>() * scale, pointNode["y"].as<double>() * scale));
        }
        if (!outsideLoop.empty())
        {
            outsideLoop.push_back(outsideLoop.front());
        }

        if (Area(outsideLoop) < 0)
        {
            std::reverse(outsideLoop.begin(), outsideLoop.end());
        }
        // SimplifyPath(outsideLoop, 1.0, true);
        part.outsideLoop = outsideLoop;

        RectD rect = GetBounds(part.outsideLoop);
        double dx, dy;
        dx = rect.left;
        dy = rect.bottom;

        PathsD insideLoop;
        if (partNode["insideLoops"])
        {
            for (const auto &insideLoopNode : partNode["insideLoops"])
            {
                PathD Loop;
                for (const auto &pointNode : insideLoopNode)
                {
                    Loop.push_back(PointD(pointNode["x"].as<double>() * scale, pointNode["y"].as<double>() * scale));
                }
                if (!Loop.empty())
                {
                    Loop.push_back(Loop.front());
                }
                if (Area(Loop) > 0)
                {
                    std::reverse(Loop.begin(), Loop.end());
                }
                // SimplifyPath(Loop, 1.0, true);
                if (std::abs(Area(Loop)) > 200)
                {
                    insideLoop.push_back(Loop);
                }
            }
            part.insideLoops = insideLoop;
        }
        else
        {
            part.insideLoops.clear();
        }
        // 将所有的坐标转换为相对于原点的坐标
        dx = 0 - dx;
        dy = 0 - dy;
        // std::cout << "dx dy: " << dx << " " << dy << std::endl;
        part.outsideLoop = TranslatePath<double>(part.outsideLoop, dx, dy);
        if (part.insideLoops.size() > 0)
        {
            part.insideLoops = TranslatePaths<double>(part.insideLoops, dx, dy);
        }

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

//
std::pair<double, double> findClosestCoordinate(PathsD &coords, double targetX, double targetY)
{
    // 目标坐标
    double minDistanceSquared = std::numeric_limits<double>::max();
    std::pair<double, double> closestCoord;
    double minOriginDistanceSquared = std::numeric_limits<double>::max();

    // 遍历坐标列表，两个元素为一组
    for (const auto &path : coords)
    {
        for (const auto &point : path)
        {
            double dx = point.x - targetX;
            double dy = point.y - targetY;
            // 计算和目标坐标的距离平方
            double distanceToTargetSquared = dx * dx + dy * dy;
            double distanceToOriginSquared = point.x * point.x + point.y * point.y;

            if (distanceToTargetSquared < minDistanceSquared ||
                (distanceToTargetSquared == minDistanceSquared && distanceToOriginSquared < minOriginDistanceSquared))
            {
                minDistanceSquared = distanceToTargetSquared;
                minOriginDistanceSquared = distanceToOriginSquared;
                closestCoord = {point.x, point.y};
            }
        }
    }

    return closestCoord;
}
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

PathD RotatePath(const PathD &path, double angle)
{
    PathD rotatedPath;
    double rad = angle * M_PI / 180.0; // 将角度转换为弧度
    double cosTheta = cos(rad);
    double sinTheta = sin(rad);

    for (const PointD &pt : path)
    {
        double x = pt.x;
        double y = pt.y;
        double newX = static_cast<double>(cosTheta * x - sinTheta * y);
        double newY = static_cast<double>(sinTheta * x + cosTheta * y);
        rotatedPath.push_back(PointD(newX, newY));
    }

    return rotatedPath;
}

PathsD RotatePaths(const PathsD &paths, double angle)
{
    PathsD rotatedPaths;
    for (const PathD &path : paths)
    {
        rotatedPaths.push_back(RotatePath(path, angle));
    }
    return rotatedPaths;
}

void Nesting(std::string filename)
{
    // 0.解析YAML，读取多边形数据。规定外边界均为逆时针，内边界顺时针
    Sheet sheet;
    int Rotations = 4;
    parseYAML(filename, sheet);

    // Access parsed data
    std::cout << "Sheet Width: " << sheet.width << std::endl;
    std::cout << "Sheet Height: " << sheet.height << std::endl;

    PathD surface = MakePathD({0, 0, int(sheet.width), 0, int(sheet.width), int(-sheet.height), 0, int(-sheet.height), 0, 0});
    std::cout << "Surface: " << surface << std::endl;

    // 1.按照面积大小进行排序
    std::sort(sheet.parts.begin(), sheet.parts.end(), compareByArea);
    std::cout << "Rank end!" << std::endl;

    // 标记未放置的零件
    std::unordered_map<std::string, int> notNestedParts;
    for (auto it = sheet.parts.begin(); it != sheet.parts.end(); ++it)
    {
        auto &part = *it;
        notNestedParts[part.id] = part.quantity;
    }
    using partInfo = std::tuple<Part, int, Coordinate>;

    // 创建一个vector来存储多个PartInfo
    std::vector<partInfo> partsCoordinate;
    // 2.计算可放置区域

    for (auto it1 = sheet.parts.begin(); it1 != sheet.parts.end(); ++it1)
    {
        auto &orbitingPart = *it1;
        int rotation;
        // TODO 加旋转
        // for (int i = 0; i < Rotations; ++i)
        // {
        //     rotation = i * 360.0 / Rotations;

        // }

        // 计算orbitingPart和surface的ifp
        for (int i = 0; i < orbitingPart.quantity; ++i)
        {
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
            ifp[1] = TranslatePath(ifp[1], 0 - ifpRect.left, 0 - ifpRect.bottom);
            PathsD temp;
            temp.push_back(ifp[1]);

            RectD orbitingPartRect = GetBounds(orbitingPart.outsideLoop);

            PathsD nfps;
            PathsD ifps;
            ifps = Difference(temp, ifps, FillRule::NonZero);

            // 计算orbit和station的nfp以及与孔洞的ifp
            SvgWriter svg1;
            PathsD temp1;
            // temp1.push_back(surface);
            for (auto it2 = partsCoordinate.begin(); it2 != partsCoordinate.end(); ++it2)
            {

                Part stationPart;
                PathD stationPartOutsideLoop;
                PathsD stationPartInsideLoops;

                for (int i = 0; i < std::get<1>(*it2); ++i)
                {
                    stationPart = std::get<0>(*it2);
                    if (Area(stationPart.insideLoops) < 0)
                    {
                        std::reverse(stationPart.insideLoops.begin(), stationPart.insideLoops.end());
                    }

                    stationPartOutsideLoop = TranslatePath(stationPart.outsideLoop, std::get<2>(*it2).point2_X, std::get<2>(*it2).point2_Y);
                    if (!stationPart.insideLoops.empty())
                    {
                        stationPartInsideLoops = TranslatePaths(stationPart.insideLoops, std::get<2>(*it2).point2_X, std::get<2>(*it2).point2_Y);
                    }
                    else
                    {
                        stationPartInsideLoops.clear();
                    }
                    PathD orbitingLoop = MirrorPolygon(orbitingPart.outsideLoop, 0.0, 0.0);

                    PathsD nfp = MinkowskiSum(stationPartOutsideLoop, orbitingLoop, true);

                    PathsD nfpOutsideLoop;
                    nfpOutsideLoop.push_back(nfp[0]);

                    nfps = Union(nfps, nfpOutsideLoop, FillRule::NonZero);
                    PathsD ifp;
                    PathsD ifp_trans;
                    for (const auto &stationInsideLoop : stationPartInsideLoops)
                    {
                        if (std::abs(Area(stationInsideLoop)) > std::abs(Area(orbitingPart.outsideLoop)))
                        {
                            ifp = MinkowskiSum(stationInsideLoop, orbitingLoop, true);
                            // ifp_trans = TranslatePaths(ifp, -orbitingPartRect.left, -orbitingPartRect.bottom);

                            if (ifp.size() > 1)
                            {
                                // ifpOutsideLoop.push_back(ifp[1]);
                                ifp_trans.push_back(ifp[1]);

                                nfps = Difference(nfps, ifp_trans, FillRule::NonZero);
                            }
                        }
                    }
                }
            }
            if (Area(ifps) < 0)
            {
                std::reverse(ifps.begin(), ifps.end());
            }
            PathsD placeableArea = Difference(ifps, nfps, FillRule::NonZero);
            std::cout << orbitingPart.id << std::endl;
            // std::cout << placeableArea << std::endl;

            std::pair<double, double> best = findClosestCoordinate(placeableArea, 0.0, 0.0);

            Coordinate placedPoint;
            placedPoint.point2_X = best.first;
            placedPoint.point2_Y = best.second;

            partsCoordinate.emplace_back(orbitingPart, orbitingPart.quantity, placedPoint);
        }
    }

    // 3.摆放部件
    SvgWriter svg2;
    PathsD temp;
    for (auto it2 = partsCoordinate.begin(); it2 != partsCoordinate.end(); ++it2)
    {
        Part stationPart;
        PathD stationPartOutsideLoop;
        PathsD stationPartInsideLoops;
        double dx, dy;

        for (int i = 0; i < std::get<1>(*it2); ++i)
        {
            stationPart = std::get<0>(*it2);
            if (Area(stationPart.insideLoops) < 0)
            {
                std::reverse(stationPart.insideLoops.begin(), stationPart.insideLoops.end());
            }

            stationPartOutsideLoop = TranslatePath(stationPart.outsideLoop, std::get<2>(*it2).point2_X, std::get<2>(*it2).point2_Y);
            if (!stationPart.insideLoops.empty())
            {
                stationPartInsideLoops = TranslatePaths(stationPart.insideLoops, std::get<2>(*it2).point2_X, std::get<2>(*it2).point2_Y);
            }

            temp = stationPartInsideLoops;
            temp.push_back(stationPartOutsideLoop);
            SvgAddSubject(svg2, temp, FillRule::NonZero);
        }
    }
    temp.push_back(surface);
    SvgAddSubject(svg2, temp, FillRule::NonZero);
    SvgSaveToFile(svg2, sheet.name + ".svg", sheet.width, sheet.height, 0);
}

int main()
{
    std::string directory = "/Users/gzz/Clipper2/CPP/Examples/test/random";
    std::vector<std::string> files;
    for (const auto &entry : std::filesystem::directory_iterator(directory))
    {
        if (entry.is_regular_file() && entry.path().extension() == ".yaml")
        {
            files.push_back(entry.path().string());
        }
    }
    std::vector<std::thread> threads;
    for (std::string filename : files)
    {
        threads.push_back(std::thread(Nesting, filename));
    }
    // 等待所有线程完成
    for (std::thread &t : threads)
    {
        t.join();
    }

    return 0;
}
