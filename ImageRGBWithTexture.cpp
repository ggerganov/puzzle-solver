/*! \file ImageRGBWithTexture.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "Types.h"
#include "Functions.h"

#include "GL/gl3w.h"

template <>
bool LoadFromFile::operator()<ImageRGBWithTexture>(ImageRGBWithTexture & obj, const char * fname) {
    return this->operator()(obj.image, fname);
}

template <>
bool Resize::operator()<ImageRGBWithTexture>(ImageRGBWithTexture & obj, const int32_t nx, const int32_t ny) {
    return this->operator()(obj.image, nx, ny);
}

template <>
bool Free::operator()<ImageRGBWithTexture>(ImageRGBWithTexture & obj) {
    printf("Destroying OpenGL texture %d\n", obj.texture.id);
    this->operator()(obj.texture);

    return true;
}

template <>
bool GenerateTexture::operator()<ImageRGBWithTexture>(ImageRGBWithTexture & obj, bool linearInterp) {
    obj.texture = Texture();

    GLuint texId;
    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);

    GLenum filter = (linearInterp) ? GL_LINEAR : GL_NEAREST;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLenum inputColourFormat = GL_RGB;

    // Create the texture
    glTexImage2D(GL_TEXTURE_2D,      // Type of texture
                 0,                  // Pyramid level (for mip-mapping) - 0 is the top level
                 GL_RGB,             // Internal colour format to convert to
                 obj.image.nx,             // Image width  i.e. 640 for Kinect in standard mode
                 obj.image.ny,             // Image height i.e. 480 for Kinect in standard mode
                 0,                  // Border width in pixels (can either be 1 or 0)
                 inputColourFormat,  // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                 GL_UNSIGNED_BYTE,   // Image data type
                 obj.image.pixels.data()); // The actual image data itself

    obj.texture.id = texId;

    printf("Allocated  OpenGL texture %d\n", obj.texture.id);

    return true;
}

template <>
bool IsValid::operator()<ImageRGBWithTexture>(const ImageRGBWithTexture & obj) {
    return this->operator()(obj.image) && this->operator()(obj.texture);
}

template <>
ImageRGBWithTexture ComputeDifference::operator()<ImageRGBWithTexture>(const ImageRGBWithTexture & obj0, const ImageRGBWithTexture & obj1) {
    ImageRGBWithTexture result;

    result.image = this->operator()(obj0.image, obj1.image);
    ::GenerateTexture()(result, true);

    return result;
}