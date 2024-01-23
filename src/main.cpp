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
#include "overdraw_indicator.h"
#include "ThreadPool.h"
#include "math/vec4f32.h"
#include "math/vec4i32.h"
#include "cmake_defs.h"

// TODO: Will bring back non-SIMD versions
#define SIMD true
#define SCREEN_WIDTH  1200
#define SCREEN_HEIGHT 800

namespace {

    enum class RenderingMethod : u8 {
        Standard,
        Wireframe,
        RasterMethod,
        Uv,
        Overdraw,
        Depth,
        Count
    };
    RenderingMethod render_method = RenderingMethod::Standard;

    const float rotate_delta = 0.1;
    float rotate_angle_x = 0.0;
    float rotate_angle_y = 0.0;
    float rotate_angle_z = 0.0;

    const float cutoff_area_delta = 2.5;
    float cuttof_area = 10.0;

    const float scale_delta = 0.5;
    float scale = 1.0;

    float edge_function(const Point2D& a, const Point2D& b, const Point2D& c) {
        return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
    };

    struct EdgeFunctionWeights {
        float w0;
        float w1;
        float w2;
    };

    EdgeFunctionWeights weights_at_point(const ScreenVertex* sv0, const ScreenVertex* sv1, const ScreenVertex* sv2, const Point2D& at) {
        const float ef0 = edge_function(sv1->p, sv2->p, at);
        const float ef1 = edge_function(sv2->p, sv0->p, at);
        const float ef2 = edge_function(sv0->p, sv1->p, at);
        const float sum = ef0 + ef1 + ef2;

        const float w0 = ef0 / sum;
        const float w1 = ef1 / sum;
        const float w2 = ef2 / sum;

        return EdgeFunctionWeights{
            w0,
            w1,
            w2
        };
    }

    float depth_for_weights(const ScreenVertex* sv0, const ScreenVertex* sv1, const ScreenVertex* sv2, const EdgeFunctionWeights& w) {
        return (sv0->depth * w.w0) + (sv1->depth * w.w1) + (sv2->depth * w.w2);
    }

    UV uv_for_weights(const ScreenVertex* sv0, const ScreenVertex* sv1, const ScreenVertex*  sv2, const EdgeFunctionWeights& w) {
        const float u = (sv0->uv.u * w.w0) + (sv1->uv.u * w.w1) + (sv2->uv.u * w.w2);
        const float v = (sv0->uv.v * w.w0) + (sv1->uv.v * w.w1) + (sv2->uv.v * w.w2);

        return UV{u, v};
    }
};

Vec4i32 GetPixelOffsets(SDL_Surface* surface, Vec4i32 xs, Vec4i32 ys) {
    return (Vec4i32(surface->h - 1) - ys) * Vec4i32(surface->pitch) + xs * Vec4i32(surface->format->BytesPerPixel);
}

