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
#include "edge_function.hpp"

// TODO: Make sure these are always multiples of 4
#define SCREEN_WIDTH  1200
#define SCREEN_HEIGHT 800

Uint32* GetPixel(SDL_Surface *surface, Point2D point) {
    return (Uint32 *) ((Uint8 *) surface->pixels + (point.y * surface->pitch) +
                       (point.x * surface->format->BytesPerPixel));
}

void ClearSurface(ThreadPool &thread_pool, SDL_Surface *surface, Color color) {
    const int width = surface->w;
    const int height = surface->h;
    const Uint32 pixel_color =
        SDL_MapRGB(surface->format, color.red, color.green, color.blue);

    for (int y = 0; y < height; y += EdgeFunction::step_increment_y) {
        thread_pool.Schedule([=]() {
            for (int x = 0; x < width; x += EdgeFunction::step_increment_x) {
                Point2D point = Point2D{x, y};
                
                *GetPixel(surface, point) = pixel_color;
                *GetPixel(surface, point + Point2D(1, 0)) = pixel_color;
                *GetPixel(surface, point + Point2D(2, 0)) = pixel_color;
                *GetPixel(surface, point + Point2D(3, 0)) = pixel_color;
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

void RenderPixels(SDL_Surface *surface, const Point2D &origin_point, Vec4i32 mask) {
    // TODO: Will be passed in to this function later, either from texture or user
    const Uint32 pixel_color = SDL_MapRGB(surface->format, 255, 0, 0);
    
    i32 ret[4];
    mask.store(ret);
    
    
    // TODO: Might have to do bounds checks on the surface here
    if (ret[0]) {
        const Point2D p = origin_point;
        *GetPixel(surface, p) = pixel_color;
    }
    if (ret[1]) {
        const Point2D p = origin_point + Point2D(1, 0);
        *GetPixel(surface, p) = pixel_color;
    }
    if (ret[2]) {
        const Point2D p = origin_point + Point2D(2, 0);
        *GetPixel(surface, p) = pixel_color;
    }
    if (ret[3]) {
        const Point2D p = origin_point + Point2D(3, 0);
        *GetPixel(surface, p) = pixel_color;
    }
}

void DrawTriangle(ThreadPool &thread_pool, SDL_Surface *surface, const Triangle &triangle, const Texture &texture) {
    const Rect2D bounding_box = ClipRect(surface->w, surface->h, TriangleBoundingBox(triangle));
    const Point2D min_point = Point2D{bounding_box.minX, bounding_box.minY};
    
    EdgeFunction e01, e12, e20;
    const Vec4i32 w0_init = e12.Init(triangle.v1, triangle.v2, min_point);
    const Vec4i32 w1_init = e20.Init(triangle.v2, triangle.v0, min_point);
    const Vec4i32 w2_init = e01.Init(triangle.v0, triangle.v1, min_point);
    
    for (int y = bounding_box.minY; y <= bounding_box.maxY; y += EdgeFunction::step_increment_y) {
        const Vec4i32 delta_y = Vec4i32(y - bounding_box.minY);
        
        const Vec4i32 w0_row = w0_init + e12.step_size_y * delta_y;
        const Vec4i32 w1_row = w1_init + e20.step_size_y * delta_y;
        const Vec4i32 w2_row = w2_init + e01.step_size_y * delta_y;
        
        thread_pool.Schedule([=]() {
            Vec4i32 w0 = w0_row;
            Vec4i32 w1 = w1_row;
            Vec4i32 w2 = w2_row;
            
            for (int x = bounding_box.minX; x <= bounding_box.maxX; x += EdgeFunction::step_increment_x) {
                const Vec4i32 mask = w0 | w1 | w2;
                
                if (mask >= 0) {
                    RenderPixels(surface, Point2D(x, y), mask);
                }
                
                w0 += e12.step_size_x;
                w1 += e20.step_size_x;
                w2 += e01.step_size_x;
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

    const float triangle_margin = 0.3f;
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

    // Render loop
    float rotate_angle = 0.0;
    bool quit = false;
    while (!quit) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            switch (e.type) {
                case SDL_KEYDOWN: {
                    switch (e.key.keysym.sym) {
                        case SDLK_ESCAPE: {
                            quit = true;
                            break;
                        }
                        case SDLK_r: {
                            rotate_angle += 0.01;
                        }
                        default: {
                            break;
                        }
                    }
                    break;
                }
                case SDL_QUIT: {
                    quit = true;
                    break;
                }
                default: {
                    break;
                }
            }
        }
        if (SDL_LockSurface(surface) < 0) {
            printf("Could not lock SDL surface");
            return 0;
        }

        const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        
        ClearSurface(thread_pool, surface, Color{100, 100, 100});
        
        const Mesh rotated_mesh =
            RotateMesh(mesh, Point2D{surface->w / 2, surface->h / 2},
                       rotate_angle);
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
