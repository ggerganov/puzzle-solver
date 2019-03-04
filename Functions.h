/*! \file Functions.h
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#pragma once

#include <array>
#include <cstdint>
#include <memory>

struct Resize {
    template <typename T> bool operator()(T & obj, const int32_t n);
    template <typename T> bool operator()(T & obj, const int32_t nx, const int32_t ny);
};

struct LoadFromFile {
    template <typename T> bool operator()(T & obj, const char * fname);
};

struct SaveToFile {
    template <typename T> bool operator()(const T & obj, const char * fname);
};

struct OnDragAndDrop {
    template <typename T> bool operator()(T & obj, const char * fname);
};

struct Render {
    template <typename T> bool operator()(T & obj);
    template <typename T> bool operator()(T & obj, struct StateApp & state);
};

struct IsValid {
    template <typename T> bool operator()(const T & obj);
};

struct Exist {
    template <typename T> bool operator()(const T & obj, const int32_t id);
    template <typename T> bool operator()(const T & obj, const char * fname);
};

struct Erase {
    template <typename T> bool operator()(T & obj, const int32_t id);
};

struct Count {
    template <typename T> int32_t operator()(const T & obj);
};

struct Free {
    template <typename T> bool operator()(T & obj);
};

struct GenerateTexture {
    template <typename T> bool operator()(T & obj, const bool linearInterp);
};

struct ComputeHomography {
    enum Method {
        GaussianElimination = 0,
    } method;

    template <typename T> struct Homography operator()(const std::array<T, 4> & p0, const std::array<T, 4> & p1);
};

struct ComputeDifference {
    enum Method {
        Standard = 0,
    } method;

    template <typename T> T operator()(const T & obj0, const T & obj1);
};
