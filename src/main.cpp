#include <assert.h>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <stdbool.h>
#include <stdio.h>
#include <vector>

#include "point.h"
#include "texture.h"
#include "color.h"
#include "rect.h"
#include "defs.h"
#include "triangle.h"
#include "edge_function.h"
#include "uv.h"
#include "mesh.h"
#include "line.h"
#include "ThreadPool.h"
#include "math/vec4f32.h"
#include "math/vec4i32.h"

#define SIMD true
#define SCREEN_WIDTH  1200
#define SCREEN_HEIGHT 800

Vec4i32 GetPixelOffsets(SDL_Surface* surface, Vec4i32 xs, Vec4i32 ys) {
    return ys * Vec4i32(surface->pitch) + xs * Vec4i32(surface->format->BytesPerPixel);
}

u32 GetPixelOffset(SDL_Surface* surface, Point2D point) {
    return point.y * surface->pitch + point.x * surface->format->BytesPerPixel;
}

Uint32* GetPixel(SDL_Surface *surface, u32 offset) {
    return (Uint32 *) ((Uint8 *) surface->pixels + offset);
}

Uint32* GetPixel(SDL_Surface* surface, Point2D point) {
    return GetPixel(surface, GetPixelOffset(surface, point));
}

Point2D hacky_project_to_surface(SDL_Surface* surface, Point3D_f point) {
    const u32 w = surface->w;
    const u32 h = surface->h;
    const float sx = (point.x + 1.0) / (2.0) * w;
    const float sy = (point.y + 1.0) / (2.0) * h;
    return Point2D{(int) round(sx), (int) round(sy)};
}

Rect2D bounding_box(Point2D p0, Point2D p1, Point2D p2) {
    const int minY = MIN3(p0.y, p1.y, p2.y);
    const int minX = MIN3(p0.x, p1.x, p2.x);
    const int maxX = MAX3(p0.x, p1.x, p2.x);
    const int maxY = MAX3(p0.y, p1.y, p2.y);
    return Rect2D{minX, minY, maxX, maxY};
}

void ClearSurface(ThreadPool& thread_pool, SDL_Surface *surface, Color color) {
    const int width = surface->w;
    const int height = surface->h;
    const Uint32 pixel_color =
        SDL_MapRGB(surface->format, color.red, color.green, color.blue);
    const Vec4i32 pixel_colors = Vec4i32(pixel_color);
    
    for (int y = 0; y < height; y += 1) {
        thread_pool.Schedule([=]() {
            Point2D point = Point2D{ 0, y };
            for (point.x = 0; point.x < width; point.x += EdgeFunction::step_increment_x) {
                *(Vec4i32*)GetPixel(surface, point) = pixel_colors;
            }
        });
    }
    thread_pool.Wait();
}

void ClearSurfaceSingle(ThreadPool& thread_pool, SDL_Surface *surface, Color color) {
    const int width = surface->w;
    const int height = surface->h;
    const Uint32 pixel_color =
        SDL_MapRGB(surface->format, color.red, color.green, color.blue);

    for (int y = 0; y < height; y += 1) {
        thread_pool.Schedule([=]() {
            Point2D point = { 0, y };
            for (point.x = 0; point.x < width; point.x += 1) {
                *GetPixel(surface, point) = pixel_color;
            }
        });
    }
    thread_pool.Wait();
}

void RenderPixels(SDL_Surface *surface, const Point2D &origin_point, Vec4i32 mask, Vec4f32 u, Vec4f32 v, const Texture &texture) {
    const Vec4i32 ui = (u.clamp(0.0, 1.0) * texture.m_width).to_int_nearest();
    const Vec4i32 vi = (v.clamp(0.0, 1.0) * texture.m_height).to_int_nearest();
    const Vec4i32 tex_idx = vi * Vec4i32(texture.m_width) + ui;

    const Vec4i32 xs = Vec4i32(origin_point.x) + Vec4i32(0, 1, 2, 3);
    const Vec4i32 ys = Vec4i32(origin_point.y);

    const Vec4i32 pixel_offsets = GetPixelOffsets(surface, xs, ys);

    if (mask.x()) {
        *GetPixel(surface, pixel_offsets.x()) = texture.get_pixel_from_idx(tex_idx.x());
    }
    if (mask.y()) {
        *GetPixel(surface, pixel_offsets.y()) = texture.get_pixel_from_idx(tex_idx.y());;
    }
    if (mask.z()) {
        *GetPixel(surface, pixel_offsets.z()) = texture.get_pixel_from_idx(tex_idx.z());;
    }
    if (mask.w()) {
        *GetPixel(surface, pixel_offsets.w()) = texture.get_pixel_from_idx(tex_idx.w());
    }
}

