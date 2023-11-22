#pragma once

#include "defs.h"

struct UV {
    float u;
    float v;
    
    UV operator*(float s) const;

    bool operator==(const UV& other) const {
        return u == other.u && v == other.v;
    }
};

namespace std {
    template<> struct hash<UV> {
        size_t operator()(UV const& s) const {
            std::size_t res = 0;
            hash_combine(res, s.u);
            hash_combine(res, s.v);
            return res;
        }
    };
}