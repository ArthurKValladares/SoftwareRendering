#include "texture.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture create_texture(const char *path) {
    int width, height, channels;
    unsigned char *img = stbi_load(path, &width, &height, &channels, 0);
    if (img == NULL) {
        printf("Error in loading the image\n");
        exit(1);
    }
    return Texture{width, height, channels, img};
}
