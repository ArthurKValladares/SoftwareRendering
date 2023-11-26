#pragma once

#include "defs.h"
#include "rect.h"

struct ScreenTileData {
    Rect2D tile_for_index(SDL_Surface* surface, u32 index) const;
    u32 num_tasks() const;
    
    u32 rows;
    u32 cols;
};

ScreenTileData partition_screen_into_tiles();
