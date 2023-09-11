#include <assert.h>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <stdbool.h>
#include <stdio.h>
#include <vector>
#ifdef __APPLE__
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

#include "ThreadPool.h"
#include "defs.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define MAX3(a, b, c) MAX(a, MAX(b, c))
#define MIN3(a, b, c) MIN(a, MIN(b, c))

#define SCREEN_WIDTH  1200
#define SCREEN_HEIGHT 800

struct Point2D {
    int x;
    int y;
};

Point2D
point_sub(Point2D a, Point2D b) {
    return Point2D{a.x - b.x, a.y - b.y};
}

Point2D
rotate_point(Point2D point, Point2D pivot, float angle) {
    const float s = sin(angle);
    const float c = cos(angle);

    const Point2D p = point_sub(point, pivot);

    const float new_x = p.x * c - p.y * s;
    const float new_y = p.x * s + p.y * c;

    return Point2D{(int) round(new_x) + pivot.x, (int) round(new_y) + pivot.y};
}

struct Rect2D {
    int minX;
    int minY;
    int maxX;
    int maxY;
};

struct Color {
    Uint8 red;
    Uint8 green;
    Uint8 blue;
};

Color
mul(Color c, float s) {
    return Color{
        (Uint8) ((float) c.red * s),
        (Uint8) ((float) c.green * s),
        (Uint8) ((float) c.blue * s),
    };
}

Color
add(Color l, Color r) {
    return Color{
        (Uint8) ((int) l.red + r.red),
        (Uint8) ((int) l.green + r.green),
        (Uint8) ((int) l.blue + r.blue),
    };
}

// Assumes counter-clockwise winding order
struct Triangle {
    Point2D a;
    Point2D b;
    Point2D c;
    Color ca;
    Color cb;
    Color cc;
};

Triangle
rotate_triangle(Triangle triangle, Point2D pivot, float angle) {
    return Triangle{rotate_point(triangle.a, pivot, angle),
                    rotate_point(triangle.b, pivot, angle),
                    rotate_point(triangle.c, pivot, angle),
                    triangle.ca,
                    triangle.cb,
                    triangle.cc};
}

Rect2D
TriangleBoundingBox(Triangle triangle) {
    const int minY = MIN3(triangle.a.y, triangle.b.y, triangle.c.y);
    const int minX = MIN3(triangle.a.x, triangle.b.x, triangle.c.x);
    const int maxX = MAX3(triangle.a.x, triangle.b.x, triangle.c.x);
    const int maxY = MAX3(triangle.a.y, triangle.b.y, triangle.c.y);
    return Rect2D{minX, minY, maxX, maxY};
}

Rect2D
ClipRect(SDL_Surface *surface, Rect2D rect) {
    const int minX = MAX(rect.minX, 0);
    const int minY = MAX(rect.minY, 0);
    const int maxX = MIN(rect.maxX, surface->w - 1);
    const int maxY = MIN(rect.maxY, surface->h - 1);
    return Rect2D{minX, minY, maxX, maxY};
}

