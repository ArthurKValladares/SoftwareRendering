#include "overdraw_indicator.h"

OverdrawBuffer::OverdrawBuffer(u32 width, u32 height) :
    m_width(width),
    m_height(height)
{
    m_pixels = (u8*) malloc(width * height * sizeof(float));
}

u8 OverdrawBuffer::ValueAt(u32 width, u32 height) const {
    return m_pixels[height * m_width + width];
}

u8 OverdrawBuffer::Increase(u32 width, u32 height) {
    const u64 index = height * m_width + width;
    const u32 curr = m_pixels[index] + 1;
    m_pixels[index] = curr;
    return curr;
}

void OverdrawBuffer::Clear() {
    // TODO: just memset here later
    for (int h = 0; h < m_height; ++h) {
        for (int w = 0; w < m_width; ++w) {
            const u64 index = h * m_width + w;
            m_pixels[index] = 0;
        }
    }
}