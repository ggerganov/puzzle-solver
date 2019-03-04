/*! \file Texture.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "Types.h"
#include "Functions.h"

#include "GL/gl3w.h"

template <>
bool Free::operator()<Texture>(Texture & obj) {
    if (obj.id < 0) {
        return false;
    }

    GLuint texId = obj.id;
    glDeleteTextures(1, &texId);
    obj.id = -1;

    return true;
}

template <>
bool IsValid::operator()<Texture>(const Texture & obj) {
    return obj.id > 0;
}
