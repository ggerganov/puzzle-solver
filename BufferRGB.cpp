/*! \file BufferRGB.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "Types.h"
#include "Functions.h"

template <>
bool Resize::operator()<BufferRGB>(BufferRGB & obj, const int32_t n) {
    obj.resize(3*n);
    return true;
}
