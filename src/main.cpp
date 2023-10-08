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
#include "point.hpp"
#include "texture.h"
#include "color.hpp"
#include "rect.hpp"
#include "defs.h"
#include "triangle.hpp"

// TODO: Make sure these are always multiples of 4
#define SCREEN_WIDTH  1200
#define SCREEN_HEIGHT 800

int EdgeFunction(const Point2D &a, const Point2D &b, const Point2D &c) {
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

Uint32* GetPixel(SDL_Surface *surface, Point2D point) {
    return (Uint32 *) ((Uint8 *) surface->pixels + (point.y * surface->pitch) +
                       (point.x * surface->format->BytesPerPixel));
}

void ClearSurface(ThreadPool &thread_pool, SDL_Surface *surface, Color color) {
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

Color get_pixel(const Texture& texture, Uint32 width, Uint32 height) {
    const Uint32 stride = texture.width * texture.channels;
    const Uint32 start = height * stride + width * texture.channels;
    const Uint8 red = texture.img[start];
    const Uint8 green = texture.img[start + 1];
    const Uint8 blue = texture.img[start + 2];
    return Color{red, green, blue};
}

void DrawTriangle(ThreadPool &thread_pool, SDL_Surface *surface, const Triangle &triangle, const Texture &texture) {
    const Rect2D bounding_box = ClipRect(surface->w, surface->h, TriangleBoundingBox(triangle));
    
    for (int y = bounding_box.minY; y <= bounding_box.maxY; ++y) {
        thread_pool.Schedule([=]() {
            for (int x = bounding_box.minX; x <= bounding_box.maxX; ++x) {
                const Point2D p = Point2D(x, y);
                
                const int edge_0 = EdgeFunction(triangle.b, triangle.c, p);
                const int edge_1 = EdgeFunction(triangle.c, triangle.a, p);
                const int edge_2 = EdgeFunction(triangle.a, triangle.b, p);

                if ((edge_0 | edge_1 | edge_2) >= 0) {
                    const Uint32 pixel_color = SDL_MapRGB(surface->format, 255, 0, 0);
                    *GetPixel(surface, p) = pixel_color;
                }
            }
        });
    }
    thread_pool.Wait();
}

struct Mesh {
    std::vector<Triangle> triangles;
    Texture texture;
};

void DrawMesh(ThreadPool &thread_pool, SDL_Surface *surface, const Mesh &mesh) {
    for (const Triangle &triangle : mesh.triangles) {
        DrawTriangle(thread_pool, surface, triangle, mesh.texture);
    }
}

Mesh RotateMesh(Mesh mesh, Point2D pivot, float angle) {
    for (Triangle &triangle : mesh.triangles) {
        triangle = rotate_triangle(triangle, pivot, angle);
    }
    return mesh;
}

int main(int argc, char *argv[]) {
    const Texture texture = create_texture("test.jpg");
    
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
        {
            Triangle{Point2D{margin_w, margin_h},
                  Point2D{surface->w - margin_w, margin_h},
                  Point2D{surface->w - margin_w, surface->h - margin_h},
                  Color{255, 0, 0}, Color{0, 0, 0}, Color{0, 255, 0}},
            Triangle{Point2D{margin_w, margin_h},
                  Point2D{surface->w - margin_w, surface->h - margin_h},
                  Point2D{margin_w, surface->h - margin_h}, Color{255, 0, 0},
                  Color{0, 255, 0}, Color{255, 255, 0}}
            
        },
        texture
    };


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

        const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        
        ClearSurface(thread_pool, surface, Color{100, 100, 100});
        
        const Mesh rotated_mesh =
            RotateMesh(mesh, Point2D{surface->w / 2, surface->h / 2},
                       ((float) elapsed_start) / 1000.0);
        DrawMesh(thread_pool, surface, rotated_mesh);
        
        const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        printf("dt: %ld ms\n", (long) std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count());

        SDL_UnlockSurface(surface);
        SDL_UpdateWindowSurface(window);
    }

    // Cleanup
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
