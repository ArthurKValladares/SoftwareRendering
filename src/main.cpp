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
#include "uv.hpp"

#define SIMD false
#define SCREEN_WIDTH  1200
#define SCREEN_HEIGHT 800

Uint32* GetPixel(SDL_Surface *surface, Point2D point) {
    return (Uint32 *) ((Uint8 *) surface->pixels + (point.y * surface->pitch) +
                       (point.x * surface->format->BytesPerPixel));
}

// NOTE: No overflow protection here
Vec4i32* GetPixels(SDL_Surface *surface, Point2D start_point) {
    return (Vec4i32 *) ((Uint8 *) surface->pixels + (start_point.y * surface->pitch) +
                       (start_point.x * surface->format->BytesPerPixel));
}

void ClearSurface(ThreadPool &thread_pool, SDL_Surface *surface, Color color) {
    const int width = surface->w;
    const int height = surface->h;
    const Uint32 pixel_color =
        SDL_MapRGB(surface->format, color.red, color.green, color.blue);
    const Vec4i32 pixel_colors = Vec4i32(pixel_color);
    
    for (int y = 0; y < height; y += EdgeFunction::step_increment_y) {
        thread_pool.Schedule([=]() {
            for (int x = 0; x < width; x += EdgeFunction::step_increment_x) {
                Point2D point = Point2D{x, y};
                *GetPixels(surface, point) = pixel_colors;
            }
        });
    }
    thread_pool.Wait();
}

void ClearSurfaceSingle(ThreadPool &thread_pool, SDL_Surface *surface, Color color) {
    const int width = surface->w;
    const int height = surface->h;
    const Uint32 pixel_color =
        SDL_MapRGB(surface->format, color.red, color.green, color.blue);

    for (int y = 0; y < height; y += 1) {
        thread_pool.Schedule([=]() {
            for (int x = 0; x < width; x += 1) {
                Point2D point = Point2D{x, y};
                *GetPixel(surface, point) = pixel_color;
            }
        });
    }
    thread_pool.Wait();
}

void RenderPixels(SDL_Surface *surface, const Point2D &origin_point, Vec4i32 mask, Vec4f32 u, Vec4f32 v, const Texture &texture) {
    float us[4];
    u.store(us);
    float vs[4];
    v.store(vs);
    
    // TODO: Might have to do bounds checks on the surface here
    i32 ret[4];
    mask.store(ret);
    if (ret[0]) {
        const Point2D p = origin_point;
        const Color color = texture.get_pixel_uv(us[0], vs[0]);
        *GetPixel(surface, p) = SDL_MapRGB(surface->format, color.red, color.green, color.blue);
    }
    if (ret[1]) {
        const Point2D p = origin_point + Point2D(1, 0);
        const Color color = texture.get_pixel_uv(us[1], vs[1]);
        *GetPixel(surface, p) = SDL_MapRGB(surface->format, color.red, color.green, color.blue);
    }
    if (ret[2]) {
        const Point2D p = origin_point + Point2D(2, 0);
        const Color color = texture.get_pixel_uv(us[2], vs[2]);
        *GetPixel(surface, p) = SDL_MapRGB(surface->format, color.red, color.green, color.blue);
    }
    if (ret[3]) {
        const Point2D p = origin_point + Point2D(3, 0);
        const Color color = texture.get_pixel_uv(us[3], vs[3]);
        *GetPixel(surface, p) = SDL_MapRGB(surface->format, color.red, color.green, color.blue);
    }
}

void DrawTriangle(ThreadPool &thread_pool, SDL_Surface *surface, const Triangle &triangle, const Texture &texture) {
    const Rect2D bounding_box = ClipRect(surface->w, surface->h, TriangleBoundingBox(triangle));
    const Point2D min_point = Point2D{bounding_box.minX, bounding_box.minY};
    
    const float c0_u = triangle.u0.u, c0_v = triangle.u0.v;
    const float c1_u = triangle.u1.u, c1_v = triangle.u1.v;
    const float c2_u = triangle.u2.u, c2_v = triangle.u2.v;
    
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
                
                if (mask.any_gte(0)) {
                    const Vec4f32 sum = (w0 + w1 + w2).to_float();
                    
                    const Vec4f32 b0 = w0.to_float() / sum;
                    const Vec4f32 b1 = w1.to_float() / sum;
                    const Vec4f32 b2 = w2.to_float() / sum;
                    
                    const Vec4f32 u_0 = Vec4f32(c0_u) * b0, v_0 = Vec4f32(c0_v) * b0;
                    const Vec4f32 u_1 = Vec4f32(c1_u) * b1, v_1 = Vec4f32(c1_v) * b1;
                    const Vec4f32 u_2 = Vec4f32(c2_u) * b2, v_2 = Vec4f32(c2_v) * b2;
                    
                    const Vec4f32 u = u_0 + u_1 + u_2;
                    const Vec4f32 v = v_0 + v_1 + v_2;
                    
                    RenderPixels(surface, Point2D(x, y), mask, u, v, texture);
                }
                
                w0 += e12.step_size_x;
                w1 += e20.step_size_x;
                w2 += e01.step_size_x;
            }
        });
    }
    thread_pool.Wait();
}

