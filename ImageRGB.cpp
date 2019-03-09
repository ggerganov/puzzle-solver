/*! \file ImageRGB.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "Types.h"
#include "Functions.h"

#include "ggimg/ggimg.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

#include "GL/gl3w.h"

template <>
bool Resize::operator()<ImageRGB>(ImageRGB & obj, const int32_t nx, const int32_t ny) {
    obj.nx = nx;
    obj.ny = ny;
    ::Resize()(obj.pixels, nx*ny);

    return true;
}

template <>
bool LoadFromFile::operator()<ImageRGB>(ImageRGB & obj, const char * fname) {
    int nx, ny, nz;
    uint8_t * data = stbi_load(fname, &nx, &ny, &nz, STBI_rgb);
    if (data == nullptr) {
        return false;
    }

    bool res = true;

    if (nz == 1) {
        obj.nx = nx;
        obj.ny = ny;
        ::Resize()(obj.pixels, nx*ny);
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
        ::Resize()(obj.pixels, nx*ny);
        std::copy(data, data + 3*nx*ny, obj.pixels.begin());
    }

    if (nz == 4) {
        obj.nx = nx;
        obj.ny = ny;
        ::Resize()(obj.pixels, nx*ny);
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
    printf("Destroying OpenGL texture %d\n", obj.texture.id);
    this->operator()(obj.texture);

    return true;
}

template <>
bool GenerateTexture::operator()<ImageRGB>(ImageRGB & obj, bool linearInterp) {
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
                 obj.nx,             // Image width  i.e. 640 for Kinect in standard mode
                 obj.ny,             // Image height i.e. 480 for Kinect in standard mode
                 0,                  // Border width in pixels (can either be 1 or 0)
                 inputColourFormat,  // Input image format (i.e. GL_RGB, GL_RGBA, GL_BGR etc.)
                 GL_UNSIGNED_BYTE,   // Image data type
                 obj.pixels.data()); // The actual image data itself

    obj.texture.id = texId;

    printf("Allocated  OpenGL texture %d\n", obj.texture.id);

    return true;
}

template <>
bool IsValid::operator()<ImageRGB>(const ImageRGB & obj) {
    return obj.nx > 0 && obj.ny > 0 && ::Count()(obj.pixels) == (obj.nx*obj.ny) && this->operator()(obj.texture);
}

template <>
ImageRGB ComputeDifference::operator()<ImageRGB>(const ImageRGB & obj0, const ImageRGB & obj1) {
    ImageRGB result;

    switch (method) {
        case Standard:
            {
                result = obj0;
                int n = result.nx*result.ny;
                for (int i = 0; i < n; ++i) {
                    if (obj1.pixels[3*i + 0] == 0 &&
                        obj1.pixels[3*i + 1] == 0 &&
                        obj1.pixels[3*i + 2] == 0) {
                        result.pixels[3*i + 0] = 0;
                        result.pixels[3*i + 1] = 0;
                        result.pixels[3*i + 2] = 0;
                        continue;
                    }

                    result.pixels[3*i + 0] = std::abs((int)(result.pixels[3*i + 0]) - obj1.pixels[3*i + 0]);
                    result.pixels[3*i + 1] = std::abs((int)(result.pixels[3*i + 1]) - obj1.pixels[3*i + 1]);
                    result.pixels[3*i + 2] = std::abs((int)(result.pixels[3*i + 2]) - obj1.pixels[3*i + 2]);
                }
            }
        break;
        case LocalDiff:
            {
                int nx = obj0.nx;
                int ny = obj0.ny;

                auto objm0 = obj0;
                auto objm1 = obj1;

                {
                    int nnx = 0;
                    int nny = 0;

                    ggimg::scale_li_maxside_2d_rgb(nx, ny, obj0.pixels.data(), 1024, nnx, nny, objm0.pixels);
                    ggimg::scale_li_maxside_2d_rgb(nx, ny, obj1.pixels.data(), 1024, nnx, nny, objm1.pixels);

                    nx = nnx;
                    ny = nny;
                }

                ggimg::median_filter_2d_rgb(nx, ny, objm0.pixels.data(), objm0.pixels.data(), 10);
                ggimg::median_filter_2d_rgb(nx, ny, objm1.pixels.data(), objm1.pixels.data(), 10);

                result = objm0;
                result.nx = nx;
                result.ny = ny;

                int w = 1;
                int ws = 5;
                int ww = ws + w;
                int nw = 3*(2*w + 1)*(2*w + 1);

                for (int y = ww; y < ny - ww; ++y) {
                    for (int x = ww; x < nx - ww; ++x) {
                        int i = y*nx + x;
                        if (objm1.pixels[3*i + 0] == 0 &&
                            objm1.pixels[3*i + 1] == 0 &&
                            objm1.pixels[3*i + 2] == 0) {
                            result.pixels[3*i + 0] = 0;
                            result.pixels[3*i + 1] = 0;
                            result.pixels[3*i + 2] = 0;
                            continue;
                        }

                        int diffbest = 300*nw;

                        for (int yy = y - ws; yy <= y + ws; ++yy) {
                            for (int xx = x - ws; xx <= x + ws; ++xx) {
                                int diffcur = 0.0;

                                for (int yyy = yy - w; yyy <= yy + w; ++yyy) {
                                    for (int xxx = xx - w; xxx <= xx + w; ++xxx) {
                                        int iii = yyy*nx + xxx;
                                        int ii = (yyy - yy + y)*nx + (xxx - xx + x);

                                        diffcur += std::abs(((int)(objm0.pixels[3*ii + 0])) - ((int)(objm1.pixels[3*iii + 0])));
                                        diffcur += std::abs(((int)(objm0.pixels[3*ii + 1])) - ((int)(objm1.pixels[3*iii + 1])));
                                        diffcur += std::abs(((int)(objm0.pixels[3*ii + 2])) - ((int)(objm1.pixels[3*iii + 2])));
                                    }
                                }

                                if (diffcur < diffbest) {
                                    diffbest = diffcur;
                                }
                            }
                        }

                        diffbest /= nw;

                        result.pixels[3*i + 0] = diffbest;
                        result.pixels[3*i + 1] = diffbest;
                        result.pixels[3*i + 2] = diffbest;
                    }
                }
            }
        break;
    }

    ::GenerateTexture()(result, true);

    return result;
}
