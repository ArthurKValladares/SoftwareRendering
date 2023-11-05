#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture(const char *path) {
    m_img = stbi_load(path, &m_width, &m_height, &m_channels, 0);
    m_stride = m_width * m_channels;
    if (m_img == NULL) {
        printf("Error in loading the image\n");
        exit(1);
    }
}

Color Texture::get_pixel_from_idx(u32 start) const {
    const u8 red = m_img[start];
    const u8 green = m_img[start + 1];
    const u8 blue = m_img[start + 2];
    return Color{red, green, blue};
}

Color Texture::get_pixel_xy(u32 x, u32 y) const {
    const u32 start = y * m_stride + x * m_channels;
    return this->get_pixel_from_idx(start);
}

float clamp(float v, float lo, float hi) {
    const float t = v < lo
        ? lo
        : v;
    return t > hi
        ? hi
        : t;
}

Color Texture::get_pixel_uv(float u, float v) const {
    // TODO: Some `sampler` like stuff for different sampling methods when outside range
    float c_u = clamp(u, 0.0, 1.0);
    float c_v = clamp(v, 0.0, 1.0);
    return this->get_pixel_xy(round(c_u * m_width), round(c_v * m_height));
}
