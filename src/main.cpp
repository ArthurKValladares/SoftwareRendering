#include <assert.h>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <stdbool.h>
#include <stdio.h>
#include <vector>
#include <algorithm>

#include "point.h"
#include "texture.h"
#include "color.h"
#include "rect.h"
#include "defs.h"
#include "triangle.h"
#include "edge_function.h"
#include "uv.h"
#include "mesh/mesh.h"
#include "mesh/obj.h"
#include "line.h"
#include "camera.h"
#include "transform.h"
#include "depth_buffer.h"
#include "ThreadPool.h"
#include "math/vec4f32.h"
#include "math/vec4i32.h"

// TODO: Will bring back non-SIMD versions
#define SIMD true
#define SCREEN_WIDTH  1200
#define SCREEN_HEIGHT 800

namespace {
    bool wireframe = false;
    float rotate_angle = 0.0;

    float edge_function(const Point2D& a, const Point2D& b, const Point2D& c) {
        return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    };
};

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

void ClearSurface(SDL_Surface *surface, ThreadPool& thread_pool, Color color) {
    const Uint32 pixel_color =
        SDL_MapRGB(surface->format, color.red, color.green, color.blue);
    const Vec4i32 pixel_colors = Vec4i32(pixel_color);

    const int width = surface->w;
    const int height = surface->h;

    const int num_threads = std::thread::hardware_concurrency();
    const int rows_per_thread = height / num_threads;

    for (int i = 0; i < num_threads; ++i) {
        const int min_y = i * rows_per_thread;
        const int max_y = i == (num_threads - 1)
            ? height
            : (i + 1) * rows_per_thread;
        thread_pool.Schedule([=] {
            for (int y = min_y; y < max_y; ++y) {
                Point2D point = Point2D{ 0, y };
                for (point.x = 0; point.x < width; point.x += EdgeFunction::step_increment_x) {
                    *(Vec4i32*)GetPixel(surface, point) = pixel_colors;
                }
            }
        });
    }
    thread_pool.Wait();
}

void RenderPixels(SDL_Surface *surface, DepthBuffer& depth_buffer, const Point2D &origin_point, Vec4i32 mask, Vec4f32 u, Vec4f32 v, Vec4f32 d, const Texture &texture) {
    const Vec4i32 ui = (u.clamp(0.0, 1.0) * texture.m_width).to_int_nearest();
    const Vec4i32 vi = (v.clamp(0.0, 1.0) * texture.m_height).to_int_nearest();
    const Vec4i32 tex_idx = vi * Vec4i32(texture.m_width) + ui;

    const Vec4i32 xs = Vec4i32(origin_point.x) + Vec4i32(0, 1, 2, 3);
    const Vec4i32 ys = Vec4i32(origin_point.y);

    const Vec4i32 pixel_offsets = GetPixelOffsets(surface, xs, ys);

    if (mask.x() && d.x() > depth_buffer.ValueAt(xs.x(), ys.x())) {
        *GetPixel(surface, pixel_offsets.x()) = texture.get_pixel_from_idx(tex_idx.x());
        depth_buffer.Write(xs.x(), ys.x(), d.x());
    }
    if (mask.y() && d.y() > depth_buffer.ValueAt(xs.y(), ys.y())) {
        *GetPixel(surface, pixel_offsets.y()) = texture.get_pixel_from_idx(tex_idx.y());
        depth_buffer.Write(xs.y(), ys.y(), d.y());
    }
    if (mask.z() && d.z() > depth_buffer.ValueAt(xs.z(), ys.z())) {
        *GetPixel(surface, pixel_offsets.z()) = texture.get_pixel_from_idx(tex_idx.z());
        depth_buffer.Write(xs.z(), ys.z(), d.z());
    }
    if (mask.w() && d.w() > depth_buffer.ValueAt(xs.w(), ys.w())) {
        *GetPixel(surface, pixel_offsets.w()) = texture.get_pixel_from_idx(tex_idx.w());
        depth_buffer.Write(xs.w(), ys.w(), d.w());
    }
}

