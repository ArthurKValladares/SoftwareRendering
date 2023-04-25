#include <chrono>

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <vector>
#include <thread>
#include <SDL.h>


#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define MAX3(a,b, c) MAX(a, MAX(b, c))
#define MIN3(a,b, c) MIN(a, MIN(b, c))

#define SCREEN_WIDTH    800
#define SCREEN_HEIGHT   600

struct Point2D {
    int x;
    int y;
};

struct Rect2D {
    int minX;
    int minY;
    int maxX;
    int maxY;
};

// Assumes counter-clockwise winding order
struct Triangle {
    Point2D a;
    Point2D b;
    Point2D c;
};

Rect2D TriangleBoundingBox(Triangle triangle) {
    const int minY = MIN3(triangle.a.y, triangle.b.y, triangle.c.y);
    const int minX = MIN3(triangle.a.x, triangle.b.x, triangle.c.x);
    const int maxX = MAX3(triangle.a.x, triangle.b.x, triangle.c.x);
    const int maxY = MAX3(triangle.a.y, triangle.b.y, triangle.c.y);
    return Rect2D{minX, minY, maxX, maxY};
}

Rect2D ClipRect(SDL_Surface* surface, Rect2D rect) {
    const int minX = MAX(rect.minX, 0);
    const int minY = MAX(rect.minY, 0);
    const int maxX = MIN(rect.maxX, surface->w - 1);
    const int maxY = MIN(rect.maxY, surface->h - 1);
    return Rect2D{ minX, minY, maxX, maxY };
}

int EdgeFunction(const Point2D& a, const Point2D& b, const Point2D& c)
{
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

Uint32* GetPixel(SDL_Surface* surface, Point2D point) {
    return (Uint32*)((Uint8*)surface->pixels + (point.y * surface->pitch) + (point.x * surface->format->BytesPerPixel));
}

void ClearSurface(SDL_Surface* surface, Uint8 red, Uint8 green, Uint8 blue) {
    const int width = surface->w;
    const int height = surface->h;
    SDL_PixelFormat* pixel_format = surface->format;
    assert(surface->format->BytesPerPixel == 4); // Not supporting non-32-bit pixel formats
    const Uint32 pixel_color = SDL_MapRGB(pixel_format, red, green, blue);

    Point2D point;
    for (point.y = 0; point.y < height; ++point.y) {
        for (point.x = 0; point.x < width; ++point.x) {
            Uint32* p = GetPixel(surface, point);
            *p = pixel_color;
        }
    }
}

void DrawTriangle(std::thread *threads, SDL_Surface* surface, Triangle triangle, Uint8 red, Uint8 green, Uint8 blue) {
    const Rect2D bounding_box = ClipRect(surface, TriangleBoundingBox(triangle));
    const Uint32 pixel_color = SDL_MapRGB(surface->format, red, green, blue);

    Point2D point;
    for (point.y = bounding_box.minY; point.y < bounding_box.maxY; ++point.y) {
        for (point.x = bounding_box.minX; point.x < bounding_box.maxX; ++point.x) {
            const int e1 = EdgeFunction(triangle.a, triangle.b, point);
            const int e2 = EdgeFunction(triangle.b, triangle.c, point);
            const int e3 = EdgeFunction(triangle.c, triangle.a, point);

            if (e1 >= 0 && e2 >= 0 && e3 >= 0) {
                Uint32* p = GetPixel(surface, point);
                *p = pixel_color;
            }
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

    std::thread threads[SCREEN_HEIGHT];

    const float triangle_margin = 0.1;
    const int margin_w = round(surface->w * triangle_margin);
    const int margin_h = round(surface->h * triangle_margin);

    const Triangle triangle = Triangle{ Point2D{surface->w - margin_w, margin_h}, Point2D{margin_w, surface->h - margin_h}, Point2D{margin_w, margin_h} };
    // Render loop
    bool quit = false;
    while (!quit)
    {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
            case SDL_KEYDOWN:
                switch (e.key.keysym.sym) {
                case SDLK_ESCAPE:
                    quit = true;
                    break;
                default:
                    break;
                }
                break;
            case SDL_QUIT:
                quit = true;
                break;
            default:
                break;
            }
        }
        if (SDL_LockSurface(surface) < 0) {
            printf("Could not lock SDL surface");
            return 0;
        }

        std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        ClearSurface(surface, 255, 0, 0);
        DrawTriangle(threads, surface, triangle, 0, 255, 0);
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        printf("dt: %ld ms\n", (int) std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count());

        SDL_UnlockSurface(surface);
        SDL_UpdateWindowSurface(window);
    }

    
    // Cleanup
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}