#include <stdio.h>
#include <stdbool.h>
#include <assert.h>

#include <SDL.h>

// Define screen dimensions
#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600

void clear_surface(SDL_Surface* surface, Uint8 red, Uint8 green, Uint8 blue) {
    const int width = surface->w;
    const int height = surface->h;
    SDL_PixelFormat* pixel_format = surface->format;
    const int bpp = pixel_format->BytesPerPixel;
    assert(bpp == 4); // Not supporting non-32-bit pixel formats
    const Uint32 pixel_color = SDL_MapRGB(pixel_format, red, green, blue);

    for (int h = 0; h < height; ++h) {
        for (int w = 0; w < width; ++w) {
            Uint32* p = (Uint32*) ((Uint8*) surface->pixels + (h * surface->pitch) + (w * bpp));
            *p = pixel_color;
        }
    }
}

int main(int argc, char* argv[])
{
    // Init
    if (SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        printf("SDL could not be initialized!\n"
            "SDL_Error: %s\n", SDL_GetError());
        return 0;
    }
    SDL_Window* window = SDL_CreateWindow("Basic C SDL project",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_SHOWN);
    if (!window)
    {
        printf("Window could not be created!\n"
            "SDL_Error: %s\n", SDL_GetError());
        return 0;
    }
    SDL_Surface* surface = SDL_GetWindowSurface(window);
 
    // Render loop
    bool quit = false;
    while (!quit)
    {
        SDL_Event e;
        SDL_WaitEvent(&e);
        if (e.type == SDL_QUIT)
        {
            quit = true;
        }
        if (SDL_LockSurface(surface) < 0) {
            printf("Could not lock SDL surface");
            return 0;
        }

        clear_surface(surface, 255, 0, 0);

        SDL_UnlockSurface(surface);

        SDL_UpdateWindowSurface(window);
    }

    
    // Cleanup
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}