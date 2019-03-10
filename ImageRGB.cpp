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

                int w = 5;
                int ws = 1;
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
        case HistDiff:
            {
                int nx = obj0.nx;
                int ny = obj0.ny;

                BufferRGB objm0;
                BufferRGB objm1;

                //BufferRGB objm0_0;
                //BufferRGB objm1_0;

                //BufferRGB objm0_1;
                //BufferRGB objm1_1;

                //BufferRGB objm0_2;
                //BufferRGB objm1_2;

                int L = 8;
                int W = 8;

                {
                    int nnx = 0;
                    int nny = 0;

                    ggimg::scale_li_maxside_2d_rgb(nx, ny, obj0.pixels.data(), 1024, nnx, nny, objm0);
                    ggimg::scale_li_maxside_2d_rgb(nx, ny, obj1.pixels.data(), 1024, nnx, nny, objm1);

                    nx = nnx;
                    ny = nny;

                    //objm0_0 = objm0;
                    //objm1_0 = objm1;

                    //objm0_1 = objm0;
                    //objm1_1 = objm1;

                    //objm0_2 = objm0;
                    //objm1_2 = objm1;

                    //ggimg::gaussian_filter_2d_rgb(nx, ny, objm0.data(), objm0_0.data(), 1.0f);
                    //ggimg::gaussian_filter_2d_rgb(nx, ny, objm1.data(), objm1_0.data(), 1.0f);

                    //ggimg::gaussian_filter_2d_rgb(nx, ny, objm0.data(), objm0_1.data(), 3.0f);
                    //ggimg::gaussian_filter_2d_rgb(nx, ny, objm1.data(), objm1_1.data(), 3.0f);

                    //ggimg::gaussian_filter_2d_rgb(nx, ny, objm0.data(), objm0_2.data(), 6.0f);
                    //ggimg::gaussian_filter_2d_rgb(nx, ny, objm1.data(), objm1_2.data(), 6.0f);

                    //ggimg::median_filter_2d_rgb(nx, ny, objm0.data(), objm0.data(), 4);
                    //ggimg::median_filter_2d_rgb(nx, ny, objm1.data(), objm1.data(), 4);

                    ggimg::gaussian_filter_2d_rgb(nx, ny, objm0.data(), objm0.data(), 3.0f);
                    ggimg::gaussian_filter_2d_rgb(nx, ny, objm1.data(), objm1.data(), 3.0f);
                }

                //std::vector<uint8_t> objr0(nx*ny);
                //std::vector<uint8_t> objr1(nx*ny);
                //std::vector<uint8_t> objrx0(nx*ny);
                //std::vector<uint8_t> objrx1(nx*ny);
                //std::vector<uint8_t> objry0(nx*ny);
                //std::vector<uint8_t> objry1(nx*ny);
                //std::vector<uint8_t> objrxy0(nx*ny);
                //std::vector<uint8_t> objrxy1(nx*ny);

                //std::vector<uint8_t> objg0(nx*ny);
                //std::vector<uint8_t> objg1(nx*ny);
                //std::vector<uint8_t> objgx0(nx*ny);
                //std::vector<uint8_t> objgx1(nx*ny);
                //std::vector<uint8_t> objgy0(nx*ny);
                //std::vector<uint8_t> objgy1(nx*ny);
                //std::vector<uint8_t> objgxy0(nx*ny);
                //std::vector<uint8_t> objgxy1(nx*ny);

                //std::vector<uint8_t> objb0(nx*ny);
                //std::vector<uint8_t> objb1(nx*ny);
                //std::vector<uint8_t> objbx0(nx*ny);
                //std::vector<uint8_t> objbx1(nx*ny);
                //std::vector<uint8_t> objby0(nx*ny);
                //std::vector<uint8_t> objby1(nx*ny);
                //std::vector<uint8_t> objbxy0(nx*ny);
                //std::vector<uint8_t> objbxy1(nx*ny);

                //std::vector<uint8_t> objgray0(nx*ny);
                //std::vector<uint8_t> objgray1(nx*ny);
                //std::vector<uint8_t> objgrayx0(nx*ny);
                //std::vector<uint8_t> objgrayx1(nx*ny);
                //std::vector<uint8_t> objgrayy0(nx*ny);
                //std::vector<uint8_t> objgrayy1(nx*ny);
                //std::vector<uint8_t> objgrayxy0(nx*ny);
                //std::vector<uint8_t> objgrayxy1(nx*ny);

                //{

                //    ggimg::rgb_to_r_2d(nx, ny, objm0.data(), objr0.data());
                //    ggimg::gradient_sobel_2d(1, nx, ny, objr0.data(), (uint8_t) 255, objrx0.data());
                //    ggimg::gradient_sobel_2d(2, nx, ny, objr0.data(), (uint8_t) 255, objry0.data());
                //    ggimg::gradient_sobel_2d(0, nx, ny, objr0.data(), (uint8_t) 255, objrxy0.data());

                //    ggimg::rgb_to_r_2d(nx, ny, objm1.data(), objr1.data());
                //    ggimg::gradient_sobel_2d(1, nx, ny, objr1.data(), (uint8_t) 255, objrx1.data());
                //    ggimg::gradient_sobel_2d(2, nx, ny, objr1.data(), (uint8_t) 255, objry1.data());
                //    ggimg::gradient_sobel_2d(0, nx, ny, objr1.data(), (uint8_t) 255, objrxy1.data());
                //}

                //{

                //    ggimg::rgb_to_g_2d(nx, ny, objm0.data(), objg0.data());
                //    ggimg::gradient_sobel_2d(1, nx, ny, objg0.data(), (uint8_t) 255, objgx0.data());
                //    ggimg::gradient_sobel_2d(2, nx, ny, objg0.data(), (uint8_t) 255, objgy0.data());
                //    ggimg::gradient_sobel_2d(0, nx, ny, objg0.data(), (uint8_t) 255, objgxy0.data());

                //    ggimg::rgb_to_g_2d(nx, ny, objm1.data(), objg1.data());
                //    ggimg::gradient_sobel_2d(1, nx, ny, objg1.data(), (uint8_t) 255, objgx1.data());
                //    ggimg::gradient_sobel_2d(2, nx, ny, objg1.data(), (uint8_t) 255, objgy1.data());
                //    ggimg::gradient_sobel_2d(0, nx, ny, objg1.data(), (uint8_t) 255, objgxy1.data());
                //}

                //{

                //    ggimg::rgb_to_b_2d(nx, ny, objm0.data(), objb0.data());
                //    ggimg::gradient_sobel_2d(1, nx, ny, objb0.data(), (uint8_t) 255, objbx0.data());
                //    ggimg::gradient_sobel_2d(2, nx, ny, objb0.data(), (uint8_t) 255, objby0.data());
                //    ggimg::gradient_sobel_2d(0, nx, ny, objb0.data(), (uint8_t) 255, objbxy0.data());

                //    ggimg::rgb_to_b_2d(nx, ny, objm1.data(), objb1.data());
                //    ggimg::gradient_sobel_2d(1, nx, ny, objb1.data(), (uint8_t) 255, objbx1.data());
                //    ggimg::gradient_sobel_2d(2, nx, ny, objb1.data(), (uint8_t) 255, objby1.data());
                //    ggimg::gradient_sobel_2d(0, nx, ny, objb1.data(), (uint8_t) 255, objbxy1.data());
                //}

                //{

                //    ggimg::rgb_to_gray_2d(nx, ny, objm0.data(), objgray0.data());
                //    ggimg::gradient_sobel_2d(1, nx, ny, objgray0.data(), (uint8_t) 255, objgrayx0.data());
                //    ggimg::gradient_sobel_2d(2, nx, ny, objgray0.data(), (uint8_t) 255, objgrayy0.data());
                //    ggimg::gradient_sobel_2d(0, nx, ny, objgray0.data(), (uint8_t) 255, objgrayxy0.data());

                //    ggimg::rgb_to_gray_2d(nx, ny, objm1.data(), objgray1.data());
                //    ggimg::gradient_sobel_2d(1, nx, ny, objgray1.data(), (uint8_t) 255, objgrayx1.data());
                //    ggimg::gradient_sobel_2d(2, nx, ny, objgray1.data(), (uint8_t) 255, objgrayy1.data());
                //    ggimg::gradient_sobel_2d(0, nx, ny, objgray1.data(), (uint8_t) 255, objgrayxy1.data());
                //}

                std::vector<uint8_t> objh0(12*L*nx*ny);
                std::vector<uint8_t> objh1(12*L*nx*ny);

                ggimg::lhist_filter_2d_rgb(nx, ny, objm0.data(), objh0.data(), L, W);
                ggimg::lhist_filter_2d_rgb(nx, ny, objm1.data(), objh1.data(), L, W);

                ggimg::lhist_filter_2d_rgb(nx, ny, objm0.data(), objh0.data() + 3*L*nx*ny, L, 2*W);
                ggimg::lhist_filter_2d_rgb(nx, ny, objm1.data(), objh1.data() + 3*L*nx*ny, L, 2*W);

                ggimg::lhist_filter_2d_rgb(nx, ny, objm0.data(), objh0.data() + 6*L*nx*ny, L, 3*W);
                ggimg::lhist_filter_2d_rgb(nx, ny, objm1.data(), objh1.data() + 6*L*nx*ny, L, 3*W);

                ggimg::lhist_filter_2d_rgb(nx, ny, objm0.data(), objh0.data() + 9*L*nx*ny, L, 4*W);
                ggimg::lhist_filter_2d_rgb(nx, ny, objm1.data(), objh1.data() + 9*L*nx*ny, L, 4*W);

                //ggimg::lhist_filter_2d_gray(nx, ny, objgray0.data(), objh0.data() + 3*L*nx*ny, L, W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objgray1.data(), objh1.data() + 3*L*nx*ny, L, W);

                //ggimg::lhist_filter_2d_rgb(nx, ny, objm0_0.data(), objh0.data() + 3*L*nx*ny, L, W);
                //ggimg::lhist_filter_2d_rgb(nx, ny, objm1_0.data(), objh1.data() + 3*L*nx*ny, L, W);

                //ggimg::lhist_filter_2d_rgb(nx, ny, objm0_1.data(), objh0.data() + 6*L*nx*ny, L, W);
                //ggimg::lhist_filter_2d_rgb(nx, ny, objm1_1.data(), objh1.data() + 6*L*nx*ny, L, W);

                //ggimg::lhist_filter_2d_rgb(nx, ny, objm0_2.data(), objh0.data() + 9*L*nx*ny, L, W);
                //ggimg::lhist_filter_2d_rgb(nx, ny, objm1_2.data(), objh1.data() + 9*L*nx*ny, L, W);

                //ggimg::lhist_filter_2d_gray(nx, ny, objgrayxy0.data(), objh0.data() + 3*L*nx*ny, L, W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objgrayxy1.data(), objh1.data() + 3*L*nx*ny, L, W);

                //ggimg::lhist_filter_2d_gray(nx, ny, objgrayxy0.data(), objh0.data() + 4*L*nx*ny, L, W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objgrayxy1.data(), objh1.data() + 4*L*nx*ny, L, W);

                //ggimg::lhist_filter_2d_gray(nx, ny, objgrayxy0.data(), objh0.data() + 5*L*nx*ny, L, W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objgrayxy1.data(), objh1.data() + 5*L*nx*ny, L, W);

                //ggimg::lhist_filter_2d_gray(nx, ny, objrxy0.data(), objh0.data() + 3*L*nx*ny, L, W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objrxy1.data(), objh1.data() + 3*L*nx*ny, L, W);

                //ggimg::lhist_filter_2d_gray(nx, ny, objgxy0.data(), objh0.data() + 4*L*nx*ny, L, W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objgxy1.data(), objh1.data() + 4*L*nx*ny, L, W);

                //ggimg::lhist_filter_2d_gray(nx, ny, objbxy0.data(), objh0.data() + 5*L*nx*ny, L, W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objbxy1.data(), objh1.data() + 5*L*nx*ny, L, W);

                //ggimg::lhist_filter_2d_gray(nx, ny, objrx0.data(),  objh0.data() +  3*L*nx*ny, L, W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objrx1.data(),  objh1.data() +  3*L*nx*ny, L, W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objry0.data(),  objh0.data() +  4*L*nx*ny, L, W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objry1.data(),  objh1.data() +  4*L*nx*ny, L, W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objrxy0.data(), objh0.data() +  5*L*nx*ny, L, W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objrxy1.data(), objh1.data() +  5*L*nx*ny, L, W);

                //ggimg::lhist_filter_2d_gray(nx, ny, objgx0.data(),  objh0.data() +  6*L*nx*ny, L, W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objgx1.data(),  objh1.data() +  6*L*nx*ny, L, W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objgy0.data(),  objh0.data() +  7*L*nx*ny, L, W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objgy1.data(),  objh1.data() +  7*L*nx*ny, L, W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objgxy0.data(), objh0.data() +  8*L*nx*ny, L, W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objgxy1.data(), objh1.data() +  8*L*nx*ny, L, W);

                //ggimg::lhist_filter_2d_gray(nx, ny, objbx0.data(),  objh0.data() +  9*L*nx*ny, L, W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objbx1.data(),  objh1.data() +  9*L*nx*ny, L, W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objby0.data(),  objh0.data() + 10*L*nx*ny, L, W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objby1.data(),  objh1.data() + 10*L*nx*ny, L, W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objbxy0.data(), objh0.data() + 11*L*nx*ny, L, W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objbxy1.data(), objh1.data() + 11*L*nx*ny, L, W);

                result.pixels = objm0;
                result.nx = nx;
                result.ny = ny;

                int k0 = 0;
                int k1 = 12;
                int ws = 0;
                int ww = ws;
                int nw = (k1 - k0)*L;

                for (int y = ww; y < ny - ww; ++y) {
                    for (int x = ww; x < nx - ww; ++x) {
                        int i = y*nx + x;
                        if (objm1[3*i + 0] == 0 &&
                            objm1[3*i + 1] == 0 &&
                            objm1[3*i + 2] == 0) {
                            result.pixels[3*i + 0] = 0;
                            result.pixels[3*i + 1] = 0;
                            result.pixels[3*i + 2] = 0;
                            continue;
                        }

                        int diffbest = 300*nw;

                        for (int yy = y - ws; yy <= y + ws; ++yy) {
                            for (int xx = x - ws; xx <= x + ws; ++xx) {
                                int ii = yy*nx + xx;

                                int diffcur = 0.0;

                                for (int l = 0; l < L; ++l) {
                                    for (int k = k0; k < k1; ++k) {
                                        diffcur += std::abs(objh0[k*L*nx*ny + L*i + l] - objh1[k*L*nx*ny + L*ii + l]);
                                    }
                                }

                                if (diffcur < diffbest) {
                                    diffbest = diffcur;
                                }
                            }
                        }

                        diffbest /= nw;

                        result.pixels[3*i + 0] = std::min(255, 10*diffbest);
                        result.pixels[3*i + 1] = std::min(255, 10*diffbest);
                        result.pixels[3*i + 2] = std::min(255, 10*diffbest);

                    }
                }

                for (int y = ww; y < ny - ww; ++y) {
                    for (int x = ww; x < nx - ww; ++x) {
                        int i = y*nx + x;
                        if (objm1[3*i + 0] == 0 &&
                            objm1[3*i + 1] == 0 &&
                            objm1[3*i + 2] == 0) {
                            result.pixels[3*i + 0] = 0;
                            result.pixels[3*i + 1] = 0;
                            result.pixels[3*i + 2] = 0;
                            continue;
                        }

                        double sum0  = 0.0;
                        double sum02 = 0.0;

                        for (int l = 0; l < L; ++l) {
                            for (int k = k0; k < k1; ++k) {
                                double v = objh0[k*L*nx*ny + L*i + l];

                                sum0 += v;
                                sum02 += v*v;
                            }
                        }

                        double ccbest = -1.0;

                        for (int yy = y - ws; yy <= y + ws; ++yy) {
                            for (int xx = x - ws; xx <= x + ws; ++xx) {
                                int ii = yy*nx + xx;

                                double sum1 = 0.0;
                                double sum12 = 0.0;
                                double sum01 = 0.0;

                                for (int l = 0; l < L; ++l) {
                                    for (int k = k0; k < k1; ++k) {
                                        double v0 = objh0[k*L*nx*ny + L*i + l];
                                        double v1 = objh1[k*L*nx*ny + L*ii + l];

                                        sum1 += v1;
                                        sum12 += v1*v1;
                                        sum01 += v0*v1;
                                    }
                                }

                                double cccur = 0.0;

                                {
                                    double nom   = sum01*nw - sum0*sum1;
                                    double den2a = sum02*nw - sum0*sum0;
                                    double den2b = sum12*nw - sum1*sum1;
                                    cccur = (nom)/(sqrt(den2a*den2b));
                                }

                                if (cccur > ccbest) {
                                    ccbest = cccur;
                                }
                            }
                        }

                        result.pixels[3*i + 0] *= (1.0 - ccbest);
                        result.pixels[3*i + 1] *= (1.0 - ccbest);
                        result.pixels[3*i + 2] *= (1.0 - ccbest);

                        //result.pixels[3*i + 0] = 127*(1.0 - ccbest);
                        //result.pixels[3*i + 1] = 127*(1.0 - ccbest);
                        //result.pixels[3*i + 2] = 127*(1.0 - ccbest);

                        //result.pixels[3*i + 0] = objgray0[i];
                        //result.pixels[3*i + 1] = objgray0[i];
                        //result.pixels[3*i + 2] = objgray0[i];
                    }
                }
            }
        break;
    }

    ::GenerateTexture()(result, true);

    return result;
}
