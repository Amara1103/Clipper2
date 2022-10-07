/*******************************************************************************
* Author    :  Angus Johnson                                                   *
* Version   :  Clipper2 - ver.1.0.5                                            *
* Date      :  6 October 2022                                                  *
* Website   :  http://www.angusj.com                                           *
* Copyright :  Angus Johnson 2010-2022                                         *
* Purpose   :  Simple FAST rectangular clipping                                *
* License   :  http://www.boost.org/LICENSE_1_0.txt                            *
*******************************************************************************/

#ifndef CLIPPER_RECTCLIP_H
#define CLIPPER_RECTCLIP_H

#include <cstdlib>
#include <vector>
#include <string>
#include "clipper.h"
#include "clipper.core.h"

namespace Clipper2Lib 
{


  namespace detail
  {
    enum class Location { Left, Top, Right, Bottom, Inside };

    inline bool GetLocation(const Rect64& rec,
      const Point64& pt, Location& loc)
    {
      if (pt.x == rec.left && pt.y >= rec.top && pt.y <= rec.bottom)
      {
        loc = Location::Left;
        return false;
      }
      else if (pt.x == rec.right && pt.y >= rec.top && pt.y <= rec.bottom)
      {
        loc = Location::Right;
        return false;
      }
      else if (pt.y == rec.top && pt.x >= rec.left && pt.x <= rec.right)
      {
        loc = Location::Top;
        return false;
      }
      else if (pt.y == rec.bottom && pt.x >= rec.left && pt.x <= rec.right)
      {
        loc = Location::Bottom;
        return false;
      }
      else if (pt.x < rec.left) loc = Location::Left;
      else if (pt.x > rec.right) loc = Location::Right;
      else if (pt.y < rec.top) loc = Location::Top;
      else if (pt.y > rec.bottom) loc = Location::Bottom;
      else loc = Location::Inside;
      return true;
    }

    Point64 GetIntersectPoint64(const Point64& ln1a, const Point64& ln1b, 
      const Point64& ln2a, const Point64& ln2b)
    {
      // see http://astronomy.swin.edu.au/~pbourke/geometry/lineline2d/
      if (ln1b.x == ln1a.x) 
      {
        if (ln2b.x == ln2a.x) return Point64(); // parallel lines
        double m2 = static_cast<double>(ln2b.y - ln2a.y) / (ln2b.x - ln2a.x);
        double b2 = ln2a.y - m2 * ln2a.x;
        return Point64(ln1a.x, static_cast<int64_t>(std::round(m2 * ln1a.x + b2)));
      }
      else if (ln2b.x == ln2a.x)
      {
        double m1  = static_cast<double>(ln1b.y - ln1a.y) / (ln1b.x - ln1a.x);
        double b1 = ln1a.y - m1 * ln1a.x;
        return Point64(ln2a.x, static_cast<int64_t>(std::round(m1 * ln2a.x + b1)));
      }
      else
      {
        double m1 = static_cast<double>(ln1b.y - ln1a.y) / (ln1b.x - ln1a.x);
        double b1 = ln1a.y - m1 * ln1a.x;
        double m2 = static_cast<double>(ln2b.y - ln2a.y) / (ln2b.x - ln2a.x);
        double b2 = ln2a.y - m2 * ln2a.x;
        if (std::fabs(m1 - m2) > 1.0E-15)
        {
          double x = (b2 - b1) / (m1 - m2);
          return Point64(x, m1 * x + b1);
        }
        else
          return Point64((ln1a.x + ln1b.x) * 0.5, (ln1a.y + ln1b.y) * 0.5);
      }
    }

