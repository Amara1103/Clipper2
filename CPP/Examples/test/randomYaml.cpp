#include <iostream>
#include <vector>
#include <string>
#include <random>
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <ctime>
#include <fstream>
#include <limits>

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <random>

struct Point
{
    double x;
    double y;
};

// 生成随机多边形的顶点
std::vector<Point> generate_random_polygon(int n, double radius)
{
    std::vector<Point> points;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(0, 2 * M_PI); // 均匀分布的角度

    for (int i = 0; i < n; ++i)
    {
        double angle = dis(gen);
        double x = std::cos(angle) * radius;
        double y = std::sin(angle) * radius;
        points.push_back({x, y});
    }

    // 根据极角对顶点进行排序
    std::sort(points.begin(), points.end(), [](const Point &a, const Point &b)
              { return std::atan2(a.y, a.x) < std::atan2(b.y, b.x); });

    return points;
}

// 生成随机字符串
std::string generateRandomString(size_t length)
{
    const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    const size_t max_index = sizeof(charset) - 1;
    std::string random_string;
    random_string.reserve(length);
    for (size_t i = 0; i < length; ++i)
    {
        random_string += charset[rand() % max_index];
    }
    return random_string;
}

int randomInt(int min, int max)
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

// 生成随机 UUID
std::string generateUUID()
{
    std::stringstream ss;
    ss << std::hex;
    for (int i = 0; i < 8; i++)
        ss << (rand() % 16);
    ss << "-";
    for (int i = 0; i < 4; i++)
        ss << (rand() % 16);
    ss << "-4"; // 4 signifies the UUID version
    for (int i = 0; i < 3; i++)
        ss << (rand() % 16);
    ss << "-";
    ss << (rand() % 4 + 8); // the first hex digit can only be 8, 9, a, or b
    for (int i = 0; i < 3; i++)
        ss << (rand() % 16);
    ss << "-";
    for (int i = 0; i < 12; i++)
        ss << (rand() % 16);
    return ss.str();
}

// 检查点是否在多边形内
bool pointInPolygon(const std::vector<Point> &polygon, const Point &point)
{
    bool inside = false;
    size_t n = polygon.size();
    for (size_t i = 0, j = n - 1; i < n; j = i++)
    {
        if (((polygon[i].y > point.y) != (polygon[j].y > point.y)) &&
            (point.x < (polygon[j].x - polygon[i].x) * (point.y - polygon[i].y) / (polygon[j].y - polygon[i].y) + polygon[i].x))
        {
            inside = !inside;
        }
    }
    return inside;
}

std::string formatDouble(double value)
{
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(5) << value;
    return oss.str();
}

struct Part
{
    std::string id;
    int quantity;
    std::vector<Point> outsideLoop;
    std::vector<std::vector<Point>> insideLoops;
};

double generateRandomDouble(double min, double max)
{
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<> dis(min, max);
    return dis(gen);
}

void generateRandomPoints(std::vector<Point> &points, int count, double minX, double maxX, double minY, double maxY)
{
    for (int i = 0; i < count; ++i)
    {
        points.push_back({generateRandomDouble(minX, maxX), generateRandomDouble(minY, maxY)});
    }
}

// 生成n个随机点
std::vector<Point> generateRandomPoints(int n, double width, double height)
{
    std::vector<Point> points;
    for (int i = 0; i < n; ++i)
    {
        points.push_back({(double)rand() / RAND_MAX * width, (double)rand() / RAND_MAX * height});
    }
    return points;
}
// 计算两个点的叉积
double cross(const Point &O, const Point &A, const Point &B)
{
    return (A.x - O.x) * (B.y - O.y) - (A.y - O.y) * (B.x - O.x);
}

