#include <chrono>

#include <stdio.h>
#include <stdbool.h>
#include <assert.h>
#include <vector>
#ifdef __APPLE__
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

#include "ThreadPool.h"

#define MAX(a,b) ((a) > (b) ? (a) : (b))
#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define MAX3(a,b, c) MAX(a, MAX(b, c))
#define MIN3(a,b, c) MIN(a, MIN(b, c))

#define SCREEN_WIDTH    1200
#define SCREEN_HEIGHT   800

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

void ClearSurface(ThreadPool& thread_pool, SDL_Surface* surface, Uint8 red, Uint8 green, Uint8 blue) {
    const int width = surface->w;
    const int height = surface->h;
    const Uint32 pixel_color = SDL_MapRGB(surface->format, red, green, blue);

    for (int y = 0; y < height; ++y) {
        thread_pool.Schedule([=]() {
            for (int x = 0; x < width; ++x) {
                Point2D point = Point2D{ x,y };
                Uint32* p = GetPixel(surface, point);
                *p = pixel_color;
            }
        });
    }
}

void DrawTriangle(ThreadPool& thread_pool, SDL_Surface* surface, Triangle triangle, Uint8 red, Uint8 green, Uint8 blue) {
    const Rect2D bounding_box = ClipRect(surface, TriangleBoundingBox(triangle));
    const Uint32 pixel_color = SDL_MapRGB(surface->format, red, green, blue);

    const Point2D min_point = Point2D{ bounding_box.minX,bounding_box.minY };

    const int e0 = EdgeFunction(triangle.b, triangle.c, min_point);
    const int e1 = EdgeFunction(triangle.c, triangle.a, min_point);
    const int e2 = EdgeFunction(triangle.a, triangle.b, min_point);

    struct EdgeFunctionSteps {
        int step01;
        int step12;
        int step20;
    };

    EdgeFunctionSteps A{
        triangle.a.y - triangle.b.y,
        triangle.b.y - triangle.c.y,
        triangle.c.y - triangle.a.y
    };

    EdgeFunctionSteps B{
        triangle.b.x - triangle.a.x,
        triangle.c.x - triangle.b.x,
        triangle.a.x - triangle.c.x
    };

    for (int y = bounding_box.minY; y <= bounding_box.maxY; ++y) {
        const int delta_y = y - bounding_box.minY;
        thread_pool.Schedule([=]() {
            for (int x = bounding_box.minX; x <= bounding_box.maxX; ++x) {
                const int delta_x = x - bounding_box.minX;
                
                const int _e0 = e0 + B.step12 * delta_y + A.step12 * delta_x;
                const int _e1 = e1 + B.step20 * delta_y + A.step20 * delta_x;
                const int _e2 = e2 + B.step01 * delta_y + A.step01 * delta_x;
                
                Point2D point = Point2D{ x,y };
                if ((_e0 | _e1 | _e2) >= 0) {
                    *GetPixel(surface, point) = pixel_color;
                }
            }
        });
    }

    thread_pool.Wait();
}

int main(int argc, char* argv[])
{
    ThreadPool thread_pool;

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
    assert(surface->format->BytesPerPixel == 4); // Not supporting non-32-bit pixel formats

    const float triangle_margin = 0.1f;
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
        ClearSurface(thread_pool, surface, 255, 0, 0);
        DrawTriangle(thread_pool, surface, triangle, 0, 255, 0);
        std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        printf("dt: %ld ms\n", (long) std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count());

        SDL_UnlockSurface(surface);
        SDL_UpdateWindowSurface(window);
    }

    
    // Cleanup
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
