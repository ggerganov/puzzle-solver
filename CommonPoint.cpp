/*! \file CommonPoint.cpp
 *  \brief Enter description here.
 *  \author Georgi Gerganov
 */

#include "Types.h"
#include "Functions.h"

#include <fstream>

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

template <>
bool LoadFromFile::operator()<std::vector<CommonPoint>>(std::vector<CommonPoint> & obj, const char * fname) {
    std::ifstream fin(fname);
    if (fin.good() == false) {
        return false;
    }

    obj.clear();

    {
        int n = 0;
        fin >> n;
        obj.resize(n);
    }
    for (auto & p : obj) {
        int n = 0;
        fin >> n;
        for (int i = 0; i < n; ++i) {
            int id;
            fin >> id;
            Point2D c;
            fin >> c.x >> c.y;
            p.posInImage[id] = c;
        }
    }

    return true;
}

template <>
bool SaveToFile::operator()<std::vector<CommonPoint>>(const std::vector<CommonPoint> & obj, const char * fname) {
    std::ofstream fout(fname);
    fout << obj.size() << std::endl;
    for (auto & p : obj) {
        fout << p.posInImage.size() << std::endl;
        for (auto & c : p.posInImage) {
            fout << c.first << " " << c.second.x << " " << c.second.y << std::endl;
        }
    }

    return true;
}

template <>
int32_t Count::operator()<std::vector<CommonPoint>>(const std::vector<CommonPoint> & obj) {
    return obj.size();
}
