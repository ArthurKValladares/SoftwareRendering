#include "screen_tile.h"

#include <thread>

namespace {
    u32 u_log2(u32 n) {
        u32 l = 0;
        while (n >>= 1) {
            ++l;
        }
        return l;
    }
};

ScreenTileData partition_screen_into_tiles(SDL_Surface* surface) {
    u32 num_tiles = std::thread::hardware_concurrency();
    // Ensure that the total number of tiles is even so that it can easily be factored
    if (num_tiles % 2 == 1) {
        num_tiles *= 2;
    }

    const u32 rows = u_log2(num_tiles);
    const u32 cols = num_tiles / rows;

    const u32 tile_width = ceil(surface->w / cols);
    const u32 tile_height = ceil(surface->h / rows);

    return ScreenTileData{
        surface->w,
        surface->h,
        rows,
        cols,
        tile_width,
        tile_height
    };
}

Rect2D ScreenTileData::tile_for_index(u32 index) const {
    const u32 row = index / cols;
    const u32 col = index % cols;

    const u32 row_start = row * tile_height;
    const u32 col_start = col * tile_width;

    return Rect2D {
        (int) col_start,
        (int) row_start,
        (int) (col_start + tile_width),
        (int) (row_start + tile_height)
    };
}

u32 ScreenTileData::num_tasks() const {
    return rows * cols;
}

IndexBounds ScreenTileData::index_bounds_for_bb(Rect2D bounding_box) const {
    const i32 min_col = MAX(0, bounding_box.minX / (i32) tile_width);
    const i32 min_row = MAX(0, bounding_box.minY / (i32) tile_height);

    const i32 max_col = MIN(bounding_box.maxX / tile_width, cols - 1);
    const i32 max_row = MIN(bounding_box.maxY / tile_height, rows - 1);
    
    return IndexBounds {
        min_row,
        max_row,
        min_col,
        max_col
    };
}