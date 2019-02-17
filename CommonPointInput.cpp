/*! \file CommonPointInput.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "Types.h"
#include "Functions.h"

template <>
bool Exist::operator()<CommonPointInput>(const int32_t id, const CommonPointInput & obj) {
    return obj.posInImage.find(id) != obj.posInImage.end();
}

template <>
int32_t Count::operator()<CommonPointInput>(const CommonPointInput & obj) {
    return obj.posInImage.size();
}
