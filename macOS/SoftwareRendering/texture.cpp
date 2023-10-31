#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture::Texture(const char *path) {
    m_img = stbi_load(path, &m_width, &m_height, &m_channels, 0);
    if (m_img == NULL) {
        printf("Error in loading the image\n");
        exit(1);
    }
}

Color Texture::get_pixel_xy(u32 x, u32 y) const {
    const u32 stride = m_width * m_channels;
    const u32 start = y * stride + x * m_channels;
    const u8 red = m_img[start];
    const u8 green = m_img[start + 1];
    const u8 blue = m_img[start + 2];
    return Color{red, green, blue};
}

Color Texture::get_pixel_uv(float u, float v) const {
    return this->get_pixel_xy(round(u * m_width), round(v * m_height));
}