std::vector<Point> convexHull(std::vector<Point> points)
{
    std::vector<Point> hull;

    // Sort points lexicographically
    std::sort(points.begin(), points.end(), [](Point a, Point b)
              { return a.x < b.x || (a.x == b.x && a.y < b.y); });

    // Build the lower hull
    for (const auto &point : points)
    {
        while (hull.size() >= 2 && cross(hull[hull.size() - 2], hull.back(), point) <= 0)
        {
            hull.pop_back();
        }
        hull.push_back(point);
    }

    // Build the upper hull
    size_t lower_hull_size = hull.size();
    for (int i = points.size() - 2; i >= 0; --i)
    {
        while (hull.size() > lower_hull_size && cross(hull[hull.size() - 2], hull.back(), points[i]) <= 0)
        {
            hull.pop_back();
        }
        hull.push_back(points[i]);
    }

    // hull.pop_back(); // Remove the last point because it's the same as the first one
    return hull;
}

// 生成多边形内的随机点
std::vector<Point> generatePointsInsidePolygon(const std::vector<Point> &poly, int numPoints)
{
    std::vector<Point> points;
    double minX = poly[0].x, maxX = poly[0].x;
    double minY = poly[0].y, maxY = poly[0].y;

    for (const auto &p : poly)
    {
        if (p.x < minX)
            minX = p.x;
        if (p.x > maxX)
            maxX = p.x;
        if (p.y < minY)
            minY = p.y;
        if (p.y > maxY)
            maxY = p.y;
    }

    while (points.size() < numPoints)
    {
        Point pt = {minX + (double)rand() / RAND_MAX * (maxX - minX), minY + (double)rand() / RAND_MAX * (maxY - minY)};
        if (pointInPolygon(poly, pt))
        {
            points.push_back(pt);
        }
    }

    return points;
}

void generateYAML(const std::string &filename)
{
    std::ofstream file(filename);

    double sheetWidth = generateRandomDouble(1000.0, 2000.0);
    double sheetHeight = generateRandomDouble(800.0, 1500.0);

    file << "sheet:\n";
    file << "  width: " << formatDouble(sheetWidth) << "\n";
    file << "  height: " << formatDouble(sheetHeight) << "\n";

    file << "parts:\n";
    for (int i = 0; i < randomInt(20, 30); i++)
    {
        Part part;
        part.id = generateUUID();
        part.quantity = randomInt(1, 3);

        // generateRandomPoints(part.outsideLoop, randomInt(20, 30), -sheetWidth / 4, sheetWidth / 4, -sheetHeight / 4, sheetHeight / 4);
        std::vector<Point> points = generateRandomPoints(randomInt(10, 30), sheetWidth / 8, sheetHeight / 8);
        part.outsideLoop = convexHull(points);

        file << "- id: " << part.id << "\n";
        file << "  quantity: " << part.quantity << "\n";
        file << "  outsideLoop:\n";

        for (const auto &point : part.outsideLoop)
        {
            file << "  - x: " << formatDouble(point.x) << "\n";
            file << "    y: " << formatDouble(point.y) << "\n";
        }
        int numLoops = randomInt(1, 5);
        part.insideLoops.resize(numLoops);
        for (int i = 0; i < numLoops; ++i)
        {
            std::vector<Point> temp1 = generatePointsInsidePolygon(part.outsideLoop, randomInt(5, 10));
            std::vector<Point> holes = convexHull(temp1);
            // for (const auto &point : temp)
            // {
            // if (pointInPolygon(part.outsideLoop, point))
            // {
            part.insideLoops[i] = holes;
            // }
            // }
        }

        file << "  insideLoops:\n";

        for (const auto &loop : part.insideLoops)
        {
            bool isFirst = true;
            for (const auto &point : loop)
            {
                if (isFirst)
                {
                    file << "  - - x: " << formatDouble(point.x) << "\n";
                    file << "      y: " << formatDouble(point.y) << "\n";
                    isFirst = false;
                }
                else
                {
                    file << "    - x: " << formatDouble(point.x) << "\n";
                    file << "      y: " << formatDouble(point.y) << "\n";
                }
            }
        }
    }

    file.close();
}

int main()
{
    for (int i = 0; i < 5; i++)
    {
        generateYAML("/Users/gzz/Clipper2/CPP/Examples/test/random/" + std::to_string(i) + ".yaml");
        std::cout << "YAML " << std::to_string(i) << "file generated successfully." << std::endl;
    }
    return 0;
}