void FillBottomFlatTriangle(SDL_Surface* surface, const ScreenVertex* v0, const ScreenVertex* v1, const ScreenVertex* v2) {
    assert(v1->p.y == v2->p.y);
    assert(v0->p.y < v1->p.y);

    const float invslope2 = (float) (v2->p.x - v0->p.x) / (v2->p.y - v0->p.y);
    const float invslope1 = (float) (v1->p.x - v0->p.x) / (v1->p.y - v0->p.y);

    const float min_slope = MIN(invslope1, invslope2);
    const float max_slope = MAX(invslope1, invslope2);

    float curr_x_min = v0->p.x;
    float curr_x_max = v0->p.x;

    const Uint32 mapped_color = SDL_MapRGB(surface->format, 255, 0, 0);
    for (int scanlineY = v0->p.y; scanlineY <= v1->p.y; scanlineY++) {
        for (int x = curr_x_min; x <= curr_x_max; x += EdgeFunction::step_increment_x) {
            if (x >= surface->w || scanlineY >= surface->h) {
                break;
            }

            const Vec4i32 xs = Vec4i32(x) + Vec4i32(0, 1, 2, 3);
            const Vec4i32 ys = Vec4i32(scanlineY);
            const Vec4i32 pixel_offsets = GetPixelOffsets(surface, xs, ys);

            *GetPixel(surface, pixel_offsets.x()) = mapped_color;
            if (xs.y() <= curr_x_max)
                *GetPixel(surface, pixel_offsets.y()) = mapped_color;
            if (xs.z() <= curr_x_max)
                *GetPixel(surface, pixel_offsets.z()) = mapped_color;
            if (xs.w() <= curr_x_max)
                *GetPixel(surface, pixel_offsets.w()) = mapped_color;
        }
        curr_x_min += min_slope;
        curr_x_max += max_slope;
    }
}

void FillTopFlatTriangle(SDL_Surface* surface, const ScreenVertex* v0, const ScreenVertex* v1, const ScreenVertex* v2) {
    assert(v0->p.y == v1->p.y);
    assert(v2->p.y > v1->p.y);

    float invslope1 = (float) (v2->p.x - v0->p.x) / (v2->p.y - v0->p.y);
    float invslope2 = (float) (v2->p.x - v1->p.x) / (v2->p.y - v1->p.y);

    const float min_slope = MIN(invslope1, invslope2);
    const float max_slope = MAX(invslope1, invslope2);

    float curr_x_min = v2->p.x;
    float curr_x_max = v2->p.x;

    const Uint32 mapped_color = SDL_MapRGB(surface->format, 0, 255, 0);
    for (int scanlineY = v2->p.y; scanlineY > v0->p.y; scanlineY--)
    {
        for (int x = curr_x_min; x <= curr_x_max; x += EdgeFunction::step_increment_x) {
            if (x >= surface->w || scanlineY >= surface->h) {
                break;
            }
            
            const Vec4i32 xs = Vec4i32(x) + Vec4i32(0, 1, 2, 3);
            const Vec4i32 ys = Vec4i32(scanlineY);
            const Vec4i32 pixel_offsets = GetPixelOffsets(surface, xs, ys);

            *GetPixel(surface, pixel_offsets.x()) = mapped_color;
            if (xs.y() <= curr_x_max)
                *GetPixel(surface, pixel_offsets.y()) = mapped_color;
            if (xs.z() <= curr_x_max)
                *GetPixel(surface, pixel_offsets.z()) = mapped_color;
            if (xs.w() <= curr_x_max)
                *GetPixel(surface, pixel_offsets.w()) = mapped_color;
        }
        curr_x_min -= max_slope;
        curr_x_max -= min_slope;
    }
}

