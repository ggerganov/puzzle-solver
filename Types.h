/*! \file Types.h
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#pragma once

#include <map>
#include <vector>
#include <memory>
#include <string>
#include <cstdint>
#include <array>

struct Texture {
    int32_t id = -1;
};

struct BufferRGB : public std::vector<uint8_t> {
    using vector::operator[];
    int32_t npixels = 0;
};

struct Point2D {
    float x = 0.0f;
    float y = 0.0f;
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
    ImageRGB * image = nullptr;
};

struct LoadedImage {
    std::string fname = "";
    ImageRGB image;
};

struct ViewLoadedImages {
    int32_t editId = 0;
    int32_t referenceId = 0;
    int32_t selectedId = -1;
};

struct CommonPoint {
    using ImageId = int32_t;
    std::map<ImageId, Point2D> posInImage;
};

struct ViewSelectedImage {
    bool showGrid = true;
    bool showProjected = true;

    float alphaProjected = 1.0f;

    FieldOfView fov;
    CommonPoint commonPointInput;
};

struct ViewActions {
};

struct Homography : public std::array<float, 9> {
    using array::array;
};

struct StateApp {
    // UI

    enum EAction {
        None = 0,
        LoadingImages,
        AddingCommonPoint,
        EditingCommonPoint,
    };

    EAction curAction = LoadingImages;

    float leftPanelSizeX = 320.f;
    float loadedImagesSizeY = 200.0f;

    ViewLoadedImages viewLoadedImages;
    ViewSelectedImage viewSelectedImage;
    ViewActions viewActions;

    // Data

    ImageRGB projectedImage;

    std::vector<CommonPoint> commonPoints;
    std::vector<LoadedImage> loadedImages;
    std::map<int, std::map<int, Homography>> homographies;
};
