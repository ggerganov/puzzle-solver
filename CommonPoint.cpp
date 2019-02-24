/*! \file CommonPoint.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "Types.h"
#include "Functions.h"

template <>
bool Exist::operator()<CommonPoint>(const int32_t id, const CommonPoint & obj) {
    return obj.posInImage.find(id) != obj.posInImage.end();
}

template <>
bool Erase::operator()<CommonPoint>(const int32_t id, CommonPoint & obj) {
    if (::Exist()(id, obj)) {
        obj.posInImage.erase(id);
        return true;
    }
    return false;
}

template <>
int32_t Count::operator()<CommonPoint>(const CommonPoint & obj) {
    return obj.posInImage.size();
}