    inline bool GetIntersection(const Path64& rectPath,
      const Point64& p, const Point64& p2, Location& loc, Point64& ip)
    {
      // gets the intersection closest to 'p'
      // when Result = false, loc will remain unchanged
      switch (loc)
      {
      case Location::Left:
        if (SegmentsIntersect(p, p2, rectPath[0], rectPath[3], true))
          ip = GetIntersectPoint64(p, p2, rectPath[0], rectPath[3]);
        else if (p.y < rectPath[0].y &&
          SegmentsIntersect(p, p2, rectPath[0], rectPath[1], true))
        {
          ip = GetIntersectPoint64(p, p2, rectPath[0], rectPath[1]);
          loc = Location::Top;
        }
        else if (SegmentsIntersect(p, p2, rectPath[2], rectPath[3], true))
        {
          ip = GetIntersectPoint64(p, p2, rectPath[2], rectPath[3]);
          loc = Location::Bottom;
        }
        else return false;
        break;

      case Location::Top:
        if (SegmentsIntersect(p, p2, rectPath[0], rectPath[1], true))
          ip = GetIntersectPoint64(p, p2, rectPath[0], rectPath[1]);
        else if (p.x < rectPath[0].x &&
          SegmentsIntersect(p, p2, rectPath[0], rectPath[3], true))
        {
          ip = GetIntersectPoint64(p, p2, rectPath[0], rectPath[3]);
          loc = Location::Left;
        }
        else if (p.x > rectPath[1].x &&
          SegmentsIntersect(p, p2, rectPath[1], rectPath[2], true))
        {
          ip = GetIntersectPoint64(p, p2, rectPath[1], rectPath[2]);
          loc = Location::Right;
        }
        else return false;
          break;

      case Location::Right:
        if (SegmentsIntersect(p, p2, rectPath[1], rectPath[2], true))
          ip = GetIntersectPoint64(p, p2, rectPath[1], rectPath[2]);
        else if (p.y < rectPath[0].y &&
          SegmentsIntersect(p, p2, rectPath[0], rectPath[1], true))
        {
          ip = GetIntersectPoint64(p, p2, rectPath[0], rectPath[1]);
          loc = Location::Top;
        }
        else if (SegmentsIntersect(p, p2, rectPath[2], rectPath[3], true))
        {
          ip = GetIntersectPoint64(p, p2, rectPath[2], rectPath[3]);
          loc = Location::Bottom;
        }
        else return false;
        break;

      case Location::Bottom:
        if (SegmentsIntersect(p, p2, rectPath[2], rectPath[3], true))
          ip = GetIntersectPoint64(p, p2, rectPath[2], rectPath[3]);
        else if (p.x < rectPath[3].x &&
          SegmentsIntersect(p, p2, rectPath[0], rectPath[3], true))
        {
          ip = GetIntersectPoint64(p, p2, rectPath[0], rectPath[3]);
          loc = Location::Left;
        }
        else if (p.x > rectPath[2].x &&
          SegmentsIntersect(p, p2, rectPath[1], rectPath[2], true))
        {
          ip = GetIntersectPoint64(p, p2, rectPath[1], rectPath[2]);
          loc = Location::Right;
        }
        else return false;
        break;

      default: // loc == rInside
        if (SegmentsIntersect(p, p2, rectPath[0], rectPath[3], true))
        {
          ip = GetIntersectPoint64(p, p2, rectPath[0], rectPath[3]);
          loc = Location::Left;
        }
        else if (SegmentsIntersect(p, p2, rectPath[0], rectPath[1], true))
        {
          ip = GetIntersectPoint64(p, p2, rectPath[0], rectPath[1]);
          loc = Location::Top;
        }
        else if (SegmentsIntersect(p, p2, rectPath[1], rectPath[2], true))
        {
          ip = GetIntersectPoint64(p, p2, rectPath[1], rectPath[2]);
          loc = Location::Right;
        }
        else if (SegmentsIntersect(p, p2, rectPath[2], rectPath[3], true))
        {
          ip = GetIntersectPoint64(p, p2, rectPath[2], rectPath[3]);
          loc = Location::Bottom;
        }
        else return false;
        break;
      }

      return true;
    }

    class RectClip {
    private:
      const Rect64 rect_;
      const Path64 rectPath_;
      Location first_cross_;
      Path64 result_;

