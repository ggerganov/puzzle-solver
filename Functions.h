/*! \file Functions.h
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#pragma once

#include <cstdint>
#include <memory>

struct Resize {
    template <typename T> bool operator()(const int32_t n, T & obj);
    template <typename T> bool operator()(const int32_t nx, const int32_t ny, T & obj);
};

struct Create {
    template <typename T> T operator()(const int32_t n);
    template <typename T> T operator()(const int32_t nx, const int32_t ny);
};
