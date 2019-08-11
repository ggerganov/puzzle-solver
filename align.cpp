/*! \file align.cpp
 *  \brief Aling two photos
 *  \author Georgi Gerganov
 */

#include "Types.h"
#include "Functions.h"

#define GGIMG_FS
#include "ggimg/ggimg.h"

namespace {
    float frand() { return float(rand())/RAND_MAX; }
}

int main(int argc, char ** argv) {
    printf("Usage: %s img0 img1 pts\n", argv[0]);
    if (argc < 4) {
        return -1;
    }

    ImageRGB img0;
    ImageRGB img1;
    std::vector<CommonPoint> commonPoints;

    ::LoadFromFile()(img0, argv[1]);
    ::LoadFromFile()(img1, argv[2]);
    ::LoadFromFile()(commonPoints, argv[3]);

    printf("Image 0: %d %d\n", img0.nx, img0.ny);
    printf("Image 1: %d %d\n", img1.nx, img1.ny);
    printf("Points : %d\n", ::Count()(commonPoints));

    std::array<Point2D, 4> pi;
    std::array<Point2D, 4> pj;

    pi[0] = commonPoints[0].posInImage.at(0);
    pj[0] = commonPoints[0].posInImage.at(1);
    pi[1] = commonPoints[1].posInImage.at(0);
    pj[1] = commonPoints[1].posInImage.at(1);
    pi[2] = commonPoints[2].posInImage.at(0);
    pj[2] = commonPoints[2].posInImage.at(1);
    pi[3] = commonPoints[3].posInImage.at(0);
    pj[3] = commonPoints[3].posInImage.at(1);

    ImageRGB img0p = ::Register()(img0, img1, pi, pj);

    auto [ sum1, sum12 ] = ::ComputeSums()(img1, 0.5f, 0.5f, 1, 1);
    auto cc = ::ComputeCC()(img1, img0p, sum1, sum12, 0.5f, 0.5f, 1, 1);
    printf("cc = %g\n", cc);

    ggimg::write_ppm_rgb("diff_img0.ppm", img0.nx, img0.ny, img0.pixels);
    ggimg::write_ppm_rgb("diff_img1.ppm", img1.nx, img1.ny, img1.pixels);
    ggimg::write_ppm_rgb("diff_img0p.ppm", img0p.nx, img0p.ny, img0p.pixels);

    //auto diff = ::ComputeDifference{::ComputeDifference::Binary}(img1, img0p);
    //auto diff = ::ComputeDifference{::ComputeDifference::LocalDiff}(img1, img0p);

    return 0;
}