      inline void Reset() { result_.clear(); }

      inline void AddCorner(Location prev, Location curr)
      {
        unsigned i = static_cast<unsigned>(prev);
        unsigned j = static_cast<unsigned>(curr);
        if (i > 3 || j > 3 || i == j) return;
        if ((i + 1) % 4 == j)
          result_.push_back(rectPath_[i]);
        else
          result_.push_back(rectPath_[j]);
      }

      void CheckCorners(Location prev, Location curr, const Point64& pt)
      {
        if (prev == Location::Inside) return;
        unsigned i = static_cast<unsigned>(prev);
        unsigned j = static_cast<unsigned>(curr);
        if (first_cross_ == Location::Inside) first_cross_ = prev;

        if (j != (i + 2) % 4)           
          AddCorner(prev, curr);
        else
        {
          switch (prev) 
          {
          case Location::Left:
            if (pt.y < rect_.top)
            {
              result_.push_back(rectPath_[0]);
              result_.push_back(rectPath_[1]);
            }
            else
            {
              result_.push_back(rectPath_[3]);
              result_.push_back(rectPath_[2]);
            }
            break;

          case Location::Top:
            if (pt.x < rect_.left)
            {
              result_.push_back(rectPath_[0]);
              result_.push_back(rectPath_[3]);
            }
            else
            {
              result_.push_back(rectPath_[1]);
              result_.push_back(rectPath_[2]);
            }
            break;

          case Location::Right:
            if (pt.y < rect_.top)
            {
              result_.push_back(rectPath_[1]);
              result_.push_back(rectPath_[0]);
            }
            else
            {
              result_.push_back(rectPath_[2]);
              result_.push_back(rectPath_[3]);
            }
            break;

          case Location::Bottom:
            if (pt.x < rect_.left)
            {
              result_.push_back(rectPath_[3]);
              result_.push_back(rectPath_[0]);
            }
            else
            {
              result_.push_back(rectPath_[2]);
              result_.push_back(rectPath_[1]);
            }
            break;
          }
        }
      }

    public:
      
      RectClip(const Rect64& rect) : 
        rect_(rect), 
        rectPath_(rect.AsPath()), 
        first_cross_(Location::Inside) {}

