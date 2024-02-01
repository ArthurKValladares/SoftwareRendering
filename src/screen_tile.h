#pragma once

#include "defs.h"
#include "rect.h"

struct IndexBounds {
    i32 min_row;
    i32 max_row;
    i32 min_col;
    i32 max_col;
};

struct ScreenTileData {
    Rect2D tile_for_index(u32 index) const;
    u32 num_tasks() const;
    IndexBounds index_bounds_for_bb(Rect2D bounding_box) const;

    int total_width;
    int total_height;
    u32 rows;
    u32 cols;
    u32 tile_width;
    u32 tile_height;
};

ScreenTileData partition_screen_into_tiles(SDL_Surface* surface);