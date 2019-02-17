/*! \file ImageRGB.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "Types.h"
#include "Functions.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include <GLFW/glfw3.h>

template <>
bool Resize::operator()<ImageRGB>(const int32_t nx, const int32_t ny, ImageRGB & obj) {
    obj.nx = nx;
    obj.ny = ny;
    ::Resize()(nx*ny, obj.pixels);

    return true;
}

template <>
bool LoadFromFile::operator()<ImageRGB>(const char * fname, ImageRGB & obj) {
    int nx, ny, nz;
    uint8_t * data = stbi_load(fname, &nx, &ny, &nz, STBI_rgb);
    if (data == nullptr) {
        return false;
    }

    bool res = true;

    if (nz == 1) {
        obj.nx = nx;
        obj.ny = ny;
        ::Resize()(nx*ny, obj.pixels);
        for (int i = 0; i < nx*ny; ++i) {
            obj.pixels[3*i + 0] = data[i];
            obj.pixels[3*i + 1] = data[i];
            obj.pixels[3*i + 2] = data[i];
        }
    }

    if (nz == 2) {
        res = false;
    }

    if (nz == 3) {
        obj.nx = nx;
        obj.ny = ny;
        ::Resize()(nx*ny, obj.pixels);
        std::copy(data, data + 3*nx*ny, obj.pixels.begin());
    }

    if (nz == 4) {
        obj.nx = nx;
        obj.ny = ny;
        ::Resize()(nx*ny, obj.pixels);
        for (int i = 0; i < nx*ny; ++i) {
            obj.pixels[3*i + 0] = data[4*i + 0];
            obj.pixels[3*i + 1] = data[4*i + 1];
            obj.pixels[3*i + 2] = data[4*i + 2];
        }
    }

    if (nz > 4) {
        res = false;
    }

    stbi_image_free(data);

    return res;
}

template <>
bool Free::operator()<ImageRGB>(ImageRGB & obj) {
    this->operator()(obj.texture);

    return true;
}

template <>
bool GenerateTexture::operator()<ImageRGB>(bool linearInterp, ImageRGB & obj) {
    if (obj.texture.id >= 0) {
        GLuint texId = obj.texture.id;
        glDeleteTextures(1, &texId);
    }

    obj.texture = Texture();

    GLuint texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);

    GLenum filter = (linearInterp) ? GL_LINEAR : GL_NEAREST;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

    GLenum inputColourFormat = GL_RGB;

    // Create the texture
    glTexImage2D(GL_TEXTURE_2D,      // Type of texture
                 0,                  // Pyramid level (for mip-mapping) - 0 is the top level
                 GL_RGB,             // Internal colour format to convert to
                 obj.nx,             // Image width  i.e. 640 for Kinect in standard mode
                 obj.ny,             // Image height i.e. 480 for Kinect in standard mode
                 0,                  // Border width in pixels (can either be 1 or 0)
                 inputColourFormat,  // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                 GL_UNSIGNED_BYTE,   // Image data type
                 obj.pixels.data()); // The actual image data itself

    obj.texture.id = texId;

    return true;
}
