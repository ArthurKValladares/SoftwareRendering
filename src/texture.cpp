#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <filesystem>
#include <iostream>

Texture::Texture(const std::string& path, SDL_Surface *surface) {
    int channels;
    unsigned char *img = stbi_load(path.c_str(), &m_width, &m_height, &channels, 0);
    if (img == NULL) {
        printf("Error in loading the image: %s\n", path.c_str());
        if(!std::filesystem::exists(path)) {
            printf("Path does not exist.");
        }
        exit(1);
    }
    const int stride = m_width * channels;
    const int total_size = m_width * m_height;
    m_img = new Uint32[total_size];
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            const u32 start = y * stride + x * channels;
            const u8 red = img[start];
            const u8 green =img[start + 1];
            const u8 blue = img[start + 2];
            m_img[y * m_width + x] = SDL_MapRGB(surface->format, red, green, blue);
        }
    }
    stbi_image_free(img);
}

void Texture::free() {
    delete[] m_img;
}

Uint32 Texture::get_pixel_from_idx(u32 idx) const {
    return m_img[idx];
}

Uint32 Texture::get_pixel_xy(u32 x, u32 y) const {
    const u32 idx = y * m_width + x;
    return this->get_pixel_from_idx(idx);
}

Uint32 Texture::get_pixel_uv(float u, float v) const {
    // TODO: Some `sampler` like stuff for different sampling methods when outside range
    float u_whole, u_fractional;
    u_fractional = std::modf(u, &u_whole);

    float v_whole, v_fractional;
    v_fractional = std::modf(v, &v_whole);
    
    return this->get_pixel_xy(round(u_fractional * m_width), round(v_fractional * m_height));
}
