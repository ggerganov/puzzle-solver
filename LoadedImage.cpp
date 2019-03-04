/*! \file LoadedImage.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "Types.h"
#include "Functions.h"

template <>
bool Free::operator()<LoadedImage>(LoadedImage & obj) {
    bool res = true;

    res &= ::Free()(obj.image);

    return res;
}

template <>
bool Exist::operator()<std::vector<LoadedImage>>(const std::vector<LoadedImage> & obj, const char * fname) {
    bool res = false;

    for (const auto & cur : obj) {
        if (cur.fname == fname) {
            res = true;
            break;
        }
    }

    return res;
}
