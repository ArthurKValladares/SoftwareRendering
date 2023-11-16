#include "depth_buffer.h"

DepthBuffer::DepthBuffer(u32 width, u32 height) :
    m_width(width),
    m_height(height)
{
    m_pixels = (float*) malloc(width * height * sizeof(float));
}

float DepthBuffer::ValueAt(u32 width, u32 height) const {
    return m_pixels[height * m_width + width];
}

void DepthBuffer::Write(u32 width, u32 height, float depth) {
    m_pixels[height * m_width + width] = depth;
}

void DepthBuffer::Clear() {
    // TODO: just memset here later
    for (int h = 0; h < m_height; ++h) {
        for (int w = 0; w < m_width; ++w) {
            this->Write(w, h, 0.0);
        }
    }
}