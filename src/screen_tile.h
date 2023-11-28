#pragma once

#include "defs.h"
#include "rect.h"

struct IndexBounds {
    u32 min_row;
    u32 max_row;
    u32 min_col;
    u32 max_col;
};

struct ScreenTileData {
    Rect2D tile_for_index(SDL_Surface* surface, u32 index) const;
    u32 num_tasks() const;
    IndexBounds index_bounds_for_bb(Rect2D bounding_box) const;

    u32 rows;
    u32 cols;
    u32 tile_width;
    u32 tile_height;
};

ScreenTileData partition_screen_into_tiles(SDL_Surface* surface);