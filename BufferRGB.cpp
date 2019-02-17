/*! \file BufferRGB.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "Types.h"
#include "Functions.h"

template <>
bool Resize::operator()<BufferRGB>(const int32_t n, BufferRGB & obj) {
    obj.resize(3*n);
    return true;
}