void DrawTriangleSingle(ThreadPool &thread_pool, SDL_Surface *surface, const Triangle &triangle, const Texture &texture) {
    const Rect2D bounding_box = ClipRect(surface->w, surface->h, TriangleBoundingBox(triangle));
    const Point2D min_point = Point2D{bounding_box.minX, bounding_box.minY};
    
    const i32 A01  = triangle.v0.y - triangle.v1.y;
    const i32 A12  = triangle.v1.y - triangle.v2.y;
    const i32 A20  = triangle.v2.y - triangle.v0.y;
    
    const i32 B01  = triangle.v1.x - triangle.v0.x;
    const i32 B12  = triangle.v2.x - triangle.v1.x;
    const i32 B20  = triangle.v0.x - triangle.v2.x;
    
    const auto edge_function = [](const Point2D& a, const Point2D& b, const Point2D& c) {
        return (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);
    };
    const i32 w0_init = edge_function(triangle.v1, triangle.v2, min_point);
    const i32 w1_init = edge_function(triangle.v2, triangle.v0, min_point);
    const i32 w2_init = edge_function(triangle.v0, triangle.v1, min_point);
    
    for (int y = bounding_box.minY; y <= bounding_box.maxY; y += 1) {
        const i32 delta_y = y - bounding_box.minY;
        
        const i32 w0_row = w0_init + B12 * delta_y;
        const i32 w1_row = w1_init + B20 * delta_y;
        const i32 w2_row = w2_init + B01 * delta_y;
        
        thread_pool.Schedule([=]() {
            i32 w0 = w0_row;
            i32 w1 = w1_row;
            i32 w2 = w2_row;
            
            for (int x = bounding_box.minX; x <= bounding_box.maxX; x += 1) {
                const i32 mask = w0 | w1 | w2;
                
                if (mask >= 0) {
                    const i32 sum = w0 + w1 + w2;
                    
                    const float b0 = (float) w0 / sum;
                    const float b1 = (float) w1 / sum;
                    const float b2 = (float) w2 / sum;

                    const Color c0 = triangle.c0 * b0;
                    const Color c1 = triangle.c1 * b1;
                    const Color c2 = triangle.c2 * b2;
                    const Color color = c0 + c1 + c2;

                    const UV u0 = triangle.u0 * b0;
                    const UV u1 = triangle.u1 * b1;
                    const UV u2 = triangle.u2 * b2;
                    const UV uv = UV{
                        u0.u + u1.u + u2.u,
                        u0.v + u1.v + u2.v
                    };
                    
                    const Point2D point = Point2D{x, y};
                    const Color texture_color = texture.get_pixel_uv(uv.u, uv.v);
                    *GetPixel(surface, point) = SDL_MapRGB(surface->format, texture_color.red, texture_color.green, texture_color.blue);
                }
                
                w0 += A12;
                w1 += A20;
                w2 += A01;
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
#if SIMD
        DrawTriangle(thread_pool, surface, triangle, mesh.texture);
#else
        DrawTriangleSingle(thread_pool, surface, triangle, mesh.texture);
#endif
    }
}

Mesh RotateMesh(Mesh mesh, Point2D pivot, float angle) {
    for (Triangle &triangle : mesh.triangles) {
        triangle = rotate_triangle(triangle, pivot, angle);
    }
    return mesh;
}

int main(int argc, char *argv[]) {
    const Texture texture = Texture("test.jpg");
    
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
    
    const float triangle_margin = 0.2f;
    const int margin_w = round(surface->w * triangle_margin);
    const int margin_h = round(surface->h * triangle_margin);

    const Mesh mesh = {
        {
            Triangle{
                Point2D{margin_w, margin_h},
                Point2D{surface->w - margin_w, margin_h},
                Point2D{surface->w - margin_w, surface->h - margin_h},
                Color{255, 0, 0},
                Color{0, 0, 0},
                Color{0, 255, 0},
                UV{1.0, 0.0},
                UV{0.0, 0.0},
                UV{0.0, 1.0},
                
            },
            Triangle{
                Point2D{margin_w, margin_h},
                Point2D{surface->w - margin_w, surface->h - margin_h},
                Point2D{margin_w, surface->h - margin_h},
                Color{255, 0, 0},
                Color{0, 255, 0},
                Color{255, 255, 0},
                UV{1.0, 0.0},
                UV{0.0, 1.0},
                UV{1.0, 1.0}
            }
        },
        texture
    };

    // Render loop
    const float rotate_delta = 0.03;
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
                            rotate_angle += rotate_delta;
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
        
#if SIMD
        ClearSurface(thread_pool, surface, Color{100, 100, 100});
#else
        ClearSurfaceSingle(thread_pool, surface, Color{100, 100, 100});
#endif
        
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
