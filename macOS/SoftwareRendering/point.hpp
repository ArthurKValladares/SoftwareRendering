#ifndef point_h
#define point_h

struct Point2D {
    Point2D(int x, int y)
        : x(x)
        , y(y) {}
    
    int x;
    int y;
};

Point2D point_sub(Point2D a, Point2D b);
Point2D rotate_point(Point2D point, Point2D pivot, float angle);

#endif