void DrawTriangle(SDL_Surface* surface, Rect2D tile_rect, Rect2D bounding_box, DepthBuffer& depth_buffer, const ScreenTriangle& st, const Texture& texture) {
    ScreenVertex const* sv0 = &st.v0;
    ScreenVertex const* sv1 = &st.v1;
    ScreenVertex const* sv2 = &st.v2;
    if (sv1->p.y < sv0->p.y) {
        std::swap(sv1, sv0);
    }
    if (sv2->p.y < sv0->p.y) {
        std::swap(sv2, sv0);
    }
    if (sv2->p.y < sv1->p.y) {
        std::swap(sv2, sv1);
    }

    if (sv2->p.y == sv1->p.y) {
        FillBottomFlatTriangle(surface, sv0, sv1, sv2);
    }
    else if (sv0->p.y == sv1->p.y) {
        FillTopFlatTriangle(surface, sv0, sv1, sv2);
    }
    else {
        const Point2D p3 = Point2D{
            (int)(sv0->p.x + ((float)(sv1->p.y - sv0->p.y) / (float)(sv2->p.y - sv0->p.y)) * (sv2->p.x - sv0->p.x)), 
            sv1->p.y
        };
        const ScreenVertex sv3 = ScreenVertex{
            p3,
            UV{} // TODO: Gotta figure out UV here
        };
        FillBottomFlatTriangle(surface, sv0, sv1, &sv3);
        FillTopFlatTriangle(surface, sv1, &sv3, sv2);
    }
    /*
    // Early return if triangle has zero area
    if (edge_function(st.p0, st.p1, st.p2) == 0.0) {
        return;
    }

    const Point2D min_point = Point2D{bounding_box.minX, bounding_box.minY};
    
    const float c0_u = triangle.v0.uv.u, c0_v = triangle.v0.uv.v;
    const float c1_u = triangle.v1.uv.u, c1_v = triangle.v1.uv.v;
    const float c2_u = triangle.v2.uv.u, c2_v = triangle.v2.uv.v;
    
    const float c0_d = triangle.v0.p.z;
    const float c1_d = triangle.v1.p.z;
    const float c2_d = triangle.v2.p.z;

    EdgeFunction e01, e12, e20;
    Vec4i32 w0_row = e12.Init(st.p1, st.p2, min_point);
    Vec4i32 w1_row = e20.Init(st.p2, st.p0, min_point);
    Vec4i32 w2_row = e01.Init(st.p0, st.p1, min_point);
    
    Point2D point = { 0, 0 };
    for (point.y = bounding_box.minY; point.y <= bounding_box.maxY; point.y += EdgeFunction::step_increment_y) {
        const Vec4i32 delta_y = Vec4i32(point.y - bounding_box.minY);
        
        Vec4i32 w0 = w0_row;
        Vec4i32 w1 = w1_row;
        Vec4i32 w2 = w2_row;
            
        bool is_in_triangle = false;
        for (point.x = bounding_box.minX; point.x <= bounding_box.maxX; point.x += EdgeFunction::step_increment_x) {
            const Vec4i32 mask = w0 | w1 | w2;
                
            if (mask.any_gte(0)) {
                is_in_triangle = true;

                const Vec4f32 sum = (w0 + w1 + w2).to_float();
                    
                const Vec4f32 b0 = w0.to_float() / sum;
                const Vec4f32 b1 = w1.to_float() / sum;
                const Vec4f32 b2 = w2.to_float() / sum;
                    
                const Vec4f32 u_0 = Vec4f32(c0_u) * b0, v_0 = Vec4f32(c0_v) * b0;
                const Vec4f32 u_1 = Vec4f32(c1_u) * b1, v_1 = Vec4f32(c1_v) * b1;
                const Vec4f32 u_2 = Vec4f32(c2_u) * b2, v_2 = Vec4f32(c2_v) * b2;
                    
                const Vec4f32 u = u_0 + u_1 + u_2;
                const Vec4f32 v = v_0 + v_1 + v_2;
                    
                // TODO: This is probably not right?
                const Vec4f32 d_0 = Vec4f32(c0_d) * b0;
                const Vec4f32 d_1 = Vec4f32(c1_d) * b1;
                const Vec4f32 d_2 = Vec4f32(c2_d) * b2;
                const Vec4f32 d = d_0 + d_1 + d_2;

                RenderPixels(surface, depth_buffer, point, mask, u, v, d, texture);
            }
            else if (is_in_triangle) {
                // Since we are drawing triangles, we can never go in and out of the shape in the same line
                break;
            }
                
            w0 += e12.step_size_x;
            w1 += e20.step_size_x;
            w2 += e01.step_size_x;
        }

        w0_row += e12.step_size_y;
        w1_row += e20.step_size_y;
        w2_row += e01.step_size_y;
    }
    */
}