void DrawTriangle(ThreadPool& thread_pool, SDL_Surface *surface, const Triangle &triangle, const Texture &texture) {
    const Point2D sv0 = hacky_project_to_surface(surface, triangle.v0.p);
    const Point2D sv1 = hacky_project_to_surface(surface, triangle.v1.p);
    const Point2D sv2 = hacky_project_to_surface(surface, triangle.v2.p);

    const Rect2D bounding_box = ClipRect(surface->w, surface->h, ::bounding_box(sv0, sv1, sv2));
    const Point2D min_point = Point2D{bounding_box.minX, bounding_box.minY};
    
    // TODO: I need to `project` this as well
    const float c0_u = triangle.v0.uv.u, c0_v = triangle.v0.uv.v;
    const float c1_u = triangle.v1.uv.u, c1_v = triangle.v1.uv.v;
    const float c2_u = triangle.v2.uv.u, c2_v = triangle.v2.uv.v;
    
    EdgeFunction e01, e12, e20;
    const Vec4i32 w0_init = e12.Init(sv1, sv2, min_point);
    const Vec4i32 w1_init = e20.Init(sv2, sv0, min_point);
    const Vec4i32 w2_init = e01.Init(sv0, sv1, min_point);
    
    for (int y = bounding_box.minY; y <= bounding_box.maxY; y += EdgeFunction::step_increment_y) {
        const Vec4i32 delta_y = Vec4i32(y - bounding_box.minY);
        
        const Vec4i32 w0_row = w0_init + e12.step_size_y * delta_y;
        const Vec4i32 w1_row = w1_init + e20.step_size_y * delta_y;
        const Vec4i32 w2_row = w2_init + e01.step_size_y * delta_y;
        
        thread_pool.Schedule([=]() {
            Point2D point = { 0, y };

            Vec4i32 w0 = w0_row;
            Vec4i32 w1 = w1_row;
            Vec4i32 w2 = w2_row;
            
            for (point.x = bounding_box.minX; point.x <= bounding_box.maxX; point.x += EdgeFunction::step_increment_x) {
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
                    
                    RenderPixels(surface, point, mask, u, v, texture);
                }
                
                w0 += e12.step_size_x;
                w1 += e20.step_size_x;
                w2 += e01.step_size_x;
            }
        });
    }
    thread_pool.Wait();
}

void DrawTriangleSingle(ThreadPool& thread_pool, SDL_Surface *surface, const Triangle &triangle, const Texture &texture) {
    const Point2D sv0 = hacky_project_to_surface(surface, triangle.v0.p);
    const Point2D sv1 = hacky_project_to_surface(surface, triangle.v1.p);
    const Point2D sv2 = hacky_project_to_surface(surface, triangle.v2.p);

    const Rect2D bounding_box = ClipRect(surface->w, surface->h, ::bounding_box(sv0, sv1, sv2));
    const Point2D min_point = Point2D{bounding_box.minX, bounding_box.minY};
    
    const i32 A01  = sv0.y - sv1.y;
    const i32 A12  = sv1.y - sv2.y;
    const i32 A20  = sv2.y - sv0.y;
    
    const i32 B01  = sv1.x - sv0.x;
    const i32 B12  = sv2.x - sv1.x;
    const i32 B20  = sv0.x - sv2.x;
    
    const auto edge_function = [](const Point2D& a, const Point2D& b, const Point2D& c) {
        return (b.x-a.x)*(c.y-a.y) - (b.y-a.y)*(c.x-a.x);
    };
    const i32 w0_init = edge_function(sv1, sv2, min_point);
    const i32 w1_init = edge_function(sv2, sv0, min_point);
    const i32 w2_init = edge_function(sv0, sv1, min_point);
    
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

                    const UV u0 = triangle.v0.uv * b0;
                    const UV u1 = triangle.v1.uv * b1;
                    const UV u2 = triangle.v2.uv * b2;
                    const UV uv = UV{
                        u0.u + u1.u + u2.u,
                        u0.v + u1.v + u2.v
                    };
                    
                    const Point2D point = Point2D{x, y};
                    *GetPixel(surface, point) = texture.get_pixel_uv(uv.u, uv.v);
                }
                
                w0 += A12;
                w1 += A20;
                w2 += A01;
            }
        });
    }
    thread_pool.Wait();
}