u32 GetPixelOffset(SDL_Surface* surface, Point2D point) {
    return (surface->h - 1 - point.y) * surface->pitch + point.x * surface->format->BytesPerPixel;
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

void RenderPixels(SDL_Surface *surface, DepthBuffer& depth_buffer,  OverdrawBuffer& overdraw_buffer, Point2D &origin_point, Vec4i32 mask, Vec4f32 u, Vec4f32 v, Vec4f32 d, const Mesh& mesh, const Mesh::Material& material, bool barrycentric) {
    const auto get_overdraw_color = [&](u8 draw_count) {
        if (draw_count == 0) {
            return SDL_MapRGB(surface->format, 0, 255, 0);
        }
        else {
            return SDL_MapRGB(surface->format, 255, 0, 0);
        }
    };

    const Vec4i32 xs = Vec4i32(origin_point.x) + Vec4i32(0, 1, 2, 3);
    const Vec4i32 ys = Vec4i32(origin_point.y);
    const Vec4i32 pixel_offsets = GetPixelOffsets(surface, xs, ys);

    const Texture& texture = mesh.texture_map[material.texture_id];
    const Vec4i32 ui = (u.modf1() * texture.m_width).to_int_round_down();
    const Vec4i32 vi = (v.modf1() * texture.m_height).to_int_round_down();
    const Vec4i32 tex_idx = vi * Vec4i32(texture.m_width) + ui;

    const auto diffuse = SDL_MapRGB(surface->format, material.diffuse[0] * 255.0, material.diffuse[1] * 255.0, material.diffuse[2] * 255.0);

    for (int index = 0; index < 4; ++index) {
        const auto curr_depth = d[index];
        if (mask[index] && 
            xs[index] >= 0 && ys[index] >= 0 && 
            curr_depth > depth_buffer.ValueAt(xs[index], ys[index])) 
        {
            switch (render_method) {
                case RenderingMethod::Standard: {
                    if (material.texture_id >= 0) {
                        *GetPixel(surface, pixel_offsets[index]) = texture.get_pixel_from_idx(tex_idx[index]);
                    } else {
                        *GetPixel(surface, pixel_offsets[index]) = diffuse;
                    }
                    break;
                }
                case RenderingMethod::Uv: {
                    *GetPixel(surface, pixel_offsets[index]) = SDL_MapRGB(surface->format, (u8)(u[index]* 255.0), (u8)(v[index] * 255.0), 0);
                    break;
                }
                case RenderingMethod::RasterMethod: {
                    // Note: The invariants for debug drawing methods are not calculated in advance to save on compute time for the main path
                    const auto red = SDL_MapRGB(surface->format, 255, 0, 0);
                    const auto green = SDL_MapRGB(surface->format, 0, 255, 0);

                    *GetPixel(surface, pixel_offsets[index]) = barrycentric ? red : green;
                    break;
                }
                case RenderingMethod::Overdraw: {
                    *GetPixel(surface, pixel_offsets[index]) = get_overdraw_color(overdraw_buffer.ValueAt(xs[index], ys[index]));
                    overdraw_buffer.Increase(xs[index], ys[index]);
                    break;
                }
                case RenderingMethod::Depth: {
                    const u8 depth_greyscale = (u8) std::min(255.0, curr_depth * 255.0);
                    *GetPixel(surface, pixel_offsets[index]) = SDL_MapRGB(surface->format, depth_greyscale, depth_greyscale, depth_greyscale);
                    break;
                }
                case RenderingMethod::Wireframe: {
                    // NOTE: Handled separately
                    break;
                }
            }
            depth_buffer.Write(xs[index], ys[index], d[index]);
        }
    }
}

void FillBottomFlatTriangle(SDL_Surface* surface, DepthBuffer& depth_buffer, OverdrawBuffer& overdraw_buffer, Rect2D bounding_box, const ScreenVertex* v0, const ScreenVertex* v1, const ScreenVertex* v2, const Mesh& mesh, const Mesh::Material& material) {
    assert(v1->p.y == v2->p.y);
    assert(v0->p.y < v1->p.y);

    const float invslope2 = (float) (v2->p.x - v0->p.x) / (v2->p.y - v0->p.y);
    const float invslope1 = (float) (v1->p.x - v0->p.x) / (v1->p.y - v0->p.y);

    const float min_slope = MIN(invslope1, invslope2);
    const float max_slope = MAX(invslope1, invslope2);

    const float start_x_min = v0->p.x;
    const float start_x_max = v0->p.x;

    const int x_min = MAX(MIN3(v0->p.x, v1->p.x, v2->p.x), 0);

    const int y_min = MAX(v0->p.y, bounding_box.minY);
    const int y_max = MIN(v2->p.y, bounding_box.maxY - 1);

    float start_y_offset = y_min - v0->p.y;
    float curr_x_min = start_x_min + min_slope * start_y_offset;
    float curr_x_max = start_x_min + max_slope * start_y_offset;

    EdgeFunction e01, e12, e20;
    const Point2D p_start = Point2D{ x_min, y_min };
    Vec4i32 w0_row = e12.Init(v1->p, v2->p, p_start);
    Vec4i32 w1_row = e20.Init(v2->p, v0->p, p_start);
    Vec4i32 w2_row = e01.Init(v0->p, v1->p, p_start);

    Point2D p = {};
    for (p.y = y_min; p.y <= y_max; p.y++) {
        const Vec4i32 ys = Vec4i32(p.y);

        const int delta_x = (curr_x_min - x_min) / EdgeFunction::step_increment_x;
        Vec4i32 w0 = w0_row + e12.step_size_x * delta_x;
        Vec4i32 w1 = w1_row + e20.step_size_x * delta_x;
        Vec4i32 w2 = w2_row + e01.step_size_x * delta_x;

        for (p.x = x_min + delta_x * EdgeFunction::step_increment_x; p.x <= MIN(curr_x_max, bounding_box.maxX - 1); p.x += EdgeFunction::step_increment_x) {
            // TODO: Maybe a way to optimize draws on the bounds
            if (p.x + EdgeFunction::step_increment_x >= 0) {
                const Vec4i32 mask = w0 | w1 | w2;

                const Vec4f32 sum = (w0 + w1 + w2).to_float();

                const Vec4f32 b0 = w0.to_float() / sum;
                const Vec4f32 b1 = w1.to_float() / sum;
                const Vec4f32 b2 = w2.to_float() / sum;

                const Vec4f32 u_0 = Vec4f32(v0->uv.u) * b0, v_0 = Vec4f32(v0->uv.v) * b0;
                const Vec4f32 u_1 = Vec4f32(v1->uv.u) * b1, v_1 = Vec4f32(v1->uv.v) * b1;
                const Vec4f32 u_2 = Vec4f32(v2->uv.u) * b2, v_2 = Vec4f32(v2->uv.v) * b2;

                const Vec4f32 u = u_0 + u_1 + u_2;
                const Vec4f32 v = v_0 + v_1 + v_2;

                const Vec4f32 d_0 = Vec4f32(v0->depth) * b0;
                const Vec4f32 d_1 = Vec4f32(v1->depth) * b1;
                const Vec4f32 d_2 = Vec4f32(v2->depth) * b2;
                const Vec4f32 d = d_0 + d_1 + d_2;

                RenderPixels(surface, depth_buffer, overdraw_buffer, p, mask, u, v, d, mesh, material, false);
            }
            w0 += e12.step_size_x;
            w1 += e20.step_size_x;
            w2 += e01.step_size_x;
        }

        curr_x_min += min_slope;
        curr_x_max += max_slope;

        w0_row += e12.step_size_y;
        w1_row += e20.step_size_y;
        w2_row += e01.step_size_y;
    }
}

void FillTopFlatTriangle(SDL_Surface* surface, DepthBuffer& depth_buffer, OverdrawBuffer& overdraw_buffer, Rect2D bounding_box, const ScreenVertex* v0, const ScreenVertex* v1, const ScreenVertex* v2, const Mesh& mesh, const Mesh::Material& material) {
    assert(v0->p.y == v1->p.y);
    assert(v2->p.y > v1->p.y);

    float invslope1 = (float) (v2->p.x - v0->p.x) / (v2->p.y - v0->p.y);
    float invslope2 = (float) (v2->p.x - v1->p.x) / (v2->p.y - v1->p.y);

    const float min_slope = MIN(invslope1, invslope2);
    const float max_slope = MAX(invslope1, invslope2);

    const float start_x_min = v2->p.x;
    const float start_x_max = v2->p.x;

    const int x_min = MAX(MIN3(v0->p.x, v1->p.x, v2->p.x), 0);

    const int y_min = MAX(v0->p.y, bounding_box.minY);
    const int y_max = MIN(v2->p.y, bounding_box.maxY - 1);

    float start_y_offset = v2->p.y - y_max;
    float curr_x_min = start_x_min - max_slope * start_y_offset;
    float curr_x_max = start_x_min - min_slope * start_y_offset;

    EdgeFunction e01, e12, e20;
    const Point2D p_start = Point2D{ x_min, y_max };
    Vec4i32 w0_row = e12.Init(v1->p, v2->p, p_start);
    Vec4i32 w1_row = e20.Init(v2->p, v0->p, p_start);
    Vec4i32 w2_row = e01.Init(v0->p, v1->p, p_start);

    Point2D p = {};
    for (p.y = y_max; p.y >= y_min; p.y--) {
        const Vec4i32 ys = Vec4i32(p.y);

        const int delta_x = (curr_x_min - x_min) / EdgeFunction::step_increment_x;
        Vec4i32 w0 = w0_row + e12.step_size_x * delta_x;
        Vec4i32 w1 = w1_row + e20.step_size_x * delta_x;
        Vec4i32 w2 = w2_row + e01.step_size_x * delta_x;

        for (p.x = x_min + delta_x * EdgeFunction::step_increment_x; p.x <= MIN(curr_x_max, bounding_box.maxX - 1); p.x += EdgeFunction::step_increment_x) {
            if (p.x + EdgeFunction::step_increment_x >= 0) {
                const Vec4i32 mask = w0 | w1 | w2;

                const Vec4f32 sum = (w0 + w1 + w2).to_float();

                const Vec4f32 b0 = w0.to_float() / sum;
                const Vec4f32 b1 = w1.to_float() / sum;
                const Vec4f32 b2 = w2.to_float() / sum;

                const Vec4f32 u_0 = Vec4f32(v0->uv.u) * b0, v_0 = Vec4f32(v0->uv.v) * b0;
                const Vec4f32 u_1 = Vec4f32(v1->uv.u) * b1, v_1 = Vec4f32(v1->uv.v) * b1;
                const Vec4f32 u_2 = Vec4f32(v2->uv.u) * b2, v_2 = Vec4f32(v2->uv.v) * b2;

                const Vec4f32 u = u_0 + u_1 + u_2;
                const Vec4f32 v = v_0 + v_1 + v_2;

                const Vec4f32 d_0 = Vec4f32(v0->depth) * b0;
                const Vec4f32 d_1 = Vec4f32(v1->depth) * b1;
                const Vec4f32 d_2 = Vec4f32(v2->depth) * b2;
                const Vec4f32 d = d_0 + d_1 + d_2;

                RenderPixels(surface, depth_buffer, overdraw_buffer, p, mask, u, v, d, mesh, material, false);
            }
            w0 += e12.step_size_x;
            w1 += e20.step_size_x;
            w2 += e01.step_size_x;
        }

        curr_x_min -= max_slope;
        curr_x_max -= min_slope;

        w0_row -= e12.step_size_y;
        w1_row -= e20.step_size_y;
        w2_row -= e01.step_size_y;
    }
}

void DrawTriangleBarycentric(SDL_Surface* surface, Rect2D tile_rect, Rect2D bounding_box, DepthBuffer& depth_buffer, OverdrawBuffer& overdraw_buffer, const ScreenTriangle& st, const Mesh& mesh, const Mesh::Material& material) {
    const Point2D min_point = Point2D{ bounding_box.minX, bounding_box.minY };

    const float c0_u = st.v0.uv.u, c0_v = st.v0.uv.v;
    const float c1_u = st.v1.uv.u, c1_v = st.v1.uv.v;
    const float c2_u = st.v2.uv.u, c2_v = st.v2.uv.v;

    const float c0_d = st.v0.depth;
    const float c1_d = st.v1.depth;
    const float c2_d = st.v2.depth;

    EdgeFunction e01, e12, e20;
    Vec4i32 w0_row = e12.Init(st.v1.p, st.v2.p, min_point);
    Vec4i32 w1_row = e20.Init(st.v2.p, st.v0.p, min_point);
    Vec4i32 w2_row = e01.Init(st.v0.p, st.v1.p, min_point);

    Point2D point = { 0, 0 };
    for (point.y = bounding_box.minY; point.y <= MIN(bounding_box.maxY, surface->h - 1); point.y += EdgeFunction::step_increment_y) {
        const Vec4i32 delta_y = Vec4i32(point.y - bounding_box.minY);

        Vec4i32 w0 = w0_row;
        Vec4i32 w1 = w1_row;
        Vec4i32 w2 = w2_row;

        bool is_in_triangle = false;
        for (point.x = bounding_box.minX; point.x <= MIN(bounding_box.maxX, surface->w - 1); point.x += EdgeFunction::step_increment_x) {
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

                RenderPixels(surface, depth_buffer, overdraw_buffer, point, mask, u, v, d, mesh, material, true);
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
}

void DrawTriangleScanline(SDL_Surface* surface, Rect2D tile_rect, Rect2D bounding_box, DepthBuffer& depth_buffer, OverdrawBuffer& overdraw_buffer, const ScreenTriangle& st, const Mesh& mesh, const Mesh::Material& material) {
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
        FillBottomFlatTriangle(surface, depth_buffer, overdraw_buffer, bounding_box, sv0, sv1, sv2, mesh, material);
    }
    else if (sv0->p.y == sv1->p.y) {
        FillTopFlatTriangle(surface, depth_buffer, overdraw_buffer, bounding_box, sv0, sv1, sv2, mesh, material);
    }
    else {
        const Point2D p3 = Point2D{
            (int)(sv0->p.x + ((float)(sv1->p.y - sv0->p.y) / (float)(sv2->p.y - sv0->p.y)) * (sv2->p.x - sv0->p.x)),
            sv1->p.y
        };
        const EdgeFunctionWeights ws = weights_at_point(sv0, sv1, sv2, p3);
        const ScreenVertex sv3 = ScreenVertex{
            p3,
            depth_for_weights(sv0, sv1, sv2, ws),
            uv_for_weights(sv0, sv1, sv2, ws)
        };

        FillBottomFlatTriangle(surface, depth_buffer, overdraw_buffer, bounding_box, sv0, sv1, &sv3, mesh, material);
        FillTopFlatTriangle(surface, depth_buffer, overdraw_buffer, bounding_box, sv1, &sv3, sv2, mesh, material);
    }
}

void DrawTriangle(SDL_Surface* surface, Rect2D tile_rect, Rect2D bounding_box, DepthBuffer& depth_buffer, OverdrawBuffer& overdraw_buffer, const ScreenTriangle& st, const Mesh& mesh, const Mesh::Material& material) {
    const float double_area = edge_function(st.v0.p, st.v1.p, st.v2.p);
    if (double_area <= 0.0) {
        return;
    }


    if (double_area < cuttof_area) {
        DrawTriangleBarycentric(surface, tile_rect, bounding_box, depth_buffer, overdraw_buffer, st, mesh, material);
    }
    else {
        DrawTriangleScanline(surface, tile_rect, bounding_box, depth_buffer, overdraw_buffer, st, mesh, material);
    }
}

// NOTE: I don't think this SIMD version is really worth it tbh,
// but I wanted to do it for completeness sake
void DrawLine(SDL_Surface* surface, const Line2D& line, const Uint32 mapped_color) {
    // TODO: This is very sloppy, will write something real later
    // Since we know the slope, we can be smarter about breaking out early when we go out of bounds
    // Maybe the testing can be SIMD too

    const auto is_in_bounds = [=](i32 x, i32 y) {
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

        const i32 x_start = MAX(p0.x, 0);
        if (x_start < 0) {
            const auto test = 0;
        }
        const i32 x_end = MIN(p1.x, surface->w);
        for (i32 x = x_start; x <= x_end; x += 4) {
            const i32 x_delta = x - p0.x;

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

        const i32 y_start = MAX(p0.y, 0);
        if (y_start < 0) {
            const auto test = 0;
        }
        const i32 y_end = MIN(p1.y, surface->h);
        for (i32 y = y_start; y <= y_end; y += 4) {
            const i32 y_delta = y - p0.y;

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

void DrawMesh(SDL_Surface *surface, const Mat4f32& proj_model, ThreadPool &thread_pool, ScreenTileData tile_data, DepthBuffer& depth_buffer, OverdrawBuffer& overdraw_buffer, Mesh &mesh) {
    // TODO: Will need to modify this logic to take care of the material index stuff
    TriangleTileMap triangle_tile_map = mesh.SetupScreenTriangles(surface, tile_data, proj_model);
    const u32 num_tasks = tile_data.num_tasks();
    for (auto const& item : triangle_tile_map) {
        auto const tile_index = item.first;
        auto const tile_value = item.second;
        thread_pool.Schedule([=]() mutable {
            for (const TriangleTileValueInner& val : tile_value.values) {
                if (render_method != RenderingMethod::Wireframe) {
                    // TODO: Don' need to pass mesh and material
                    const int material_id = mesh.screen_triangles[val.index].material_id;
                    const Mesh::Material& material = mesh.materials[material_id];
                    DrawTriangle(surface, tile_value.tile_rect, val.bounding_box, depth_buffer, overdraw_buffer, mesh.screen_triangles[val.index], mesh, material);
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
    OverdrawBuffer overdraw_buffer = OverdrawBuffer(surface->w, surface->h);

    const std::string mesh_path = std::string(PROJECT_ROOT) + std::string("/assets/meshes/teapot");
    Mesh mesh = load_obj(mesh_path, "teapot.obj", surface);

    const float depth_min = 0.0;
    const float x_span = mesh.bb.maxX - mesh.bb.minX;
    const float y_span = mesh.bb.maxY - mesh.bb.minY;
    const float span_padding = 0.1f;
    const Camera camera = Camera{OrtographicData{
        mesh.bb.minX - x_span * span_padding,
        mesh.bb.maxX + x_span * span_padding,
        mesh.bb.minY - y_span * span_padding,
        mesh.bb.maxY + y_span * span_padding,
        -10.0,
        1.0
    }};

    // TODO: This will need to be re-done when resizing
    const ScreenTileData tile_data = partition_screen_into_tiles(surface);

    // Render loop
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
                        //
                        case SDLK_EQUALS: {
                            render_method = static_cast<RenderingMethod>((static_cast<u8>(render_method) + 1) % static_cast<u8>(RenderingMethod::Count));
                            break;
                        }
                        case SDLK_MINUS: {
                            render_method = static_cast<RenderingMethod>((static_cast<u8>(render_method) - 1) % static_cast<u8>(RenderingMethod::Count));
                            break;
                        }
                        //
                        case SDLK_LEFTBRACKET: {
                            cuttof_area -= cutoff_area_delta;
                            break;
                        }
                        case SDLK_RIGHTBRACKET: {
                            cuttof_area += cutoff_area_delta;
                            break;
                        }
                        //
                        case SDLK_9: {
                            scale -= scale_delta;
                            break;
                        }
                        case SDLK_0: {
                            scale += scale_delta;
                            break;
                        }
                        //
                        case SDLK_q: {
                            rotate_angle_x += rotate_delta;
                            break;
                        }
                        case SDLK_w: {
                            rotate_angle_y += rotate_delta;
                            break;
                        }
                        case SDLK_e: {
                            rotate_angle_z += rotate_delta;
                            break;
                        }
                        //
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
        // TODO: Can optimize this with a single matrix, will do later
        const Mat4f32 rotations = rotate_matrix(Vec3D_f{ 1.0, 0.0, 0.0 }, rotate_angle_x) *
            rotate_matrix(Vec3D_f{ 0.0, 1.0, 0.0 }, rotate_angle_y) *
            rotate_matrix(Vec3D_f{ 0.0, 0.0, 1.0 }, rotate_angle_z);
        const Mat4f32 model_matrix = rotations * uniform_scale_matrix(scale);
        const Mat4f32 proj_model = proj_matrix * model_matrix;

        const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
        
        ClearSurface(surface, thread_pool, Color{100, 100, 100});
        depth_buffer.Set(depth_min);
        DrawMesh(surface, proj_model, thread_pool, tile_data, depth_buffer, overdraw_buffer, mesh);
        overdraw_buffer.Clear();

        const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
        printf("dt: %ld ms\n", (long) std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count());

        SDL_UnlockSurface(surface);
        SDL_UpdateWindowSurface(window);
    }

    // Cleanup
    SDL_DestroyWindow(window);
    SDL_Quit();
    // TODO: Free Mesh
    
    return 0;
}
