#pragma once

#include "point.h"
#include "uv.h"

struct Vertex {
    Point3D_f p;
    UV uv;

    bool operator==(const Vertex& other) const {
        return p == other.p && uv == other.uv;
    }
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& s) const {
            std::size_t res = 0;
            hash_combine(res, s.p);
            hash_combine(res, s.uv);
            return res;
        }
    };
}