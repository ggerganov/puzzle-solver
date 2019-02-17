/*! \file Types.h
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#pragma once

#include <vector>
#include <memory>
#include <cstdint>

struct BufferRGB : std::vector<char> {
    int32_t npixels = 0;
};

struct ImageRGB {
    int32_t nx = -1;
    int32_t ny = -1;

    BufferRGB pixels;
};
