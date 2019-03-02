/*! \file CommonPoint.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "Types.h"
#include "Functions.h"

template <>
bool Exist::operator()<CommonPoint>(const CommonPoint & obj, const int32_t id) {
    return obj.posInImage.find(id) != obj.posInImage.end();
}

template <>
bool Erase::operator()<CommonPoint>(CommonPoint & obj, const int32_t id) {
    if (::Exist()(obj, id)) {
        obj.posInImage.erase(id);
        return true;
    }
    return false;
}

template <>
int32_t Count::operator()<CommonPoint>(const CommonPoint & obj) {
    return obj.posInImage.size();
}
