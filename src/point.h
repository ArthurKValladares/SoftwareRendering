#ifndef point_h
#define point_h

#include "defs.h"

struct Point2D {
    Point2D() {}
    Point2D(int x, int y)
        : x(x)
        , y(y) {}
    
    Point2D clamp(Point2D min, Point2D max) const;

    Point2D operator+(const Point2D& rhs) const;
    Point2D operator-(const Point2D& rhs) const;
    
    int x;
    int y;
};
Point2D rotate_point(Point2D point, Point2D pivot, float angle);

struct Point3D {
    int x;
    int y;
    int z;
};

// TODO: I will probably need to make some of the point/rect/line/etc functions generic in some way later
struct Point2D_f {
    float x;
    float y;

    Point2D round() const;
};

struct Point3D_f {
    float x;
    float y;
    float z;
    
    Point3D_f() {}
    Point3D_f(float x, float y, float z) :
        x(x),
        y(y),
        z(z)
    {}
    Point3D round() const;

    bool operator==(const Point3D_f& other) const {
        return x == other.x && y == other.y && z == other.z;
    }
};

namespace std {
    template<> struct hash<Point3D_f> {
        size_t operator()(Point3D_f const& s) const {
            std::size_t res = 0;
            hash_combine(res, s.x);
            hash_combine(res, s.y);
            hash_combine(res, s.z);
            return res;
        }
    };
}

#endif
