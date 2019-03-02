/*! \file Homography.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "Types.h"
#include "Functions.h"

#include <cmath>

template <>
Homography ComputeHomography::operator()<Point2D>(const std::array<Point2D, 4> & p0, const std::array<Point2D, 4> & p1) {
    Homography result;

    std::vector<float> b(8);
    b[0] = p1[0].x; b[1] = p1[0].y;
    b[2] = p1[1].x; b[3] = p1[1].y;
    b[4] = p1[2].x; b[5] = p1[2].y;
    b[6] = p1[3].x; b[7] = p1[3].y;

    std::vector<std::vector<float>> A(8);
    for (auto & row : A) row.resize(8);

    A[0][0] = p0[0].x; A[0][1] = p0[0].y; A[0][2] = 1.0f; A[0][3] =    0.0f; A[0][4] =    0.0f; A[0][5] = 0.0f; A[0][6] = -p0[0].x*p1[0].x; A[0][7] = -p0[0].y*p1[0].x;
    A[1][0] =    0.0f; A[1][1] =    0.0f; A[1][2] = 0.0f; A[1][3] = p0[0].x; A[1][4] = p0[0].y; A[1][5] = 1.0f; A[1][6] = -p0[0].x*p1[0].y; A[1][7] = -p0[0].y*p1[0].y;

    A[2][0] = p0[1].x; A[2][1] = p0[1].y; A[2][2] = 1.0f; A[2][3] =    0.0f; A[2][4] =    0.0f; A[2][5] = 0.0f; A[2][6] = -p0[1].x*p1[1].x; A[2][7] = -p0[1].y*p1[1].x;
    A[3][0] =    0.0f; A[3][1] =    0.0f; A[3][2] = 0.0f; A[3][3] = p0[1].x; A[3][4] = p0[1].y; A[3][5] = 1.0f; A[3][6] = -p0[1].x*p1[1].y; A[3][7] = -p0[1].y*p1[1].y;

    A[4][0] = p0[2].x; A[4][1] = p0[2].y; A[4][2] = 1.0f; A[4][3] =    0.0f; A[4][4] =    0.0f; A[4][5] = 0.0f; A[4][6] = -p0[2].x*p1[2].x; A[4][7] = -p0[2].y*p1[2].x;
    A[5][0] =    0.0f; A[5][1] =    0.0f; A[5][2] = 0.0f; A[5][3] = p0[2].x; A[5][4] = p0[2].y; A[5][5] = 1.0f; A[5][6] = -p0[2].x*p1[2].y; A[5][7] = -p0[2].y*p1[2].y;

    A[6][0] = p0[3].x; A[6][1] = p0[3].y; A[6][2] = 1.0f; A[6][3] =    0.0f; A[6][4] =    0.0f; A[6][5] = 0.0f; A[6][6] = -p0[3].x*p1[3].x; A[6][7] = -p0[3].y*p1[3].x;
    A[7][0] =    0.0f; A[7][1] =    0.0f; A[7][2] = 0.0f; A[7][3] = p0[3].x; A[7][4] = p0[3].y; A[7][5] = 1.0f; A[7][6] = -p0[3].x*p1[3].y; A[7][7] = -p0[3].y*p1[3].y;

    if (method == GaussianElimination) {
        int n = A.size();
        for (int i = 0; i < n; i++) {
            int imax = i;
            {
                double amax = std::abs(A[i][i]);
                for (int k = i + 1; k < n; k++) {
                    if (std::abs(A[k][i]) > amax) {
                        amax = std::abs(A[k][i]);
                        imax = k;
                    }
                }
            }

            for (int k = i; k < n; k++) {
                std::swap(A[imax][k], A[i][k]);
            }
            std::swap(b[imax], b[i]);

            for (int k = i + 1; k < n; k++) {
                double c = -A[k][i]/A[i][i];
                for (int j = i; j < n; j++) {
                    if (i == j) {
                        A[k][j] = 0.0f;
                    } else {
                        A[k][j] += c*A[i][j];
                    }
                }
                b[k] += c*b[i];
            }
        }

        for (int i = n - 1; i >= 0; i--) {
            result[i] = b[i]/A[i][i];
            for (int k = i - 1; k >= 0; k--) {
                b[k] -= A[k][i]*result[i];
            }
        }

        result[8] = 1.0f;
    }

    return result;
};
