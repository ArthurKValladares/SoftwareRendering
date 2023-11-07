#ifndef point_h
#define point_h

struct Point2D {
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

#endif
