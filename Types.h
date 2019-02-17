/*! \file Types.h
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#pragma once

#include <vector>
#include <memory>
#include <string>
#include <cstdint>

struct Texture {
    int32_t id = -1;
};

struct BufferRGB : public std::vector<uint8_t> {
    using vector::operator[];
    int32_t npixels = 0;
};

struct ImageRGB {
    int32_t nx = -1;
    int32_t ny = -1;

    BufferRGB pixels;
    Texture texture;
};

struct FieldOfView {
    float centerX = 0.5f;
    float centerY = 0.5f;

    float sizeX = 1.0f;
    float sizeY = 1.0f;
};

struct ImageRGBView {
    FieldOfView * fov = nullptr;
    ImageRGB *image = nullptr;
};

struct LoadedImage {
    std::string fname = "";
    ImageRGB image;
};

struct LoadedImages : public std::vector<LoadedImage> {
    int32_t selectedId = -1;
};

struct StateUI {
    LoadedImages loadedImages;
    FieldOfView fovSelectedImage;
};
