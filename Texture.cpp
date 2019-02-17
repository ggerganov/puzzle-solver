/*! \file Texture.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "Types.h"
#include "Functions.h"

#include <GLFW/glfw3.h>

template <>
bool Free::operator()<Texture>(Texture & obj) {
    if (obj.id < 0) {
        return false;
    }

    GLuint texId = obj.id;
    printf("Destroying OpenGL texture %d\n", texId);
    glDeleteTextures(1, &texId);
    obj.id = -1;

    return true;
}
