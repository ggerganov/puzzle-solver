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

template <>
int32_t Count::operator()<BufferRGB>(const BufferRGB & obj) {
    return obj.size()/3;
}
