/*! \file ImageRGB.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "Types.h"
#include "Functions.h"

template <>
bool Resize::operator()<ImageRGB>(const int32_t nx, const int32_t ny, ImageRGB & obj) {
    obj.nx = nx;
    obj.ny = ny;
    ::Resize()(nx*ny, obj.pixels);

    return true;
}

template <>
ImageRGB Create::operator()<ImageRGB>(const int32_t nx, const int32_t ny) {
    ImageRGB res;

    res.nx = nx;
    res.ny = ny;
    ::Resize()(nx*ny, res.pixels);

    return res;
}