// NOTE: I don't think this SIMD version is really worth it tbh,
// but I wanted to do it for completeness sake
void DrawLine(SDL_Surface *surface, const Line2D &line, const Uint32 mapped_color) {
    // TODO: This is very sloppy, will write something real later
    // Since we know the slope, we can be smarter about breaking out early when we go out of bounds
    // Maybe the testin can be SIMD too

    const auto is_in_bounds = [=](u32 x, u32 y) {
        return x > 0 && x < surface->w && y > 0 && y < surface->h;
    };
    
    Point2D p0 = line.p0;
    Point2D p1 = line.p1;
    
    const i32 dx = p1.x - p0.x;
    const i32 dy = p1.y - p0.y;
    
    if (abs(dx) > abs(dy)) {
        if (p0.x > p1.x) {
            std::swap(p0, p1);
        }
        const float slope = dy / (float) dx;

        const u32 x_start = MAX(p0.x, 0);
        const u32 x_end = MIN(p1.x, surface->w);
        for (u32 x = x_start; x <= x_end; x += 4) {
            const u32 x_delta = x - p0.x;

            const Vec4i32 xs = Vec4i32(x) + Vec4i32(0, 1, 2, 3);
            const Vec4i32 ys = (Vec4f32(p0.y) + Vec4f32(slope) * Vec4f32(x_delta, x_delta + 1, x_delta + 2, x_delta + 3)).to_int_nearest();
            if (is_in_bounds(xs.x(), ys.x())) {
                *GetPixel(surface, Point2D(xs.x(), ys.x())) = mapped_color;
            }
            if (is_in_bounds(xs.y(), ys.y())) {
                *GetPixel(surface, Point2D(xs.y(), ys.y())) = mapped_color;
            }
            if (is_in_bounds(xs.z(), ys.z())) {
                *GetPixel(surface, Point2D(xs.z(), ys.z())) = mapped_color;
            }
            if (is_in_bounds(xs.w(), ys.w())) {
                *GetPixel(surface, Point2D(xs.w(), ys.w())) = mapped_color;
            }
        }
    } else {
        if (p0.y > p1.y) {
            std::swap(p0, p1);
        }
        const float slope = dx / (float) dy;

        const u32 y_start = MAX(p0.y, 0);
        const u32 y_end = MIN(p1.y, surface->h);
        for (i32 y = y_start; y <= y_end; y += 4) {
            const u32 y_delta = y - p0.y;

            const Vec4i32 ys = Vec4i32(y) + Vec4i32(0, 1, 2, 3);
            const Vec4i32 xs = (Vec4f32(p0.x) + Vec4f32(slope) * Vec4f32(y_delta, y_delta + 1, y_delta + 2, y_delta + 3)).to_int_nearest();
            if (is_in_bounds(xs.x(), ys.x())) {
                *GetPixel(surface, Point2D(xs.x(), ys.x())) = mapped_color;
            }
            if (is_in_bounds(xs.y(), ys.y())) {
                *GetPixel(surface, Point2D(xs.y(), ys.y())) = mapped_color;
            }
            if (is_in_bounds(xs.z(), ys.z())) {
                *GetPixel(surface, Point2D(xs.z(), ys.z())) = mapped_color;
            }
            if (is_in_bounds(xs.w(), ys.w())) {
                *GetPixel(surface, Point2D(xs.w(), ys.w())) = mapped_color;
            }
        }
    }
}

void DrawLineSingle(SDL_Surface *surface, const Line2D &line, const Uint32 mapped_color) {
    // TODO: This is very sloppy, will write something real later
    Point2D p0 = line.p0;
    Point2D p1 = line.p1;
    
    const i32 dx = p1.x - p0.x;
    const i32 dy = p1.y - p0.y;
    
    if (abs(dx) > abs(dy)) {
        if (p0.x > p1.x) {
            std::swap(p0, p1);
        }
        
        const float slope = dy / (float) dx;
        float y = p0.y;
        for (u32 x = p0.x; x <= p1.x; ++x) {
            const Point2D point = Point2D(x, round(y));
            *GetPixel(surface, point) = mapped_color;
            y += slope;
        }
    } else {
        if (p0.y > p1.y) {
            std::swap(p0, p1);
        }
        
        const float slope = dx / (float) dy;
        float x = p0.x;
        for (i32 y = p0.y; y <= p1.y; ++y) {
            const Point2D point = Point2D(round(x), y);
            *GetPixel(surface, point) = mapped_color;
            x += slope;
        }
    }
}

