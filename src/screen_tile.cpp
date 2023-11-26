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

ScreenTileData partition_screen_into_tiles() {
    u32 num_tiles = std::thread::hardware_concurrency();
    // Ensure that the total number of tiles is even so that it can easily be factored
    if (num_tiles % 2 == 1) {
        num_tiles *= 2;
    }
    const u32 rows = u_log2(num_tiles);
    const u32 cols = num_tiles / rows;
    return ScreenTileData{
        rows,
        cols
    };
}

Rect2D ScreenTileData::tile_for_index(SDL_Surface* surface, u32 index) const {
    // TODO: Should probably do some of this calculation up-front together with getting
    // The data itself
    const u32 tile_width = ceil(surface->w / cols);
    const u32 tile_height = ceil(surface->h / rows);

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