// NOTE: I don't think this SIMD version is really worth it tbh,
// but I wanted to do it for completeness sake
void DrawLine(SDL_Surface* surface, const Line2D& line, const Uint32 mapped_color) {
    // TODO: This is very sloppy, will write something real later
    // Since we know the slope, we can be smarter about breaking out early when we go out of bounds
    // Maybe the testing can be SIMD too

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
        const float slope = dy / (float)dx;

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
    }
    else {
        if (p0.y > p1.y) {
            std::swap(p0, p1);
        }
        const float slope = dx / (float)dy;

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

void DrawTriangleWireframe(SDL_Surface* surface, const ScreenTriangle& st) {
    const Line2D line0 = Line2D{ st.v0.p, st.v1.p };
    const Line2D line1 = Line2D{ st.v1.p, st.v2.p };
    const Line2D line2 = Line2D{ st.v2.p, st.v0.p };

    const Color wireframe_color = Color{ 255, 0, 0 };
    const auto mapped_color = SDL_MapRGB(surface->format, wireframe_color.red, wireframe_color.green, wireframe_color.blue);

    DrawLine(surface, line0, mapped_color);
    DrawLine(surface, line1, mapped_color);
    DrawLine(surface, line2, mapped_color);
}

void DrawMesh(SDL_Surface *surface, const Mat4f32& proj_model, ThreadPool &thread_pool, ScreenTileData tile_data, DepthBuffer& depth_buffer, Mesh &mesh, const Texture &texture) {
    TriangleTileMap triangle_tile_map = mesh.SetupScreenTriangles(surface, tile_data, proj_model);

    const u32 num_tasks = tile_data.num_tasks();
    for (auto const& [tile_index, tile_value] : triangle_tile_map) {
        thread_pool.Schedule([=]() mutable {
            for (const TriangleTileValueInner& val : tile_value.values) {
                if (!wireframe) {
                    DrawTriangle(surface, tile_value.tile_rect, val.bounding_box, depth_buffer, mesh.screen_triangles[val.index], texture);
                }
                else {
                    DrawTriangleWireframe(surface, mesh.screen_triangles[val.index]);
                }
            }
        });
    }

    thread_pool.Wait();
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
    // Not supporting non-32-bit pixel formats
    assert(surface->format->BytesPerPixel == 4);
    DepthBuffer depth_buffer = DepthBuffer(surface->w, surface->h);

    Texture texture = Texture("../assets/meshes/teapot/default.png", surface);
    Mesh mesh = load_obj("../assets/meshes/teapot", "teapot.obj");

    const Camera camera = Camera::orthographic(OrtographicCamera{
        -100.0,
        100.0,
        -100.0,
        100.0,
        0.0,
        1.0
    });

    // TODO: This will need to be re-done when resizing
    const ScreenTileData tile_data = partition_screen_into_tiles(surface);

    // Render loop
    // TODO: rotate stuff again
    const float rotate_delta = 0.1;
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
        
        const Mat4f32 proj_matrix = camera.GetProjMatrix();
        const Mat4f32 model_matrix = rotate_matrix(Vec3D_f{ 1.0, 0.0, 0 }, rotate_angle);
        const Mat4f32 proj_model = proj_matrix * model_matrix;

        const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        
        depth_buffer.Clear();
        ClearSurface(surface, thread_pool, Color{100, 100, 100});
        DrawMesh(surface, proj_model, thread_pool, tile_data, depth_buffer, mesh, texture);
        
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

// TOOD: Optimize how many pixels we try to draw per triangle