void DrawMesh(ThreadPool& thread_pool, SDL_Surface *surface, const Mesh &mesh, bool wireframe) {
    for (int i = 0; i < mesh.indices.size(); i += 3) {
        const Vertex& v0 = mesh.vertices[mesh.indices[i]];
        const Vertex& v1 = mesh.vertices[mesh.indices[i + 1]];
        const Vertex& v2 = mesh.vertices[mesh.indices[i + 2]];
        const Triangle triangle = Triangle {
            v0,
            v1,
            v2
        };
#if SIMD
        if (!wireframe) {
            DrawTriangle(thread_pool, surface, triangle, mesh.texture);
        } else {
            const Color wireframe_color = Color{255, 0, 0};
            const auto mapped_color = SDL_MapRGB(surface->format, wireframe_color.red, wireframe_color.green, wireframe_color.blue);
            
            const Point2D sv0 = hacky_project_to_surface(surface, triangle.v0.p);
            const Point2D sv1 = hacky_project_to_surface(surface, triangle.v1.p);
            const Point2D sv2 = hacky_project_to_surface(surface, triangle.v2.p);

            const Line2D line0 = Line2D{sv0, sv1};
            const Line2D line1 = Line2D{sv1, sv2};
            const Line2D line2 = Line2D{sv2, sv0};

            DrawLine(surface, line0, mapped_color);
            DrawLine(surface, line1, mapped_color);
            DrawLine(surface, line2, mapped_color);
        }
#else
        if (!wireframe) {
            DrawTriangleSingle(thread_pool, surface, triangle, mesh.texture);
        } else {
            const Color wireframe_color = Color{255, 0, 0};
            const auto mapped_color = SDL_MapRGB(surface->format, wireframe_color.red, wireframe_color.green, wireframe_color.blue);
            
            const Point2D sv0 = hacky_project_to_surface(surface, triangle.v0.p);
            const Point2D sv1 = hacky_project_to_surface(surface, triangle.v1.p);
            const Point2D sv2 = hacky_project_to_surface(surface, triangle.v2.p);

            const Line2D line0 = Line2D{sv0, sv1};
            const Line2D line1 = Line2D{sv1, sv2};
            const Line2D line2 = Line2D{sv2, sv0};

            DrawLineSingle(surface, line0, mapped_color);
            DrawLineSingle(surface, line1, mapped_color);
            DrawLineSingle(surface, line2, mapped_color);
        }
#endif
    }
}

int main(int argc, char *argv[]) { 
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
    ThreadPool thread_pool;

    SDL_Surface *surface = SDL_GetWindowSurface(window);
    assert(surface->format->BytesPerPixel ==
           4);   // Not supporting non-32-bit pixel formats

    Texture texture = Texture("../assets/test.jpg", surface);

    const Mesh mesh = {
        {
            // Front-face
            Vertex {
                Point3D_f{-0.5, -0.5, 1.0},
                UV{0.0, 0.0}
            }, 
            Vertex {
                Point3D_f{0.5, -0.5, 1.0},
                UV{1.0, 0.0},

            },
            Vertex{
                Point3D_f{0.5, 0.5, 1.0},
                UV{1.0, 1.0},
            }, 
            Vertex{
                Point3D_f{-0.5, 0.5, 1.0},
                UV{0.0, 1.0},
            },
        },
        {
            0, 1, 2, 2, 3, 0,
        },
        texture
    };

    // Render loop
    // TODO: rotate stuff again
    const float rotate_delta = 0.03;
    float rotate_angle = 0.0;
    bool wireframe = false;
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
                            break;
                        }
                        case SDLK_f: {
                            wireframe = !wireframe;
                            break;
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
        
        DrawMesh(thread_pool, surface, mesh, wireframe);
        
        const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        printf("dt: %ld ms\n", (long) std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count());

        SDL_UnlockSurface(surface);
        SDL_UpdateWindowSurface(window);
    }

    // Cleanup
    SDL_DestroyWindow(window);
    SDL_Quit();
    texture.free();
    
    return 0;
}


// TODO: Use a model-view-proj system to convert from world-space to clip space
// TOOD: no-op thread pool (i.e single-threaded)