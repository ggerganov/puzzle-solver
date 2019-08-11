/*! \file ImageRGB.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "Types.h"
#include "Functions.h"

#define GGIMG_FS
#include "ggimg/ggimg.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"

namespace {
constexpr auto kInvMax = 1.0/RAND_MAX;
inline float frand() { return float(rand())*kInvMax; }
}

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
bool IsValid::operator()<ImageRGB>(const ImageRGB & obj) {
    return obj.nx > 0 && obj.ny > 0 && ::Count()(obj.pixels) == (obj.nx*obj.ny);
}

template <>
std::tuple<int64_t, int64_t> ComputeSums::operator()<ImageRGB>(const ImageRGB & a, float wx, float wy, int sx, int sy) {
    int nx = a.nx;
    int ny = a.ny;

    int nx0 = 0.5f*float(nx)*(1.0f - wx);
    int nx1 = 0.5f*float(nx)*(1.0f + wx);
    int ny0 = 0.5f*float(ny)*(1.0f - wy);
    int ny1 = 0.5f*float(ny)*(1.0f + wy);

    int64_t sum0  = 0;
    int64_t sum02 = 0;

    for (int y = ny0; y < ny1; y += sy) {
        for (int x = nx0; x < nx1; x += sx) {
            int i = y*nx + x;

            { int32_t v = a.pixels[3*i + 0]; sum0 += v; sum02 += v*v; }
            { int32_t v = a.pixels[3*i + 1]; sum0 += v; sum02 += v*v; }
            { int32_t v = a.pixels[3*i + 2]; sum0 += v; sum02 += v*v; }
        }
    }

    return { sum0, sum02 };
}

template <>
double ComputeSSD::operator()<ImageRGB>(const ImageRGB & a, const ImageRGB & b, float wx, float wy, int sx, int sy) {
    int nx = a.nx;
    int ny = a.ny;

    int nx0 = 0.5f*float(nx)*(1.0f - wx);
    int nx1 = 0.5f*float(nx)*(1.0f + wx);
    int ny0 = 0.5f*float(ny)*(1.0f - wy);
    int ny1 = 0.5f*float(ny)*(1.0f + wy);

    int64_t n = 0;
    int64_t sum = 0;

    for (int y = ny0; y < ny1; y += sy) {
        for (int x = nx0; x < nx1; x += sx) {
            int i = y*nx + x;

            {
                int32_t v0 = a.pixels[3*i + 0];
                int32_t v1 = b.pixels[3*i + 0];

                sum += (v1 - v0)*(v1 - v0);
            }

            {
                int32_t v0 = a.pixels[3*i + 1];
                int32_t v1 = b.pixels[3*i + 1];

                sum += (v1 - v0)*(v1 - v0);
            }

            {
                int32_t v0 = a.pixels[3*i + 2];
                int32_t v1 = b.pixels[3*i + 2];

                sum += (v1 - v0)*(v1 - v0);
            }

            ++n;
        }
    }

    n *= 3;

    return double(sum)/n;
}

template <>
double ComputeCC::operator()<ImageRGB>(const ImageRGB & a, const ImageRGB & b, int64_t sum0, int64_t sum02, float wx, float wy, int sx, int sy) {
    int nx = a.nx;
    int ny = a.ny;

    int nx0 = 0.5f*float(nx)*(1.0f - wx);
    int nx1 = 0.5f*float(nx)*(1.0f + wx);
    int ny0 = 0.5f*float(ny)*(1.0f - wy);
    int ny1 = 0.5f*float(ny)*(1.0f + wy);

    int64_t n = 0;
    int64_t sum1 = 0;
    int64_t sum12 = 0;
    int64_t sum01 = 0;

    for (int y = ny0; y < ny1; y += sy) {
        for (int x = nx0; x < nx1; x += sx) {
            int i = y*nx + x;

            {
                int32_t v0 = a.pixels[3*i + 0];
                int32_t v1 = b.pixels[3*i + 0];

                sum1 += v1;
                sum12 += v1*v1;
                sum01 += v0*v1;
            }
            {
                int32_t v0 = a.pixels[3*i + 1];
                int32_t v1 = b.pixels[3*i + 1];

                sum1 += v1;
                sum12 += v1*v1;
                sum01 += v0*v1;
            }
            {
                int32_t v0 = a.pixels[3*i + 2];
                int32_t v1 = b.pixels[3*i + 2];

                sum1 += v1;
                sum12 += v1*v1;
                sum01 += v0*v1;
            }

            ++n;
        }
    }

    n *= 3;

    double cc = 0.0;

    {
        double nom   = sum01*n - sum0*sum1;
        double den2a = sum02*n - sum0*sum0;
        double den2b = sum12*n - sum1*sum1;
        cc = (nom)/(sqrt(den2a*den2b));
    }

    return cc;
}

template <>
ImageRGB Register::operator()<ImageRGB>(const ImageRGB & img0, const ImageRGB & img1, const std::array<Point2D, 4> & pi, std::array<Point2D, 4> & pj) {
    int nx0 = img0.nx;
    int ny0 = img0.ny;

    auto bestpj = pj;
    double bestcc = -1e30;

    for (int nTry = 0; nTry < 1; ++nTry) {
        int nIters = 256;
        int nScale = 256;
        float pvar = 0.005;

        for (int s = 0; s < 8; ++s) {
            ImageRGB img0r;
            ImageRGB img1r;

            int nnx = 0;
            int nny = 0;

            ggimg::scale_li_maxside_2d_rgb(nx0, ny0, img0.pixels.data(), nScale,
                                           nnx, nny, img0r.pixels);
            ggimg::scale_li_maxside_2d_rgb(nx0, ny0, img1.pixels.data(), nScale,
                                           nnx, nny, img1r.pixels);

            int nx = nnx;
            int ny = nny;

            img1r.nx = nx;
            img1r.ny = ny;

            auto [sum1, sum12] = ::ComputeSums()(img1r, 0.75f, 0.75f, 1, 1);

            for (int i = 0; i < nIters; ++i) {
                pj = bestpj;
                if (i > 0) {
                    for (int k = 0; k < 4; ++k) {
                        pj[k].x += pvar * (0.5f - frand());
                        pj[k].y += pvar * (0.5f - frand());
                    }
                }

                auto homography = ::ComputeHomography()(pj, pi);

                ImageRGB img0rp;
                ::Resize()(img0rp, nx, ny);
                ::ggimg::transform_homography_nn_rgb(
                    nx, ny, img0r.pixels.data(), homography, nx, ny,
                    img0rp.pixels.data());

                auto cc = ::ComputeCC()(img1r, img0rp, sum1, sum12, 0.75f, 0.75f, 1, 1);

                if (cc > bestcc) {
                    if (bestcc < -1e10) {
                        printf("cc = %g\n", cc);
                    }
                    bestcc = cc;
                    bestpj = pj;
                }
            }

            nScale *= 1.1;
            pvar *= 0.5f;
            printf("pyramid %d, bestcc = %g\n", s, bestcc);
        }
    }

    pj = bestpj;

    ImageRGB img0p;

    auto homography = ::ComputeHomography()(pj, pi);

    printf("Homography:\n");
    printf("    %9.5f %9.5f %9.5f\n", homography[0], homography[1], homography[2]);
    printf("    %9.5f %9.5f %9.5f\n", homography[3], homography[4], homography[5]);
    printf("    %9.5f %9.5f %9.5f\n", homography[6], homography[7], homography[8]);
    printf("\n");

    ::Resize()(img0p, nx0, ny0);
    ::ggimg::transform_homography_nn_rgb(nx0, ny0, img0.pixels.data(), homography, nx0, ny0, img0p.pixels.data());

    return img0p;
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

                //ggimg::gaussian_filter_2d_rgb(nx, ny, objm0.pixels.data(), objm0.pixels.data(), 3.0f);
                //ggimg::gaussian_filter_2d_rgb(nx, ny, objm1.pixels.data(), objm1.pixels.data(), 3.0f);

                //ggimg::median_filter_2d_rgb(nx, ny, objm0.pixels.data(), objm0.pixels.data(), 1);
                //ggimg::median_filter_2d_rgb(nx, ny, objm1.pixels.data(), objm1.pixels.data(), 1);

                result.pixels.resize(3*nx*ny);
                result.nx = nx;
                result.ny = ny;

                {
                    int w = 10;
                    int ws = 5;
                    int ww = ws + w;
                    int nw = (2*w + 1)*(2*w + 1);

                    auto objc0 = objm0;
                    auto objc1 = objm1;

                    for (int y = w; y < ny - w; ++y) {
                        for (int x = w; x < nx - w; ++x) {
                            int i = y*nx + x;

                            double sum0r = 0.0;
                            double sum0g = 0.0;
                            double sum0b = 0.0;

                            double sum1r = 0.0;
                            double sum1g = 0.0;
                            double sum1b = 0.0;

                            for (int yyy = y - w; yyy <= y + w; ++yyy) {
                                for (int xxx = x - w; xxx <= x + w; ++xxx) {
                                    int iii = yyy*nx + xxx;
                                    sum0r += objm0.pixels[3*iii + 0];
                                    sum0g += objm0.pixels[3*iii + 1];
                                    sum0b += objm0.pixels[3*iii + 2];

                                    sum1r += objm1.pixels[3*iii + 0];
                                    sum1g += objm1.pixels[3*iii + 1];
                                    sum1b += objm1.pixels[3*iii + 2];
                                }
                            }

                            sum0r /= nw;
                            sum0g /= nw;
                            sum0b /= nw;

                            sum1r /= nw;
                            sum1g /= nw;
                            sum1b /= nw;

                            if (sum0r == 0.0) sum0r = 1.0;
                            if (sum0g == 0.0) sum0g = 1.0;
                            if (sum0b == 0.0) sum0b = 1.0;

                            if (sum1r == 0.0) sum1r = 1.0;
                            if (sum1g == 0.0) sum1g = 1.0;
                            if (sum1b == 0.0) sum1b = 1.0;

                            double avg0 = sqrt(0.333*(sum0r*sum0r + sum0g*sum0g + sum0b*sum0b));
                            double avg1 = sqrt(0.333*(sum1r*sum1r + sum1g*sum1g + sum1b*sum1b));

                            { auto & x = objc0.pixels[3*i + 0]; double y = (double)(x)/avg0*127.0; x = std::max(0.0, std::min(255.0, y)); }
                            { auto & x = objc0.pixels[3*i + 1]; double y = (double)(x)/avg0*127.0; x = std::max(0.0, std::min(255.0, y)); }
                            { auto & x = objc0.pixels[3*i + 2]; double y = (double)(x)/avg0*127.0; x = std::max(0.0, std::min(255.0, y)); }

                            { auto & x = objc1.pixels[3*i + 0]; double y = (double)(x)/avg1*127.0; x = std::max(0.0, std::min(255.0, y)); }
                            { auto & x = objc1.pixels[3*i + 1]; double y = (double)(x)/avg1*127.0; x = std::max(0.0, std::min(255.0, y)); }
                            { auto & x = objc1.pixels[3*i + 2]; double y = (double)(x)/avg1*127.0; x = std::max(0.0, std::min(255.0, y)); }
                        }
                    }

                    objm0 = objc0;
                    objm1 = objc1;

                    ggimg::write_ppm_rgb("diff_rel_img0.ppm", nx, ny, objm0.pixels);
                    ggimg::write_ppm_rgb("diff_rel_img1.ppm", nx, ny, objm1.pixels);
                }

                int w = 10;
                int ws = 3;
                int ww = ws + w;
                int nw = (2*w + 1)*(2*w + 1);

                std::vector<float> fres(nx*ny, 0.0f);

                float diffmin = 1e9;
                float diffmax = -1e9;

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

                        float diffbest = 1e9;

                        for (int yy = y - ws; yy <= y + ws; ++yy) {
                            for (int xx = x - ws; xx <= x + ws; ++xx) {
                                float diffcur = 0.0;

                                for (int yyy = yy - w; yyy <= yy + w; ++yyy) {
                                    for (int xxx = xx - w; xxx <= xx + w; ++xxx) {
                                        int iii = yyy*nx + xxx;
                                        int ii = (yyy - yy + y)*nx + (xxx - xx + x);

                                        //auto max = std::abs(((int)(objm0.pixels[3*ii + 0])) - ((int)(objm1.pixels[3*iii + 0])));
                                        //max = std::max(max, std::abs(((int)(objm0.pixels[3*ii + 1])) - ((int)(objm1.pixels[3*iii + 1]))));
                                        //max = std::max(max, std::abs(((int)(objm0.pixels[3*ii + 2])) - ((int)(objm1.pixels[3*iii + 2]))));
                                        //diffcur += 3*max;

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

                        diffbest /= 3*nw;

                        if (diffbest < diffmin) diffmin = diffbest;
                        if (diffbest > diffmax) diffmax = diffbest;

                        fres[i] = diffbest;
                    }
                }

                printf("diffmin = %g\n", diffmin);
                printf("diffmax = %g\n", diffmax);

                for (int y = ww; y < ny - ww; ++y) {
                    for (int x = ww; x < nx - ww; ++x) {
                        float v = (fres[y*nx + x] - diffmin)/(diffmax - diffmin);
                        result.pixels[3*(y*nx + x) + 0] = 250*(v);
                        result.pixels[3*(y*nx + x) + 1] = 250*(v);
                        result.pixels[3*(y*nx + x) + 2] = 250*(v);
                    }
                }
            }
            break;
        case HistDiff:
            {
                int nx = obj0.nx;
                int ny = obj0.ny;

                BufferRGB objm0;
                BufferRGB objm1;

                int L = 4;
                int W = 8;
                int C = 24;

                {
                    int nnx = 0;
                    int nny = 0;

                    ggimg::scale_li_maxside_2d_rgb(nx, ny, obj0.pixels.data(), 1024, nnx, nny, objm0);
                    ggimg::scale_li_maxside_2d_rgb(nx, ny, obj1.pixels.data(), 1024, nnx, nny, objm1);

                    nx = nnx;
                    ny = nny;
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

                {
                    //ggimg::median_filter_2d_rgb(nx, ny, objm0.data(), objm0.data(), 4);
                    //ggimg::median_filter_2d_rgb(nx, ny, objm1.data(), objm1.data(), 4);

                    ggimg::gaussian_filter_2d_rgb(nx, ny, objm0.data(), objm0.data(), 1.0f);
                    ggimg::gaussian_filter_2d_rgb(nx, ny, objm1.data(), objm1.data(), 1.0f);

                    //ggimg::gaussian_filter_2d_gray(nx, ny, objgray0.data(), objgray0.data(), 1.0f);
                    //ggimg::gaussian_filter_2d_gray(nx, ny, objgray1.data(), objgray1.data(), 1.0f);

                    //ggimg::gaussian_filter_2d_gray(nx, ny, objrxy0.data(), objrxy0.data(), 1.0f);
                    //ggimg::gaussian_filter_2d_gray(nx, ny, objrxy1.data(), objrxy1.data(), 1.0f);

                    //ggimg::gaussian_filter_2d_gray(nx, ny, objgxy0.data(), objgxy0.data(), 1.0f);
                    //ggimg::gaussian_filter_2d_gray(nx, ny, objgxy1.data(), objgxy1.data(), 1.0f);

                    //ggimg::gaussian_filter_2d_gray(nx, ny, objbxy0.data(), objbxy0.data(), 1.0f);
                    //ggimg::gaussian_filter_2d_gray(nx, ny, objbxy1.data(), objbxy1.data(), 1.0f);

                    //ggimg::gaussian_filter_2d_gray(nx, ny, objgrayxy0.data(), objgrayxy0.data(), 10.0f);
                    //ggimg::gaussian_filter_2d_gray(nx, ny, objgrayxy1.data(), objgrayxy1.data(), 10.0f);
                }

                std::vector<uint8_t> objh0(C*L*nx*ny);
                std::vector<uint8_t> objh1(C*L*nx*ny);

                ggimg::lhist_filter_2d_rgb(nx, ny, objm0.data(), objh0.data() + 0*L*nx*ny, L, 1*W);
                ggimg::lhist_filter_2d_rgb(nx, ny, objm1.data(), objh1.data() + 0*L*nx*ny, L, 1*W);

                ggimg::lhist_filter_2d_rgb(nx, ny, objm0.data(), objh0.data() + 3*L*nx*ny, L, 2*W);
                ggimg::lhist_filter_2d_rgb(nx, ny, objm1.data(), objh1.data() + 3*L*nx*ny, L, 2*W);

                ggimg::lhist_filter_2d_rgb(nx, ny, objm0.data(), objh0.data() + 6*L*nx*ny, L, 3*W);
                ggimg::lhist_filter_2d_rgb(nx, ny, objm1.data(), objh1.data() + 6*L*nx*ny, L, 3*W);

                ggimg::lhist_filter_2d_rgb(nx, ny, objm0.data(), objh0.data() + 9*L*nx*ny, L, 4*W);
                ggimg::lhist_filter_2d_rgb(nx, ny, objm1.data(), objh1.data() + 9*L*nx*ny, L, 4*W);

                //ggimg::lhist_filter_2d_gray(nx, ny, objgray0.data(), objh0.data() + 12*L*nx*ny, L, 1*W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objgray1.data(), objh1.data() + 12*L*nx*ny, L, 1*W);

                //ggimg::lhist_filter_2d_gray(nx, ny, objgray0.data(), objh0.data() + 13*L*nx*ny, L, 2*W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objgray1.data(), objh1.data() + 13*L*nx*ny, L, 2*W);

                //ggimg::lhist_filter_2d_gray(nx, ny, objgray0.data(), objh0.data() + 14*L*nx*ny, L, 3*W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objgray1.data(), objh1.data() + 14*L*nx*ny, L, 3*W);

                //ggimg::lhist_filter_2d_gray(nx, ny, objgray0.data(), objh0.data() + 15*L*nx*ny, L, 4*W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objgray1.data(), objh1.data() + 15*L*nx*ny, L, 4*W);

                //ggimg::lhist_filter_2d_gray(nx, ny, objrxy0.data(), objh0.data() + 12*L*nx*ny, L, 1*W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objrxy1.data(), objh1.data() + 12*L*nx*ny, L, 1*W);

                //ggimg::lhist_filter_2d_gray(nx, ny, objrxy0.data(), objh0.data() + 13*L*nx*ny, L, 2*W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objrxy1.data(), objh1.data() + 13*L*nx*ny, L, 2*W);

                //ggimg::lhist_filter_2d_gray(nx, ny, objrxy0.data(), objh0.data() + 14*L*nx*ny, L, 3*W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objrxy1.data(), objh1.data() + 14*L*nx*ny, L, 3*W);

                //ggimg::lhist_filter_2d_gray(nx, ny, objrxy0.data(), objh0.data() + 15*L*nx*ny, L, 4*W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objrxy1.data(), objh1.data() + 15*L*nx*ny, L, 4*W);

                //ggimg::lhist_filter_2d_gray(nx, ny, objgxy0.data(), objh0.data() + 16*L*nx*ny, L, 1*W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objgxy1.data(), objh1.data() + 16*L*nx*ny, L, 1*W);

                //ggimg::lhist_filter_2d_gray(nx, ny, objgxy0.data(), objh0.data() + 17*L*nx*ny, L, 2*W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objgxy1.data(), objh1.data() + 17*L*nx*ny, L, 2*W);

                //ggimg::lhist_filter_2d_gray(nx, ny, objgxy0.data(), objh0.data() + 18*L*nx*ny, L, 3*W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objgxy1.data(), objh1.data() + 18*L*nx*ny, L, 3*W);

                //ggimg::lhist_filter_2d_gray(nx, ny, objgxy0.data(), objh0.data() + 19*L*nx*ny, L, 4*W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objgxy1.data(), objh1.data() + 19*L*nx*ny, L, 4*W);

                //ggimg::lhist_filter_2d_gray(nx, ny, objbxy0.data(), objh0.data() + 20*L*nx*ny, L, 1*W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objbxy1.data(), objh1.data() + 20*L*nx*ny, L, 1*W);

                //ggimg::lhist_filter_2d_gray(nx, ny, objbxy0.data(), objh0.data() + 21*L*nx*ny, L, 2*W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objbxy1.data(), objh1.data() + 21*L*nx*ny, L, 2*W);

                //ggimg::lhist_filter_2d_gray(nx, ny, objbxy0.data(), objh0.data() + 22*L*nx*ny, L, 3*W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objbxy1.data(), objh1.data() + 22*L*nx*ny, L, 3*W);

                //ggimg::lhist_filter_2d_gray(nx, ny, objbxy0.data(), objh0.data() + 23*L*nx*ny, L, 4*W);
                //ggimg::lhist_filter_2d_gray(nx, ny, objbxy1.data(), objh1.data() + 23*L*nx*ny, L, 4*W);

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

                ggimg::write_ppm_rgb("obj0.ppm", nx, ny, objm0);
                ggimg::write_ppm_rgb("obj1.ppm", nx, ny, objm1);

                //ggimg::write_ppm_gray("objr0.ppm", nx, ny, objr0);
                //ggimg::write_ppm_gray("objr1.ppm", nx, ny, objr1);
                //ggimg::write_ppm_gray("objrx0.ppm", nx, ny, objrx0);
                //ggimg::write_ppm_gray("objrx1.ppm", nx, ny, objrx1);
                //ggimg::write_ppm_gray("objry0.ppm", nx, ny, objry0);
                //ggimg::write_ppm_gray("objry1.ppm", nx, ny, objry1);
                //ggimg::write_ppm_gray("objrxy0.ppm", nx, ny, objrxy0);
                //ggimg::write_ppm_gray("objrxy1.ppm", nx, ny, objrxy1);

                //ggimg::write_ppm_gray("objg0.ppm", nx, ny, objg0);
                //ggimg::write_ppm_gray("objg1.ppm", nx, ny, objg1);
                //ggimg::write_ppm_gray("objgx0.ppm", nx, ny, objgx0);
                //ggimg::write_ppm_gray("objgx1.ppm", nx, ny, objgx1);
                //ggimg::write_ppm_gray("objgy0.ppm", nx, ny, objgy0);
                //ggimg::write_ppm_gray("objgy1.ppm", nx, ny, objgy1);
                //ggimg::write_ppm_gray("objgxy0.ppm", nx, ny, objgxy0);
                //ggimg::write_ppm_gray("objgxy1.ppm", nx, ny, objgxy1);

                //ggimg::write_ppm_gray("objb0.ppm", nx, ny, objb0);
                //ggimg::write_ppm_gray("objb1.ppm", nx, ny, objb1);
                //ggimg::write_ppm_gray("objbx0.ppm", nx, ny, objbx0);
                //ggimg::write_ppm_gray("objbx1.ppm", nx, ny, objbx1);
                //ggimg::write_ppm_gray("objby0.ppm", nx, ny, objby0);
                //ggimg::write_ppm_gray("objby1.ppm", nx, ny, objby1);
                //ggimg::write_ppm_gray("objbxy0.ppm", nx, ny, objbxy0);
                //ggimg::write_ppm_gray("objbxy1.ppm", nx, ny, objbxy1);

                //ggimg::write_ppm_gray("objgray0.ppm", nx, ny, objgray0);
                //ggimg::write_ppm_gray("objgray1.ppm", nx, ny, objgray1);
                //ggimg::write_ppm_gray("objgrayx0.ppm", nx, ny, objgrayx0);
                //ggimg::write_ppm_gray("objgrayx1.ppm", nx, ny, objgrayx1);
                //ggimg::write_ppm_gray("objgrayy0.ppm", nx, ny, objgrayy0);
                //ggimg::write_ppm_gray("objgrayy1.ppm", nx, ny, objgrayy1);
                //ggimg::write_ppm_gray("objgrayxy0.ppm", nx, ny, objgrayxy0);
                //ggimg::write_ppm_gray("objgrayxy1.ppm", nx, ny, objgrayxy1);

                result.pixels.resize(3*nx*ny);
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

                                for (int k = k0; k < k1; ++k) {
                                    for (int l = 0; l < L; ++l) {
                                        diffcur += std::abs(objh0[k*L*nx*ny + L*i + l] - objh1[k*L*nx*ny + L*ii + l]);
                                    }
                                }

                                if (diffcur < diffbest) {
                                    diffbest = diffcur;
                                }
                            }
                        }

                        diffbest /= nw;

                        result.pixels[3*i + 0] = std::min(255, 1*diffbest);
                        result.pixels[3*i + 1] = std::min(255, 1*diffbest);
                        result.pixels[3*i + 2] = std::min(255, 1*diffbest);

                    }
                }

                ggimg::write_ppm_rgb("result_diff.ppm", nx, ny, result.pixels);

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

                                for (int k = k0; k < k1; ++k) {
                                    for (int l = 0; l < L; ++l) {
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
                    }
                }

                {
                    std::vector<uint8_t> result_gray(nx*ny);
                    ggimg::rgb_to_r_2d(nx, ny, result.pixels.data(), result_gray.data());
                    ggimg::normalize_2d(nx, ny, result_gray.data(), (uint8_t) 0, (uint8_t) 255, result_gray.data());
                    ggimg::gray_to_rgb_2d(nx, ny, result_gray.data(), result.pixels.data());
                }

                ggimg::write_ppm_rgb("result_cc.ppm", nx, ny, result.pixels);
            }
            break;
        case LocalCC:
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

                result.pixels.resize(3*nx*ny);
                result.nx = nx;
                result.ny = ny;

                int w = 3;
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

                        double sum0  = 0.0;
                        double sum02 = 0.0;

                        for (int yyy = y - w; yyy <= y + w; ++yyy) {
                            for (int xxx = x - w; xxx <= x + w; ++xxx) {
                                int iii = yyy*nx + xxx;

                                { double v = objm0.pixels[3*iii + 0]; sum0 += v; sum02 += v*v; }
                                { double v = objm0.pixels[3*iii + 1]; sum0 += v; sum02 += v*v; }
                                { double v = objm0.pixels[3*iii + 2]; sum0 += v; sum02 += v*v; }
                            }
                        }

                        double ccbest = -1.0;

                        for (int yy = y - ws; yy <= y + ws; ++yy) {
                            for (int xx = x - ws; xx <= x + ws; ++xx) {
                                int ii = yy*nx + xx;

                                double sum1 = 0.0;
                                double sum12 = 0.0;
                                double sum01 = 0.0;

                                for (int yyy = yy - w; yyy <= yy + w; ++yyy) {
                                    for (int xxx = xx - w; xxx <= xx + w; ++xxx) {
                                        int iii = yyy*nx + xxx;
                                        int ii = (yyy - yy + y)*nx + (xxx - xx + x);

                                        {
                                            double v0 = objm0.pixels[3*ii + 0];
                                            double v1 = objm1.pixels[3*iii + 0];

                                            sum1 += v1;
                                            sum12 += v1*v1;
                                            sum01 += v0*v1;
                                        }
                                        {
                                            double v0 = objm0.pixels[3*ii + 1];
                                            double v1 = objm1.pixels[3*iii + 1];

                                            sum1 += v1;
                                            sum12 += v1*v1;
                                            sum01 += v0*v1;
                                        }
                                        {
                                            double v0 = objm0.pixels[3*ii + 2];
                                            double v1 = objm1.pixels[3*iii + 2];

                                            sum1 += v1;
                                            sum12 += v1*v1;
                                            sum01 += v0*v1;
                                        }
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

                        result.pixels[3*i + 0] = 127*(1.0 - ccbest);
                        result.pixels[3*i + 1] = 127*(1.0 - ccbest);
                        result.pixels[3*i + 2] = 127*(1.0 - ccbest);
                    }
                }
            }
            break;
        case SSIM:
            {
                int nx = obj0.nx;
                int ny = obj0.ny;

                BufferRGB objm0;
                BufferRGB objm1;

                {
                    int nnx = 0;
                    int nny = 0;

                    ggimg::scale_li_maxside_2d_rgb(nx, ny, obj0.pixels.data(), 512, nnx, nny, objm0);
                    ggimg::scale_li_maxside_2d_rgb(nx, ny, obj1.pixels.data(), 512, nnx, nny, objm1);

                    nx = nnx;
                    ny = nny;
                }

                std::vector<uint8_t> objgray0(nx*ny);
                std::vector<uint8_t> objgray1(nx*ny);
                std::vector<uint8_t> objgrayxy0(nx*ny);
                std::vector<uint8_t> objgrayxy1(nx*ny);

                {
                    //ggimg::median_filter_2d_rgb(nx, ny, objm0.data(), objm0.data(), 4);
                    //ggimg::median_filter_2d_rgb(nx, ny, objm1.data(), objm1.data(), 4);

                    ggimg::rgb_to_gray_2d(nx, ny, objm0.data(), objgray0.data());
                    ggimg::rgb_to_gray_2d(nx, ny, objm1.data(), objgray1.data());

                    ggimg::gradient_sobel_2d(0, nx, ny, objgray0.data(), (uint8_t) 255, objgrayxy0.data());
                    ggimg::gradient_sobel_2d(0, nx, ny, objgray1.data(), (uint8_t) 255, objgrayxy1.data());
                }

                {
                    ggimg::gaussian_filter_2d_rgb(nx, ny, objm0.data(), objm0.data(), 1.0f);
                    ggimg::gaussian_filter_2d_rgb(nx, ny, objm1.data(), objm1.data(), 1.0f);

                    ggimg::gaussian_filter_2d_gray(nx, ny, objgray0.data(), objgray0.data(), 1.0f);
                    ggimg::gaussian_filter_2d_gray(nx, ny, objgray1.data(), objgray1.data(), 1.0f);

                    ggimg::gaussian_filter_2d_gray(nx, ny, objgrayxy0.data(), objgrayxy0.data(), 1.0f);
                    ggimg::gaussian_filter_2d_gray(nx, ny, objgrayxy1.data(), objgrayxy1.data(), 1.0f);
                }

                std::vector<uint8_t> result_gray(nx*ny);

                int w = 8;
                int ws = 1;
                int ww = ws + w;
                int nw = 3*(2*w + 1)*(2*w + 1);

                for (int y = ww; y < ny - ww; ++y) {
                    for (int x = ww; x < nx - ww; ++x) {
                        int i = y*nx + x;
                        if (objgray1[i] == 0) {
                            result_gray[i] = 0;
                            continue;
                        }

                        double sum0  = 0.0;
                        double sum02 = 0.0;

                        for (int yyy = y - w; yyy <= y + w; ++yyy) {
                            for (int xxx = x - w; xxx <= x + w; ++xxx) {
                                int iii = yyy*nx + xxx;

                                //{ double v = objgray0[iii];    sum0 += v; sum02 += v*v; }
                                //{ double v = objgrayxy0[iii];  sum0 += v; sum02 += v*v; }
                                { double v = objm0[3*iii + 0]; sum0 += v; sum02 += v*v; }
                                { double v = objm0[3*iii + 1]; sum0 += v; sum02 += v*v; }
                                { double v = objm0[3*iii + 2]; sum0 += v; sum02 += v*v; }
                            }
                        }

                        double ssimbest = -1.0;

                        for (int yy = y - ws; yy <= y + ws; ++yy) {
                            for (int xx = x - ws; xx <= x + ws; ++xx) {
                                double sum1 = 0.0;
                                double sum12 = 0.0;
                                double sum01 = 0.0;

                                for (int yyy = yy - w; yyy <= yy + w; ++yyy) {
                                    for (int xxx = xx - w; xxx <= xx + w; ++xxx) {
                                        int iii = yyy*nx + xxx;
                                        int ii = (yyy - yy + y)*nx + (xxx - xx + x);

                                        //{
                                        //    double v0 = objgray0[ii];
                                        //    double v1 = objgray1[iii];

                                        //    sum1 += v1;
                                        //    sum12 += v1*v1;
                                        //    sum01 += v0*v1;
                                        //}
                                        //{
                                        //    double v0 = objgrayxy0[ii];
                                        //    double v1 = objgrayxy1[iii];

                                        //    sum1 += v1;
                                        //    sum12 += v1*v1;
                                        //    sum01 += v0*v1;
                                        //}
                                        {
                                            double v0 = objm0[3*ii + 0];
                                            double v1 = objm1[3*iii + 0];

                                            sum1 += v1;
                                            sum12 += v1*v1;
                                            sum01 += v0*v1;
                                        }
                                        {
                                            double v0 = objm0[3*ii + 1];
                                            double v1 = objm1[3*iii + 1];

                                            sum1 += v1;
                                            sum12 += v1*v1;
                                            sum01 += v0*v1;
                                        }
                                        {
                                            double v0 = objm0[3*ii + 2];
                                            double v1 = objm1[3*iii + 2];

                                            sum1 += v1;
                                            sum12 += v1*v1;
                                            sum01 += v0*v1;
                                        }
                                    }
                                }

                                double ssimcur = 0.0;

                                {
                                    double k1 = 0.01;
                                    double k2 = 0.03;
                                    double L = 255.0;
                                    double c1 = (k1*L)*(k1*L);
                                    double c2 = (k2*L)*(k2*L);
                                    double mean0 = sum0/nw/1.1;
                                    double mean1 = sum1/nw/1.1;
                                    double cov01 = sum01/nw/1.1 - mean0*mean1;
                                    double var0  = sum02/nw/1.1 - mean0*mean0;
                                    double var1  = sum12/nw/1.1 - mean1*mean1;

                                    double term0 = (2.0*mean0*mean1 + c1)/(mean0*mean0 + mean1*mean1 + c1);
                                    double term1 = (2.0*cov01 + c2)/(var0 + var1 + c2);
                                    ssimcur = term0*term1;
                                }

                                if (ssimcur > ssimbest) {
                                    ssimbest = ssimcur;
                                }
                            }
                        }

                        result_gray[i] = 127*(1.0 - ssimbest);
                    }
                }

                result.pixels.resize(3*nx*ny);
                result.nx = nx;
                result.ny = ny;

                ggimg::gray_to_rgb_2d(nx, ny, result_gray.data(), result.pixels.data());
            }
            break;
        case Binary:
            {
                int nSamples = 5000;
                float sigma = 16.0f;
                std::vector<int> binaryKernel(4*nSamples);
                for (int i = 0; i < nSamples; ++i) {
                    float x0 = (2.0*frand() - 1.0)*sigma*0.1;
                    float y0 = (2.0*frand() - 1.0)*sigma*0.1;
                    //float x0 = 0.0f;
                    //float y0 = 0.0f;

                    float x1 = x0;
                    float y1 = y0;

                    while ((x1 - x0)*(x1 - x0) + (y1 - y0)*(y1 - y0) < 1.1*1.1) {
                        x1 = (2.0*frand() - 1.0)*sigma;
                        y1 = (2.0*frand() - 1.0)*sigma;
                    }

                    //printf("%8.6f %8.6f %8.6f %8.6f\n", x0, y0, x1, y1);

                    binaryKernel[4*i + 0] = std::lroundl(x0);
                    binaryKernel[4*i + 1] = std::lroundl(y0);
                    binaryKernel[4*i + 2] = std::lroundl(x1);
                    binaryKernel[4*i + 3] = std::lroundl(y1);
                }

                int nx = obj0.nx;
                int ny = obj0.ny;

                auto objm0 = obj0;
                auto objm1 = obj1;

                {
                    int nnx = 0;
                    int nny = 0;

                    //ggimg::scale_li_maxside_2d_rgb(nx, ny, obj0.pixels.data(), 4096, nnx, nny, objm0.pixels);
                    //ggimg::scale_li_maxside_2d_rgb(nx, ny, obj1.pixels.data(), 4096, nnx, nny, objm1.pixels);

                    ggimg::scale_li_maxside_2d_rgb(nx, ny, obj0.pixels.data(), 1024, nnx, nny, objm0.pixels);
                    ggimg::scale_li_maxside_2d_rgb(nx, ny, obj1.pixels.data(), 1024, nnx, nny, objm1.pixels);

                    nx = nnx;
                    ny = nny;
                }

                //ggimg::median_filter_2d_rgb(nx, ny, objm0.pixels.data(), objm0.pixels.data(), 5);
                //ggimg::median_filter_2d_rgb(nx, ny, objm1.pixels.data(), objm1.pixels.data(), 5);

                //ggimg::gaussian_filter_2d_rgb(nx, ny, objm0.pixels.data(), objm0.pixels.data(), 4.0f);
                //ggimg::gaussian_filter_2d_rgb(nx, ny, objm1.pixels.data(), objm1.pixels.data(), 4.0f);

                std::vector<uint8_t> objgray0(nx*ny);
                std::vector<uint8_t> objgray1(nx*ny);
                std::vector<uint8_t> objgray0_tmp(nx*ny);
                std::vector<uint8_t> objgray1_tmp(nx*ny);

                {
                    ggimg::rgb_to_gray_2d(nx, ny, objm0.pixels.data(), objgray0_tmp.data());
                    ggimg::rgb_to_gray_2d(nx, ny, objm1.pixels.data(), objgray1_tmp.data());

                    //objgray0 = objgray0_tmp;
                    //objgray1 = objgray1_tmp;

                    //ggimg::gradient_sobel_2d(0, nx, ny, objgray0_tmp.data(), (uint8_t) 255, objgray0.data());
                    //ggimg::gradient_sobel_2d(0, nx, ny, objgray1_tmp.data(), (uint8_t) 255, objgray1.data());
                }

                //ggimg::write_ppm_gray("sobel0.ppm", nx, ny, objgray0);
                //ggimg::write_ppm_gray("sobel1.ppm", nx, ny, objgray1);

                //ggimg::median_filter_2d_gray(nx, ny, objgray0.data(), objgray0_tmp.data(), 1.0f);
                //ggimg::median_filter_2d_gray(nx, ny, objgray1.data(), objgray1_tmp.data(), 1.0f);

                //objgray0 = objgray0_tmp;
                //objgray1 = objgray1_tmp;

                //ggimg::gaussian_filter_2d_gray(nx, ny, objgray0.data(), objgray0.data(), 6.0f);
                //ggimg::gaussian_filter_2d_gray(nx, ny, objgray1.data(), objgray1.data(), 6.0f);

                //ggimg::write_ppm_gray("sobel_gauss0.ppm", nx, ny, objgray0);
                //ggimg::write_ppm_gray("sobel_gauss1.ppm", nx, ny, objgray1);

                result.pixels.resize(3*nx*ny, 0);
                result.nx = nx;
                result.ny = ny;

                //std::vector<float> fv0(nSamples);
                //std::vector<float> fv1(nSamples);
                //std::vector<float> fres(nx*ny, 0.0f);

                //float vmax = -1e6;
                //float vmin = 1e6;

                //float vt = 5.0f;

                //for (int y = sigma + 1; y < ny - sigma - 1; ++y) {
                //    for (int x = sigma + 1; x < nx - sigma - 1; ++x) {
                //        for (int i = 0; i < nSamples; ++i) {
                //            {
                //                float v0 = objgray0[(y + binaryKernel[4*i + 1])*nx + (x + binaryKernel[4*i + 0])];
                //                float v1 = objgray0[(y + binaryKernel[4*i + 3])*nx + (x + binaryKernel[4*i + 2])];

                //                //fv0[i + 0] = std::fabs(v1 - v0);
                //                fv0[i + 0] = (v1 - vt > v0) ? +1.0f : (v1 + vt < v0) ? -1.0f : 0.0f;
                //                //fv0[i + 0] = v1 >= v0 ? +1.0f : -1.0f;
                //                //fv0[i + 0] = v1 - vt > v0 ? v1 - v0 : 0.0f;
                //                //fv0[i + 0] = v1 - v0;
                //            }

                //            {
                //                float v0 = objgray1[(y + binaryKernel[4*i + 1])*nx + (x + binaryKernel[4*i + 0])];
                //                float v1 = objgray1[(y + binaryKernel[4*i + 3])*nx + (x + binaryKernel[4*i + 2])];

                //                //fv1[i + 0] = std::fabs(v1 - v0);
                //                fv1[i + 0] = (v1 - vt > v0) ? +1.0f : (v1 + vt < v0) ? -1.0f : 0.0f;
                //                //fv1[i + 0] = v1 >= v0 ? +1.0f : -1.0f;
                //                //fv1[i + 0] = v1 - vt > v0 ? v1 - v0 : 0.0f;
                //                //fv1[i + 0] = v1 - v0;
                //            }
                //        }

                //        float sum = 0.0;

                //        for (int i = 0; i < nSamples; ++i) {
                //            float v = fv0[i]*fv1[i];
                //            sum  += v;
                //        }

                //        if (sum < vmin) vmin = sum;
                //        if (sum > vmax) vmax = sum;

                //        fres[y*nx + x] = sum;
                //    }
                //}

                //printf("vmin = %g\n", vmin);
                //printf("vmax = %g\n", vmax);

                //for (int y = sigma + 1; y < ny - sigma - 1; ++y) {
                //    for (int x = sigma + 1; x < nx - sigma - 1; ++x) {
                //        float v = (fres[y*nx + x] - vmin)/(vmax - vmin);
                //        result.pixels[3*(y*nx + x) + 0] = 250*(1.0f - v);
                //        result.pixels[3*(y*nx + x) + 1] = 250*(1.0f - v);
                //        result.pixels[3*(y*nx + x) + 2] = 250*(1.0f - v);
                //    }
                //}

                result.pixels.resize(3*nx*ny, 0);
                result.nx = nx;
                result.ny = ny;

                std::vector<float> fv0(3*nSamples);
                std::vector<float> fv1(3*nSamples);
                std::vector<float> fres(nx*ny, 0.0f);

                float vmax = -1e6;
                float vmin = 1e6;

                float vt = 5.0f;

                for (int y = sigma + 1; y < ny - sigma - 1; ++y) {
                    for (int x = sigma + 1; x < nx - sigma - 1; ++x) {
                        for (int i = 0; i < nSamples; ++i) {
                            {
                                float v0 = objm0.pixels[3*((y + binaryKernel[4*i + 1])*nx + (x + binaryKernel[4*i + 0])) + 0];
                                float v1 = objm0.pixels[3*((y + binaryKernel[4*i + 3])*nx + (x + binaryKernel[4*i + 2])) + 0];

                                fv0[3*i + 0] = (v1 - vt > v0) ? +1.0f : (v1 + vt < v0) ? -1.0f : 0.0f;
                            }

                            {
                                float v0 = objm0.pixels[3*((y + binaryKernel[4*i + 1])*nx + (x + binaryKernel[4*i + 0])) + 1];
                                float v1 = objm0.pixels[3*((y + binaryKernel[4*i + 3])*nx + (x + binaryKernel[4*i + 2])) + 1];

                                fv0[3*i + 1] = (v1 - vt > v0) ? +1.0f : (v1 + vt < v0) ? -1.0f : 0.0f;
                            }

                            {
                                float v0 = objm0.pixels[3*((y + binaryKernel[4*i + 1])*nx + (x + binaryKernel[4*i + 0])) + 2];
                                float v1 = objm0.pixels[3*((y + binaryKernel[4*i + 3])*nx + (x + binaryKernel[4*i + 2])) + 2];

                                fv0[3*i + 2] = (v1 - vt > v0) ? +1.0f : (v1 + vt < v0) ? -1.0f : 0.0f;
                            }

                            {
                                float v0 = objm1.pixels[3*((y + binaryKernel[4*i + 1])*nx + (x + binaryKernel[4*i + 0])) + 0];
                                float v1 = objm1.pixels[3*((y + binaryKernel[4*i + 3])*nx + (x + binaryKernel[4*i + 2])) + 0];

                                fv1[3*i + 0] = (v1 - vt > v0) ? +1.0f : (v1 + vt < v0) ? -1.0f : 0.0f;
                            }

                            {
                                float v0 = objm1.pixels[3*((y + binaryKernel[4*i + 1])*nx + (x + binaryKernel[4*i + 0])) + 1];
                                float v1 = objm1.pixels[3*((y + binaryKernel[4*i + 3])*nx + (x + binaryKernel[4*i + 2])) + 1];

                                fv1[3*i + 1] = (v1 - vt > v0) ? +1.0f : (v1 + vt < v0) ? -1.0f : 0.0f;
                            }

                            {
                                float v0 = objm1.pixels[3*((y + binaryKernel[4*i + 1])*nx + (x + binaryKernel[4*i + 0])) + 2];
                                float v1 = objm1.pixels[3*((y + binaryKernel[4*i + 3])*nx + (x + binaryKernel[4*i + 2])) + 2];

                                fv1[3*i + 2] = (v1 - vt > v0) ? +1.0f : (v1 + vt < v0) ? -1.0f : 0.0f;
                            }
                        }

                        float sum = 0.0;

                        for (int i = 0; i < nSamples; ++i) {
                            float v = 0.0f;
                            v += fv0[3*i + 0]*fv1[3*i + 0];
                            v += fv0[3*i + 1]*fv1[3*i + 1];
                            v += fv0[3*i + 2]*fv1[3*i + 2];
                            sum  += v;
                        }

                        if (sum < vmin) vmin = sum;
                        if (sum > vmax) vmax = sum;

                        fres[y*nx + x] = sum;
                    }
                }

                printf("vmin = %g\n", vmin);
                printf("vmax = %g\n", vmax);

                for (int y = sigma + 1; y < ny - sigma - 1; ++y) {
                    for (int x = sigma + 1; x < nx - sigma - 1; ++x) {
                        float v = (fres[y*nx + x] - vmin)/(vmax - vmin);
                        result.pixels[3*(y*nx + x) + 0] = 250*(1.0f - v);
                        result.pixels[3*(y*nx + x) + 1] = 250*(1.0f - v);
                        result.pixels[3*(y*nx + x) + 2] = 250*(1.0f - v);
                    }
                }
            }
            break;
    }

    ggimg::write_ppm_rgb("result_final.ppm", result.nx, result.ny, result.pixels);

    return result;
}