int
EdgeFunction(const Point2D &a, const Point2D &b, const Point2D &c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

Uint32 *
GetPixel(SDL_Surface *surface, Point2D point) {
    return (Uint32 *) ((Uint8 *) surface->pixels + (point.y * surface->pitch) +
                       (point.x * surface->format->BytesPerPixel));
}

void
ClearSurface(ThreadPool &thread_pool, SDL_Surface *surface, Color color) {
    const int width = surface->w;
    const int height = surface->h;
    const Uint32 pixel_color =
        SDL_MapRGB(surface->format, color.red, color.green, color.blue);

    for (int y = 0; y < height; ++y) {
        thread_pool.Schedule([=]() {
            for (int x = 0; x < width; ++x) {
                Point2D point = Point2D{x, y};
                Uint32 *p = GetPixel(surface, point);
                *p = pixel_color;
            }
        });
    }
    thread_pool.Wait();
}

void
DrawTriangle(ThreadPool &thread_pool, SDL_Surface *surface,
             const Triangle &triangle) {
    const Rect2D bounding_box =
        ClipRect(surface, TriangleBoundingBox(triangle));

    const Point2D min_point = Point2D{bounding_box.minX, bounding_box.minY};

    const int e0 = EdgeFunction(triangle.b, triangle.c, min_point);
    const int e1 = EdgeFunction(triangle.c, triangle.a, min_point);
    const int e2 = EdgeFunction(triangle.a, triangle.b, min_point);

    struct EdgeFunctionSteps {
        int step01;
        int step12;
        int step20;
    };

    EdgeFunctionSteps A{triangle.a.y - triangle.b.y,
                        triangle.b.y - triangle.c.y,
                        triangle.c.y - triangle.a.y};

    EdgeFunctionSteps B{triangle.b.x - triangle.a.x,
                        triangle.c.x - triangle.b.x,
                        triangle.a.x - triangle.c.x};

    for (int y = bounding_box.minY; y <= bounding_box.maxY; ++y) {
        thread_pool.Schedule([=]() {
            const int delta_y = y - bounding_box.minY;
            for (int x = bounding_box.minX; x <= bounding_box.maxX; ++x) {
                const int delta_x = x - bounding_box.minX;

                const int _e0 = e0 + B.step12 * delta_y + A.step12 * delta_x;
                const int _e1 = e1 + B.step20 * delta_y + A.step20 * delta_x;
                const int _e2 = e2 + B.step01 * delta_y + A.step01 * delta_x;

                if ((_e0 | _e1 | _e2) >= 0) {
                    const int sum = _e0 + _e1 + _e2;

                    const float ne0 = (float) _e0 / sum;
                    const float ne1 = (float) _e1 / sum;
                    const float ne2 = (float) _e2 / sum;

                    const Color wc0 = mul(triangle.ca, ne0);
                    const Color wc1 = mul(triangle.cb, ne1);
                    const Color wc2 = mul(triangle.cc, ne2);

                    const Color color = add(wc0, add(wc1, wc2));

                    const Point2D point = Point2D{x, y};
                    const Uint32 pixel_color = SDL_MapRGB(
                        surface->format, color.red, color.green, color.blue);
                    *GetPixel(surface, point) = pixel_color;
                }
            }
        });
    }
    thread_pool.Wait();
}

struct Mesh {
    std::vector<Triangle> triangles;
};

void
DrawMesh(ThreadPool &thread_pool, SDL_Surface *surface, const Mesh &mesh) {
    for (const Triangle &triangle : mesh.triangles) {
        DrawTriangle(thread_pool, surface, triangle);
    }
}

Mesh
RotateMesh(Mesh mesh, Point2D pivot, float angle) {
    for (Triangle &triangle : mesh.triangles) {
        triangle = rotate_triangle(triangle, pivot, angle);
    }
    return mesh;
}

int
main(int argc, char *argv[]) {
    int width, height, channels;
    unsigned char *img =
        stbi_load("test.jpg", &width, &height, &channels, 0);
    if (img == NULL) {
        printf("Error in loading the image\n");
        exit(1);
    }
    
    ThreadPool thread_pool;

    // Init
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not be initialized!\n"
               "SDL_Error: %s\n",
               SDL_GetError());
        return 0;
    }
    SDL_Window *window = SDL_CreateWindow(
        "Basic C SDL project", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (!window) {
        printf("Window could not be created!\n"
               "SDL_Error: %s\n",
               SDL_GetError());
        return 0;
    }
    SDL_Surface *surface = SDL_GetWindowSurface(window);
    assert(surface->format->BytesPerPixel ==
           4);   // Not supporting non-32-bit pixel formats

    const float triangle_margin = 0.1f;
    const int margin_w = round(surface->w * triangle_margin);
    const int margin_h = round(surface->h * triangle_margin);

    const Mesh mesh = {
        {Triangle{Point2D{margin_w, margin_h},
                  Point2D{surface->w - margin_w, margin_h},
                  Point2D{surface->w - margin_w, surface->h - margin_h},
                  Color{255, 0, 0}, Color{0, 0, 0}, Color{0, 255, 0}},
         Triangle{Point2D{margin_w, margin_h},
                  Point2D{surface->w - margin_w, surface->h - margin_h},
                  Point2D{margin_w, surface->h - margin_h}, Color{255, 0, 0},
                  Color{0, 255, 0}, Color{255, 255, 0}}}};


    std::chrono::steady_clock::time_point render_begin =
        std::chrono::steady_clock::now();

    // Render loop
    bool quit = false;
    while (!quit) {
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

        const long elapsed_start =
            (long) std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - render_begin)
                .count();

        std::chrono::steady_clock::time_point begin =
            std::chrono::steady_clock::now();
        ClearSurface(thread_pool, surface, Color{100, 100, 100});
        const Mesh rotated_mesh =
            RotateMesh(mesh, Point2D{surface->w / 2, surface->h / 2},
                       ((float) elapsed_start) / 1000.0);
        DrawMesh(thread_pool, surface, rotated_mesh);
        printf("dt: %ld ms\n",
               (long) std::chrono::duration_cast<std::chrono::milliseconds>(
                   std::chrono::steady_clock::now() - begin)
                   .count());

        SDL_UnlockSurface(surface);
        SDL_UpdateWindowSurface(window);
    }

    // Cleanup
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