      Path64 Execute(const Path64& path) 
      {
        if (rect_.IsEmpty() || path.size() < 3) return Path64();
        Reset();

        size_t i = 0, highI = path.size() - 1;
        Location prev, loc;
        Location crossing_loc = Location::Inside;
        first_cross_ = Location::Inside;
        detail::GetLocation(rect_, path[highI], loc);
        while (i <= highI)
        {
          prev = loc;
          Location crossing_prev = crossing_loc;


          switch (loc)
          {
          case Location::Left:
            while (i <= highI && path[i].x <= rect_.left) ++i;
            if (i > highI) break;
            else if (path[i].y <= rect_.top) loc = Location::Top;
            else if (path[i].y >= rect_.bottom) loc = Location::Bottom;
            else if (path[i].x < rect_.right) loc = Location::Inside;
            else loc = Location::Right;
            break;

          case Location::Top:
            while (i <= highI && path[i].y <= rect_.top) ++i;
            if (i > highI) break;
            else if (path[i].x <= rect_.left) loc = Location::Left;
            else if (path[i].x >= rect_.right) loc = Location::Right;
            else if (path[i].y < rect_.bottom) loc = Location::Inside;
            else loc = Location::Bottom;
            break;

          case Location::Right:
            while (i <= highI && path[i].x >= rect_.right) ++i;
            if (i > highI) break;
            else if (path[i].y <= rect_.top) loc = Location::Top;
            else if (path[i].y >= rect_.bottom) loc = Location::Bottom;
            else if (path[i].x > rect_.left) loc = Location::Inside;
            else loc = Location::Left;
            break;

          case Location::Bottom:
            while (i <= highI && path[i].y >= rect_.bottom) ++i;
            if (i > highI) break;
            else if (path[i].x <= rect_.left) loc = Location::Left;
            else if (path[i].x >= rect_.right) loc = Location::Right;
            else if (path[i].y > rect_.top) loc = Location::Inside;
            else loc = Location::Top;
            break;

          case Location::Inside:
            while (i <= highI)
            {
              if (path[i].x < rect_.left) loc = Location::Left;
              else if (path[i].x > rect_.right) loc = Location::Right;
              else if (path[i].y > rect_.bottom) loc = Location::Bottom;
              else if (path[i].y < rect_.top) loc = Location::Top;
              else { result_.push_back(path[i]); ++i; continue; }
              break; //inner loop
            }
            break;
          } //switch          
          if (i > highI) break;

          Point64 ip, ip2;
          Point64 prev_pt = (i) ? path[i - 1] : path[highI];

          crossing_loc = loc;
          if (!GetIntersection(rectPath_, path[i], prev_pt, crossing_loc, ip))
          {
            // ie remaining outside
            CheckCorners(prev, crossing_loc, prev_pt);
            ++i;
            continue;
          }

          // we must be crossing the rect boundary to get here

          if (loc == Location::Inside)
          {
            // path must be entering rect
            if (first_cross_ == Location::Inside)
              first_cross_ = crossing_loc;
            else
              AddCorner(prev, crossing_loc);
          }
          else if (prev != Location::Inside)
          {
            // passing right through but ip will be the second intersect pt
            // so get the first intersect pt

            loc = prev;
            GetIntersection(rectPath_, prev_pt, path[i], loc, ip2);
            AddCorner(crossing_prev, loc);

            if (first_cross_ == Location::Inside) 
              first_cross_  = loc;
            loc = crossing_loc;
            result_.push_back(ip2);
            if (ip == ip2)
            {
              // it's very likely that path[i] is on rect
              GetLocation(rect_, path[i], loc);
              AddCorner(crossing_loc, loc);
              crossing_loc = loc;
              continue;
            }
          }
          else
          {
            loc = crossing_loc;
            if (first_cross_ == Location::Inside) 
              first_cross_ = crossing_loc;
          }
          result_.push_back(ip);

        } //while i <= highI

        if (first_cross_ == Location::Inside)
        {
          Rect64 tmp_rect = Bounds(path);
          if (tmp_rect.Contains(rect_)) return rectPath_;
          else if (rect_.Contains(tmp_rect)) return path;
          else return Path64();
        }

        if (loc != Location::Inside)
          CheckCorners(crossing_loc, first_cross_, path[highI]);

        if (result_.size() < 3) return Path64();

        // tidy up duplicates and collinear segments
        Path64 res;
        res.reserve(result_.size());
        size_t k = 0; highI = result_.size() - 1;
        Point64 prev_pt = result_[highI];
        res.push_back(result_[0]);
        for (Path64::const_iterator cit = result_.cbegin() + 1; cit != result_.cend(); ++cit)
        {
          if (CrossProduct(prev_pt, res[k], *cit))
          {
            prev_pt = res[k++];
            res.push_back(*cit);
          }
          else 
            res[k] = *cit;
        }

        if (k < 2) return Path64();
        // and a final check for collinearity
        else if (!CrossProduct(res[0], res[k - 1], res[k])) res.pop_back();
        return res;
      }

    }; //RectClip class

  } // namespace detail

  inline Path64 RectClip64(const Rect64& rect, const Path64& path) 
  {
    detail::RectClip rc(rect);
    return rc.Execute(path);
  }

  inline Paths64 RectClip64(const Rect64& rect, const Paths64& paths)
  {
    if (rect.IsEmpty() || paths.empty()) return Paths64();
    Paths64 result;
    result.reserve(paths.size());
    for (const Path64& path : paths)
    {
      if (!rect.Intersects(Bounds(path))) continue;
      result.push_back(RectClip64(rect, path));      
    }
    return result;    
  }

} // Clipper2Lib namespace

#endif  // CLIPPER_RECTCLIP_H